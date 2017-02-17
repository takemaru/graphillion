/*
 * TdZdd: a Top-down/Breadth-first Decision Diagram Manipulation Framework
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2014 ERATO MINATO Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <cassert>
#include <cstring>
#include <stdint.h>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "../DdSpec.hpp"
#include "../util/Graph.hpp"

namespace tdzdd {

struct FrontierBasedSearchCount {
    int16_t uec; ///< uncolored edge component counter.

    FrontierBasedSearchCount()
            : uec(0) {
    }

    FrontierBasedSearchCount(int16_t uncoloredEdgeComponents)
            : uec(uncoloredEdgeComponents) {
    }

    size_t hash() const {
        return uec;
    }

    bool operator==(FrontierBasedSearchCount const& o) const {
        return uec == o.uec;
    }

    friend std::ostream& operator<<(std::ostream& os,
            FrontierBasedSearchCount const& o) {
        return os << o.uec;
    }
};

class FrontierBasedSearchMate {
public:
    typedef int16_t Offset;
    static Offset const UNCOLORED = 32766;
    static Offset const UNCOLORED_EDGE_COMPONENT = 32767;

private:
    Offset hoc; ///< offset to head or color.
    Offset nxt; ///< offset to next connected vertex.

    /*
     *  ┌────────────────────────┌─────────────────────┐
     *  │   ┌────┐────────┐      │   ┌────┐────────┐   │
     * ┌┼───┴┐  ┌┼────┐  ┌┼────┐┌┼───┴┐  ┌┼────┐  ┌┼───┴┐
     * │color│  │head │  │head ││color│  │head │  │head │
     * ├─────┤  ├─────┤  ├─────┤├─────┤  ├─────┤  ├─────┤
     * │next─┼──┤next─┼──┤next ││next─┼──┤next─┼──┤next │
     * └─────┘  └─────┘  └─────┘└─────┘  └─────┘  └─────┘
     *  head                     head              coloredTail
     */

public:
    FrontierBasedSearchMate(Offset hoc = 0)
            : hoc(hoc), nxt(0) {
    }

    bool operator==(FrontierBasedSearchMate const& o) const {
        return this == &o;
    }

    bool operator!=(FrontierBasedSearchMate const& o) const {
        return this != &o;
    }

    void clear() {
        hoc = 0;
        nxt = 0;
    }

    bool isHead() const {
        return hoc >= 0;
    }

    bool isTail() const {
        return nxt == 0;
    }

    bool isIsolated() const {
        return isHead() && isTail();
    }

    FrontierBasedSearchMate& head() {
        return isHead() ? *this : *(this + hoc);
    }

    FrontierBasedSearchMate const& head() const {
        return isHead() ? *this : *(this + hoc);
    }

    FrontierBasedSearchMate& next() {
        return *(this + nxt);
    }

    FrontierBasedSearchMate const& next() const {
        return *(this + nxt);
    }

    bool isColored() const {
        return head().hoc < UNCOLORED;
    }

    bool isUncoloredEdgeComponent() const {
        return head().hoc == UNCOLORED_EDGE_COMPONENT;
    }

    bool isColoredTail() const {
        return hoc == 0 || (hoc < 0 && hoc + (this + hoc)->hoc == 0);
    }

    bool hasSameColorAs(FrontierBasedSearchMate const& o) const {
        FrontierBasedSearchMate const* p = &head();
        FrontierBasedSearchMate const* q = &o.head();
        return p + p->hoc == q + q->hoc;
    }

    FrontierBasedSearchMate const* findColorPredecessor(
            FrontierBasedSearchMate const& o) const {
        assert(o.isColoredTail());
        FrontierBasedSearchMate const* p = &o;

        while (--p >= this) {
            FrontierBasedSearchMate const* p1 = &p->head();
            if (p1 + p1->hoc == &o) return p;
        }

        return 0;
    }

    void mergeLists(FrontierBasedSearchMate& o1, FrontierBasedSearchMate& o2) {
        FrontierBasedSearchMate* p1 = &o1.head();
        FrontierBasedSearchMate* p2 = &o2.head();
        if (p1 == p2) return;
        if (p1 > p2) std::swap(p1, p2);

        bool painting; // merging colored and uncolored

        if (p2->hoc < UNCOLORED) {
            painting = (p1->hoc >= UNCOLORED);

            if (painting || p1 + p1->hoc < p2 + p2->hoc) {
                p1->hoc = p2->hoc + (p2 - p1); // updated if uncolored element becomes tail
            }
        }
        else {
            painting = (p1->hoc < UNCOLORED);

            if (p1->hoc == UNCOLORED) {
                p1->hoc = UNCOLORED_EDGE_COMPONENT;
            }
        }

        for (FrontierBasedSearchMate* q = p2;; q += q->nxt) {
            q->hoc = p1 - q;
            if (q->nxt == 0) break;
        }

        FrontierBasedSearchMate* p = p1;
        FrontierBasedSearchMate* q = p2;

        while (true) {
            assert(p != q);
            FrontierBasedSearchMate* pp = p + p->nxt;
            assert(p <= pp && pp != q);

            while (p < pp && pp < q) {
                p = pp;
                pp += pp->nxt;
                assert(p <= pp && pp != q);
            }

            assert(p == pp || q < pp);
            p->nxt = q - p;
            if (p == pp) break;
            p = q, q = pp;
        }

        if (painting) {
            while (q->nxt) {
                q += q->nxt;
            }

            FrontierBasedSearchMate* pp = p1 + p1->hoc;

            if (pp < q) { // q must be an uncolored tail.
                for (p = this; p <= pp; ++p) {
                    if (p + p->hoc == pp) p->hoc = q - p;
                }
            }
        }
    }

    void replaceHeadWith(FrontierBasedSearchMate& newHead) const {
        FrontierBasedSearchMate const* p = &head();
        FrontierBasedSearchMate* q = &newHead;

        q->hoc = (p->hoc < UNCOLORED) ? p->hoc + (p - q) : p->hoc;

        while (q->nxt > 0) {
            q += q->nxt;
            q->hoc = &newHead - q;
        }
    }

    void removeFromList(FrontierBasedSearchMate const& o) {
        if (o.isColoredTail()) {
            assert(o.nxt == 0);
            FrontierBasedSearchMate const* pp = findColorPredecessor(o);
            if (pp == 0) return;

            for (FrontierBasedSearchMate* p = this; p <= pp; ++p) {
                if (p + p->hoc == &o) p->hoc = pp - p;
                if (p + p->nxt == &o) p->nxt = 0;
            }
        }
        else if (o.nxt == 0) {
            for (FrontierBasedSearchMate* p = this; p <= &o; ++p) {
                if (p + p->nxt == &o) p->nxt = 0;
            }
        }
        else {
            for (FrontierBasedSearchMate* p = this; p <= &o; ++p) {
                if (p + p->nxt == &o) p->nxt += o.nxt;
            }
        }
    }

    friend std::ostream& operator<<(std::ostream& os,
            FrontierBasedSearchMate const& o) {
        return os << "<" << o.hoc << "," << o.nxt << ">";
    }
};

class FrontierBasedSearch: public tdzdd::HybridDdSpec<FrontierBasedSearch,
        FrontierBasedSearchCount,FrontierBasedSearchMate,2> {
    typedef FrontierBasedSearchCount Count;
    typedef FrontierBasedSearchMate Mate;

    Graph const& graph;
    int const m;
    int const n;
    int const mateSize;
    std::vector<Mate> initialMate;
    int numUEC;
    bool const noLoop;
    bool const lookahead;

    int takable(Count& c, Mate const* mate, Graph::EdgeInfo const& e) const {
        Mate const& w1 = mate[e.v1 - e.v0];
        Mate const& w2 = mate[e.v2 - e.v0];

        // don't connect again
        if (noLoop && w1.head() == w2.head()) return false;

        // don't connect different colors
        if (w1.isColored() && w2.isColored() && !w1.hasSameColorAs(w2)) return false;

        if (e.v1final && e.v2final) {
            if (w1.isIsolated() && w2.isIsolated()) { // new component leaves immediately
                if (w2.isColored()) {
                    // don't leave the color unconnected
                    if (!w2.isColoredTail()) return false;
                    if (mate[1].findColorPredecessor(w2)) return false;
                }
                else {
                    if (w1.isColored()) {
                        // don't leave the color unconnected
                        if (!w1.isColoredTail()) return false;
                    }
                    else {
                        if (c.uec == 0) return false;
                        if (c.uec > 0) --c.uec;
                    }
                }
            }
            else if (w1.isHead() && w2 == w1.next() && w2.isTail()) { // existing component leaves
                if (w1.isColored()) {
                    // don't leave the color unconnected
                    if (!w2.isColoredTail()) return false;
                    if (w2.findColorPredecessor(mate[1])) return false;
                }
                else {
                    assert(w1.isUncoloredEdgeComponent());
                    if (c.uec == 0) return false;
                    if (c.uec > 0) --c.uec;
                }
            }
        }

        if (e.finalEdge && c.uec > 0) return false; //TODO 枝刈り可能
        return true;
    }

    bool doTake(Count& count, Mate* mate, Graph::EdgeInfo const& e) const {
        Count c = count;

        if (!takable(c, mate, e)) return false;

        count = c;
        mate[0].mergeLists(mate[e.v1 - e.v0], mate[e.v2 - e.v0]);
        return true;
    }

    bool doNotTake(Count& count, Mate* mate, Graph::EdgeInfo const& e) const {
        Count c = count;
        Mate const& w1 = mate[e.v1 - e.v0];
        Mate const& w2 = mate[e.v2 - e.v0];

        if (e.v1final && w1.isIsolated()) {
            if (w1.isColored()) {
                // don't leave the color unconnected
                if (!w1.isColoredTail()) return false;
            }
            else if (c.uec >= 0 && w1.isUncoloredEdgeComponent()) {
                if (c.uec == 0) return false;
                --c.uec;
            }
        }

        if (e.v2final && w2.isIsolated()) {
            if (w2.isColored()) {
                // don't leave the color unconnected
                if (!w2.isColoredTail()) return false;
                if (mate[1].findColorPredecessor(w2)) return false;
            }
            else if (c.uec >= 0 && w2.isUncoloredEdgeComponent()) {
                if (c.uec == 0) return false;
                --c.uec;
            }
        }

        if (e.v1final && e.v2final && w1.isHead() && w2 == w1.next()
                && w2.isTail()) { // existing component leaves) {
            if (w1.isColored()) {
                // don't leave the color unconnected
                if (!w2.isColoredTail()) return false;
                if (w2.findColorPredecessor(mate[1])) return false;
            }
            else {
                assert(w1.isUncoloredEdgeComponent());
                if (c.uec == 0) return false;
                if (c.uec > 0) --c.uec;
            }
        }

        if (e.finalEdge && c.uec > 0) return false; //TODO 枝刈り可能
        count = c;
        return true;
    }

    void update(Mate* mate, Graph::EdgeInfo const& e,
            Graph::EdgeInfo const& ee) const {
        int const d = ee.v0 - e.v0;
        assert(d >= 0);
        Mate* p1 = &mate[e.v1 - e.v0];
        Mate* p2 = &mate[e.v2 - e.v0];
        Mate* pd = p1 + d;

        for (Mate* q = p1; q < pd; ++q) {
            Mate* qq = &q->next();
            if (qq >= pd) {
                q->replaceHeadWith(*qq);
            }
        }

        if (e.v2final) {
            mate[0].removeFromList(*p2);
            p2->clear();
        }

        if (e.v1final) {
            mate[0].removeFromList(*p1);
            p1->clear();
        }

        if (d > 0) {
            std::memmove(p1, pd, (mateSize - d) * sizeof(*mate));
            for (int i = mateSize - d; i < mateSize; ++i) {
                p1[i] = initialMate[ee.v0 + i];
            }
        }
    }

public:
    FrontierBasedSearch(Graph const& graph, int numUEC = -1,
            bool noLoop = false, bool lookahead = true)
            : graph(graph), m(graph.vertexSize()), n(graph.edgeSize()),
              mateSize(graph.maxFrontierSize()), initialMate(1 + m + mateSize),
              numUEC(numUEC), noLoop(noLoop), lookahead(lookahead) {
        this->setArraySize(mateSize);

        std::vector<int> rootOfColor(graph.numColor() + 1);
        for (int v = 1; v <= m; ++v) {
            rootOfColor[graph.colorNumber(v)] = v;
        }
        for (int v = 1; v <= m; ++v) {
            int k = graph.colorNumber(v);
            int hoc = (k > 0) ? rootOfColor[k] - v : Mate::UNCOLORED;
            initialMate[v] = Mate(hoc);
        }
    }

    int getRoot(Count& count, Mate* mate) const {
        int const v0 = graph.edgeInfo(0).v0;

        count = Count(numUEC);

        for (int i = 0; i < mateSize; ++i) {
            mate[i] = initialMate[v0 + i];
        }

        return n;
    }

    int getChild(Count& count, Mate* mate, int level, int take) const {
        assert(1 <= level && level <= n);
        int i = n - level;
        Graph::EdgeInfo const* e = &graph.edgeInfo(i);

        if (take) {
            if (!doTake(count, mate, *e)) return 0;
        }
        else {
            if (!doNotTake(count, mate, *e)) return 0;
        }

        if (++i == n) return -1;

        Graph::EdgeInfo const* ee = &graph.edgeInfo(i);
        update(mate, *e, *ee);

        while (lookahead) {
            e = ee;

            Count c = count;
            if (takable(c, mate, *e)) break;
            if (!doNotTake(count, mate, *e)) return 0;

            if (++i == n) return -1;

            ee = &graph.edgeInfo(i);
            update(mate, *e, *ee);
        }

        assert(i < n);
        return n - i;
    }

    size_t hashCode(Count const& count) const {
        return count.hash();
    }
};

} // namespace tdzdd
