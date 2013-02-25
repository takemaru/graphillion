/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: RcutZdd.hpp 415 2013-02-22 12:55:13Z iwashita $
 */

#pragma once

#include <cassert>
#include <stdint.h>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "../dd/DdSpec.hpp"
#include "../util/Graph.hpp"

struct RcutZddMate {
    typedef int16_t Offset;
    static Offset const UNCOLORED = 32767;

    Offset next; // offset to next connected vertex
    Offset hoc;  // offset to head or color

    void clear() {
        next = 0;
        hoc = 0;
    }

    RcutZddMate& head() {
        return (hoc < 0) ? this[hoc] : this[0];
    }

    RcutZddMate const& head() const {
        return (hoc < 0) ? this[hoc] : this[0];
    }

    bool hasSameColorAs(RcutZddMate const& o) const {
        RcutZddMate const* p = &head();
        RcutZddMate const* q = &o.head();
        return p + p->hoc == q + q->hoc;
    }

    bool isIsolated() const {
        return hoc >= 0 && next == 0;
    }

    bool isColored() const {
        return head().hoc != UNCOLORED;
    }

    bool isTailOfColor() const {
        return (hoc < 0) ? hoc + this[hoc].hoc == 0 : hoc == 0;
    }

    RcutZddMate const* findColorPredecessor(RcutZddMate const& top) const {
        assert(isColored());
        RcutZddMate const* p = this;

        while (--p >= &top) {
            RcutZddMate const* p1 = &p->head();
            if (p1 + p1->hoc == this) return p;
        }

        return 0;
    }

    void merge(RcutZddMate& o) {
        RcutZddMate* p1 = &head();
        RcutZddMate* q1 = &o.head();
        if (p1 == q1) return;

        if (p1 > q1) std::swap(p1, q1);
        bool painting = (p1->hoc == UNCOLORED || q1->hoc == UNCOLORED);

        if (q1->hoc != UNCOLORED
                && (p1->hoc == UNCOLORED || p1 + p1->hoc < q1 + q1->hoc)) {
            p1->hoc = q1->hoc + (q1 - p1);
        }

        for (RcutZddMate* q = q1;; q += q->next) {
            q->hoc = p1 - q;
            if (q->next == 0) break;
        }

        RcutZddMate* p = p1;
        RcutZddMate* q = q1;

        while (true) {
            assert(p != q);
            RcutZddMate* pp = p + p->next;
            assert(p <= pp && pp != q);

            while (p < pp && pp < q) {
                p = pp;
                pp += pp->next;
                assert(p <= pp && pp != q);
            }

            assert(p == pp || q < pp);
            p->next = q - p;
            if (p == pp) break;
            p = q, q = pp;
        }

        if (painting) {
            while (q->next) {
                q += q->next;
            }

            RcutZddMate* qq = p1 + p1->hoc;
            if (qq < q) {
                // make all ids for the color point to q.
                p1->hoc = q - p1;
                for (RcutZddMate* pp = p1 + 1; pp <= qq; ++pp) {
                    if (pp + pp->hoc == qq) {
                        pp->hoc = q - pp;
                    }
                }
            }
        }
    }

    void moveTopTo(RcutZddMate& to) const {
        RcutZddMate const* p = &head();
        RcutZddMate* q = &to;

        q->hoc = (p->hoc == UNCOLORED) ? UNCOLORED : p->hoc + (p - q);

        while (q->next > 0) {
            q += q->next;
            q->hoc = &to - q;
        }
    }

    void deleteFromTo(RcutZddMate& top,
            RcutZddMate const& newTailOfColor) const {
        assert(&newTailOfColor + newTailOfColor.next == this);
        assert(&newTailOfColor.head() + newTailOfColor.head().hoc == this);

        if (next == 0) {
            for (RcutZddMate* p = &top; p <= &newTailOfColor; ++p) {
                if (p + p->next == this) p->next = 0;
                if (p + p->hoc == this) p->hoc = &newTailOfColor - p;
            }
        }
        else {
            for (RcutZddMate* p = &top; p <= &newTailOfColor; ++p) {
                if (p + p->next == this) p->next += next;
                if (p + p->hoc == this) p->hoc = &newTailOfColor - p;
            }
        }
    }

    void deleteFrom(RcutZddMate& top) const {
        if (next == 0) {
            for (RcutZddMate* p = &top; p <= this; ++p) {
                if (p + p->next == this) p->next = 0;
            }
        }
        else {
            for (RcutZddMate* p = &top; p <= this; ++p) {
                if (p + p->next == this) p->next += next;
            }
        }
    }

    friend std::ostream& operator<<(std::ostream& os, RcutZddMate const& o) {
        return os << "<" << o.next << "," << o.hoc << ">";
    }
};

class RcutZdd: public PodHybridDdSpec<RcutZdd,int16_t,RcutZddMate> {
    Graph const& graph;
    int const m;
    int const n;
    int const mateSize;
    std::vector<RcutZddMate> initialMate;

    bool doTake(int16_t& count, RcutZddMate* mate,
            Graph::EdgeInfo const& e) const {
        int16_t c = count;
        RcutZddMate const& w1 = mate[0];
        RcutZddMate const& w2 = mate[e.v2 - e.v1];

        if (e.v1final && w1.isIsolated()) {
            if (!w1.isTailOfColor()) return false;
            --c;
        }

        if (e.v2final && w2.isIsolated()) {
            if (!w2.isTailOfColor()) return false;
            if (w2.findColorPredecessor(mate[1])) return false;
            --c;
        }

        assert(c >= 0);
        if (c == 0 && !e.finalEdge) return false;

        count = c;
        return true;
    }

    bool doNotTake(int16_t& count, RcutZddMate* mate,
            Graph::EdgeInfo const& e) const {
        int16_t c = count;
        RcutZddMate& w1 = mate[0];
        RcutZddMate& w2 = mate[e.v2 - e.v1];

        if (w1.isColored() && w2.isColored() && !w1.hasSameColorAs(w2)) return false;

        if (e.v1final && e.v2final && w1.isIsolated() && w2.isIsolated()) {
            if (!w1.isTailOfColor() && !w2.isTailOfColor()) return false;
            if (w2.isColored() && w2.findColorPredecessor(mate[1])) return false;
            --c;
        }

        assert(c >= 0);
        if (c == 0 && !e.finalEdge) return false;

        w1.merge(w2);
        count = c;
        return true;
    }

    void update(RcutZddMate* mate, Graph::EdgeInfo const& e,
            Graph::EdgeInfo const& ee) const {
        int const d = ee.v1 - e.v1;
        assert(d >= 0);
        RcutZddMate* p1 = mate;
        RcutZddMate* p2 = mate + e.v2 - e.v1;
        RcutZddMate* pd = p1 + d;

        for (RcutZddMate* q = p1; q < pd; ++q) {
            RcutZddMate* qq = q + q->next;
            if (qq >= pd) {
                q->moveTopTo(*qq);
            }
        }

        if (e.v2final) {
            if (p2->isTailOfColor()) {
                RcutZddMate const* pp = p2->findColorPredecessor(*p1);
                if (pp) {
                    p2->deleteFromTo(*p1, *pp);
                }
            }
            else {
                p2->deleteFrom(*p1);
            }
            p2->clear();
        }

        if (e.v1final) {
            p1->clear();
        }

        if (d > 0) {
            std::memmove(p1, pd, (mateSize - d) * sizeof(*mate));
            for (int i = mateSize - d; i < mateSize; ++i) {
                p1[i] = initialMate[ee.v1 + i];
                //std::cerr << "\np1[" << i << "] = " << p1[i];
            }
        }
    }

public:
    RcutZdd(Graph const& graph)
            : graph(graph), m(graph.vertexSize()), n(graph.edgeSize()),
              mateSize(graph.maxFrontierSize()), initialMate(1 + m + mateSize) {
        setArraySize(mateSize + 1);

        std::vector<int> lastVertex(graph.numColor() + 1);
        for (int v = 1; v <= m; ++v) {
            lastVertex[graph.colorNumber(v)] = v;
        }
        for (int v = 1; v <= m; ++v) {
            int k = graph.colorNumber(v);
            initialMate[v].hoc =
                    (k > 0) ? lastVertex[k] - v : RcutZddMate::UNCOLORED;
            //std::cerr << "\nitialMate[" << v << "] = " << initialMate[v];
        }
    }

    int getRoot(int16_t& count, RcutZddMate* mate) const {
        int const v1 = graph.edgeInfo(0).v1;

        count = graph.numColor();

        for (int i = 0; i < mateSize; ++i) {
            mate[i] = initialMate[v1 + i];
        }

        return n;
    }

    int getChild(int16_t& count, RcutZddMate* mate, int level,
            bool take) const {
        assert(1 <= level && level <= n);
        int i = n - level;
        Graph::EdgeInfo const* e = &graph.edgeInfo(i);
//        std::cerr << "\ne" << i << ": " << (take ? "T" : "F") << " v" << e->v1 << "-v" << e->v2 << " ";
//        print(std::cerr, mate);

        if (take) {
            if (!doTake(count, mate, *e)) return 0;
        }
        else {
            if (!doNotTake(count, mate, *e)) return 0;
        }

        if (++i == n) return -1;
//        std::cerr << " -> ";
//        print(std::cerr, mate);

        Graph::EdgeInfo const* ee = &graph.edgeInfo(i);
        update(mate, *e, *ee);
//        std::cerr << " => ";
//        print(std::cerr, mate);

        while (true) {
            e = ee;

            int16_t c = count;
            if (doTake(c, mate, *e)) break;
//            std::cerr << "\ne" << i << ": F v" << e->v1 << "-v" << e->v2 << " ";
//            print(std::cerr, mate);
            if (!doNotTake(count, mate, *e)) return 0;

            if (++i == n) return -1;
//            std::cerr << " -> ";
//            print(std::cerr, mate);

            ee = &graph.edgeInfo(i);
            update(mate, *e, *ee);
//            std::cerr << " => ";
//            print(std::cerr, mate);
        }

        assert(i < n);
        return n - i;
    }
};

/**
 * This implementation does not accept mutiple terminals with the same color.
 * Simpler and less memory, but somewhat slower than RcutZdd.
 */
class RcutZdd0: public PodHybridDdSpec<RcutZdd0,int16_t,int16_t> {
    Graph const& graph;
    int const n;
    int const nv;
    int const nc;
    int const mateSize;
    std::vector<int16_t> initialMate;

    mutable std::vector<int16_t> vMap;
    mutable std::vector<int16_t> cMap;

    bool isIsolated(int16_t const* mate, int k) const {
        int w = mate[k];
        for (int j = 0; j < k; ++j) {
            if (mate[j] == w) return false;
        }
        for (int j = k + 1; j < mateSize; ++j) {
            if (mate[j] == w) return false;
        }
        return true;
    }

    bool doTake(int16_t& count, int16_t const* mate,
            Graph::EdgeInfo const& e) const {
        int c = count;
        int const k1 = 0;
        int const k2 = e.v2 - e.v1;

        if (e.v1final && isIsolated(mate, k1)) {
            if (mate[k1] >= 0) return false;
            --c;
        }

        if (e.v2final && isIsolated(mate, k2)) {
            if (mate[k2] >= 0) return false;
            --c;
        }

        assert(c >= 0);
        if (c == 0 && !e.finalEdge) return false;

        count = c;
        return true;
    }

    bool doNotTake(int16_t& count, int16_t* mate,
            Graph::EdgeInfo const& e) const {
        int c = count;
        int const k1 = 0;
        int const k2 = e.v2 - e.v1;
        int w1 = mate[k1];
        int w2 = mate[k2];

        if (w1 < 0 && w2 < 0 && w1 != w2) return false;

        if (e.v1final && e.v2final && isIsolated(mate, k1)
                && isIsolated(mate, k2)) {
            if (w1 >= 0 && w2 >= 0) return false;
            --c;
        }

        assert(c >= 0);
        if (c == 0 && !e.finalEdge) return false;

        if (w1 > w2) std::swap(w1, w2);
        for (int j = 0; j < mateSize; ++j) {
            if (mate[j] == w2) mate[j] = w1;
        }

        count = c;
        return true;
    }

    void update(int16_t* mate, Graph::EdgeInfo const& e,
            Graph::EdgeInfo const& ee) const {
        int const k1 = 0;
        int const k2 = e.v2 - e.v1;
        int const d = ee.v1 - e.v1;
        assert(d >= 0);

        if (e.v1final) mate[k1] = 0;
        if (e.v2final) mate[k2] = 0;

        std::memset(&vMap[1], 0, sizeof(int16_t) * nv);
        std::memset(&cMap[1], 0, sizeof(int16_t) * nc);
        int16_t v = 0;
        int16_t c = 0;

        for (int j = 0; j < mateSize - d; ++j) {
            int w = mate[j + d];
            if (w > 0) {
                int16_t& x = vMap[w];
                if (x == 0) x = ++v;
                mate[j] = x;
            }
            else if (w < 0) {
                int16_t& x = cMap[-w];
                if (x == 0) x = ++c;
                mate[j] = -x;
            }
            else {
                mate[j] = 0;
            }
        }

        for (int j = mateSize - d; j < mateSize; ++j) {
            mate[j] = initialMate[ee.v1 + j];
            //std::cerr << "\np1[" << i << "] = " << p1[i];
        }
    }

public:
    RcutZdd0(Graph const& graph)
            : graph(graph), n(graph.edgeSize()), nv(graph.vertexSize()),
              nc(graph.numColor()), mateSize(graph.maxFrontierSize()),
              initialMate(1 + nv + mateSize), vMap(nv + 1), cMap(nc + 1) {
        setArraySize(mateSize + 1);

        std::vector<int> lastVertex(graph.numColor() + 1);
        for (int v = 1; v <= nv; ++v) {
            lastVertex[graph.colorNumber(v)] = v;
        }
        for (int v = 1; v <= nv; ++v) {
            int k = graph.colorNumber(v);
            initialMate[v] = (k > 0) ? -k : v;
            //std::cerr << "\nitialMate[" << v << "] = " << initialMate[v];
        }
    }

    int getRoot(int16_t& count, int16_t* mate) const {
        int const v1 = graph.edgeInfo(0).v1;

        count = graph.numColor();

        for (int i = 0; i < mateSize; ++i) {
            mate[i] = initialMate[v1 + i];
        }

        return n;
    }

    int getChild(int16_t& count, int16_t* mate, int level, bool take) const {
        assert(1 <= level && level <= n);
        int i = n - level;
        Graph::EdgeInfo const* e = &graph.edgeInfo(i);
//        std::cerr << "\ne" << i << ": " << (take ? "T" : "F") << " v" << e->v1 << "-v" << e->v2 << " ";
//        print(std::cerr, state);

        if (take) {
            if (!doTake(count, mate, *e)) return 0;
        }
        else {
            if (!doNotTake(count, mate, *e)) return 0;
        }

        if (++i == n) return -1;
//        std::cerr << " -> ";
//        print(std::cerr, state);

        Graph::EdgeInfo const* ee = &graph.edgeInfo(i);
        update(mate, *e, *ee);
//        std::cerr << " => ";
//        print(std::cerr, state);

        while (true) {
            e = ee;

            int16_t c = count;
            if (doTake(c, mate, *e)) break;
//            std::cerr << "\ne" << i << ": F v" << e->v1 << "-v" << e->v2 << " ";
//            print(std::cerr, state);
            if (!doNotTake(count, mate, *e)) return 0;

            if (++i == n) return -1;
//            std::cerr << " -> ";
//            print(std::cerr, state);

            ee = &graph.edgeInfo(i);
            update(mate, *e, *ee);
//            std::cerr << " => ";
//            print(std::cerr, state);
        }

        assert(i < n);
        return n - i;
    }
};
