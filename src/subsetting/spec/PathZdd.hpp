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

#include <vector>

#include "../DdSpec.hpp"
#include "../util/Graph.hpp"

namespace tdzdd {

enum SimpathBasedImplType {
    Path, Cycle
};

template<typename S, SimpathBasedImplType type, bool hamilton = false>
class SimpathBasedImpl: public tdzdd::PodArrayDdSpec<S,int16_t,2> {
public:
    typedef int16_t Mate;

private:
    Graph const& graph;
    int const m;
    int const n;
    int const mateArraySize_;
    std::vector<Mate> initialMate;

    bool const lookahead;

    enum Takable {
        NO, YES, HIT
    };

    void shiftMate(Mate* mate, int v0, int vv0) const {
        int const d = vv0 - v0;
        if (d > 0) {
            std::memmove(mate, mate + d, (mateArraySize_ - d) * sizeof(*mate));
            for (int k = mateArraySize_ - d; k < mateArraySize_; ++k) {
                mate[k] = initialMate[vv0 + k];
            }
        }
    }

    Takable takable(Mate const* mate, Graph::EdgeInfo const& e) const {
        int const w1 = mate[e.v1 - e.v0];
        int const w2 = mate[e.v2 - e.v0];

        if (w1 == 0) return NO;
        if (e.v1final && w1 == e.v1) return NO;

        if (w2 == 0) return NO;
        if (e.v2final && w2 == e.v2) return NO;

        switch (type) {
        case Path:
            if (w1 == e.v2) return NO;

            if (w1 < 0 && w2 < 0) {
                if (w1 != w2) return NO;
                if (!e.allColorsSeen) return YES;

                bool clean = true;
                for (int k = 0; k < mateArraySize_; ++k) {
                    if (e.v0 + k == e.v1) continue;
                    if (e.v0 + k == e.v2) continue;
                    int w = mate[k];
                    if (w < 0) return YES;
                    if (w != 0 && (hamilton || w != e.v0 + k)) clean = false;
                }
                return clean ? HIT : NO;
            }
            break;
        case Cycle:
            if (w1 == e.v2) {
                assert(w2 == e.v1);

                for (int k = 1; k < mateArraySize_; ++k) {
                    if (e.v0 + k == e.v1) continue;
                    if (e.v0 + k == e.v2) continue;
                    int w = mate[k];
                    if (w != 0 && (hamilton || w != e.v0 + k)) return NO;
                }
                return HIT;
            }
            break;
        }

        return YES;
    }

    bool leavable(Mate const* mate, Graph::EdgeInfo const& e) const {
        int const w1 = mate[e.v1 - e.v0];
        int const w2 = mate[e.v2 - e.v0];

        if (hamilton) {
            if (e.v1final && w1 != 0) return false;
            if (e.v2final && w2 != 0) return false;
            if (e.v1final2 && w1 == e.v1) return false;
            if (e.v2final2 && w2 == e.v2) return false;
        }
        else {
            if (e.v1final && w1 != 0 && w1 != e.v1) return false;
            if (e.v2final && w2 != 0 && w2 != e.v2) return false;
        }

        return true;
    }

public:
    SimpathBasedImpl(Graph const& graph, bool lookahead)
            : graph(graph), m(graph.vertexSize()), n(graph.edgeSize()),
              mateArraySize_(graph.maxFrontierSize()),
              initialMate(m + mateArraySize_), lookahead(lookahead) {
        this->setArraySize(mateArraySize_);

        for (int v = 1; v <= m; ++v) {
            int c = graph.colorNumber(v);
            initialMate[v] = (c > 0) ? -c : v;
        }

        for (int v = m + 1; v < m + mateArraySize_; ++v) {
            initialMate[v] = 0;
        }
    }

    int mateArraySize() const {
        return mateArraySize_;
    }

    int getRoot(Mate* mate) const {
        int const v0 = graph.edgeInfo(0).v0;

        for (int k = 0; k < mateArraySize_; ++k) {
            mate[k] = initialMate[v0 + k];
        }

        return n;
    }

    int getChild(Mate* mate, int level, int take) const {
        assert(1 <= level && level <= n);
        int i = n - level;
        Graph::EdgeInfo const& e = graph.edgeInfo(i);
        assert(e.v1 <= e.v2);

        if (take) {
            switch (takable(mate, e)) {
            case NO:
                return 0;
            case HIT:
                return -1;
            case YES:
                break;
            }

            int w1 = mate[e.v1 - e.v0];
            int w2 = mate[e.v2 - e.v0];
            if (w1 > 0) mate[w1 - e.v0] = w2;
            if (w2 > 0) mate[w2 - e.v0] = w1;
            if (e.v1final || w1 != e.v1) mate[e.v1 - e.v0] = 0;
            if (e.v2final || w2 != e.v2) mate[e.v2 - e.v0] = 0;
        }
        else {
            if (!leavable(mate, e)) return 0;

            Mate& w1 = mate[e.v1 - e.v0];
            Mate& w2 = mate[e.v2 - e.v0];
            if (e.v1final || (e.v1final2 && w1 == e.v1)) w1 = 0;
            if (e.v2final || (e.v2final2 && w2 == e.v2)) w2 = 0;
        }

        if (++i == n) return 0;
        shiftMate(mate, e.v0, graph.edgeInfo(i).v0);

        while (lookahead) {
            Graph::EdgeInfo const& e = graph.edgeInfo(i);
            assert(e.v1 <= e.v2);

            if (takable(mate, e) != NO) break;
            if (!leavable(mate, e)) return 0;
            if (++i == n) return 0;

            Mate& w1 = mate[e.v1 - e.v0];
            Mate& w2 = mate[e.v2 - e.v0];
            if (e.v1final || (e.v1final2 && w1 == e.v1)) w1 = 0;
            if (e.v2final || (e.v2final2 && w2 == e.v2)) w2 = 0;

            shiftMate(mate, e.v0, graph.edgeInfo(i).v0);
        }

        assert(i < n);
        return n - i;
    }
};

struct PathZdd: public SimpathBasedImpl<PathZdd,Path,false> {
    PathZdd(Graph const& graph, bool lookahead = true)
            : SimpathBasedImpl<PathZdd,Path,false>(graph, lookahead) {
    }
};

struct HamiltonPathZdd: public SimpathBasedImpl<HamiltonPathZdd,Path,true> {
    HamiltonPathZdd(Graph const& graph, bool lookahead = true)
            : SimpathBasedImpl<HamiltonPathZdd,Path,true>(graph, lookahead) {
    }
};

struct CycleZdd: public SimpathBasedImpl<CycleZdd,Cycle,false> {
    CycleZdd(Graph const& graph, bool lookahead = true)
            : SimpathBasedImpl<CycleZdd,Cycle,false>(graph, lookahead) {
    }
};

struct HamiltonCycleZdd: public SimpathBasedImpl<HamiltonCycleZdd,Cycle,true> {
    HamiltonCycleZdd(Graph const& graph, bool lookahead = true)
            : SimpathBasedImpl<HamiltonCycleZdd,Cycle,true>(graph, lookahead) {
    }
};

} // namespace tdzdd
