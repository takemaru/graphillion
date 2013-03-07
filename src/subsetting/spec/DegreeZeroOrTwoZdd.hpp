/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: DegreeZeroOrTwoZdd.hpp 426 2013-02-26 06:50:04Z iwashita $
 */

#pragma once

#include <cassert>
#include <cstring>
#include <iostream>

#include "../dd/DdSpec.hpp"
#include "../util/Graph.hpp"

class DegreeZeroOrTwoZdd: public PodArrayDdSpec<DegreeZeroOrTwoZdd,int16_t> {
    typedef State Mate;

    Graph const& graph;
    int const n;
    int const mateSize;

    void shiftMate(Mate* mate, int v0, int vv0) const {
        int const d = vv0 - v0;
        if (d > 0) {
            std::memmove(mate, mate + d, (mateSize - d) * sizeof(*mate));
            for (int k = mateSize - d; k < mateSize; ++k) {
                mate[k] = graph.colorNumber(vv0 + k) ? 1 : 0;
            }
        }
    }

    bool takable(int w1, int w2, bool l1, bool l2) const {
        return (w1 == 1 || (w1 == 0 && !l1)) && (w2 == 1 || (w2 == 0 && !l2));
    }

    bool leavable(int w1, int w2, bool l1, bool l2) const {
        return (!l1 || w1 != 1) && (!l2 || w2 != 1);
    }

public:
    DegreeZeroOrTwoZdd(Graph const& graph)
            : graph(graph), n(graph.edgeSize()),
              mateSize(graph.maxFrontierSize()) {
        setArraySize(mateSize);
    }

    int getRoot(Mate* mate) const {
        int const v0 = graph.edgeInfo(0).v0;

        for (int k = 0; k < mateSize; ++k) {
            mate[k] = graph.colorNumber(v0 + k) ? 1 : 0;
        }

        return n;
    }

    int getChild(Mate* mate, int level, bool take) const {
        assert(1 <= level && level <= n);
        int i = n - level;
        Graph::EdgeInfo const& e = graph.edgeInfo(i);
        int const w1 = mate[e.v1 - e.v0];
        int const w2 = mate[e.v2 - e.v0];
        assert(e.v1 <= e.v2);

        if (take) {
            if (!takable(w1, w2, e.v1final, e.v2final)) return 0;

            mate[e.v1 - e.v0] = w1 + 1;
            mate[e.v2 - e.v0] = w2 + 1;
        }
        else {
            if (!leavable(w1, w2, e.v1final, e.v2final)) return 0;
        }

        if (e.v1final) mate[e.v1 - e.v0] = 0;
        if (e.v2final) mate[e.v2 - e.v0] = 0;

        if (++i == n) return -1;
        shiftMate(mate, e.v0, graph.edgeInfo(i).v0);

        while (true) {
            Graph::EdgeInfo const& e = graph.edgeInfo(i);
            int const w1 = mate[e.v1 - e.v0];
            int const w2 = mate[e.v2 - e.v0];
            assert(e.v1 <= e.v2);

            if (takable(w1, w2, e.v1final, e.v2final)) break;
            if (!leavable(w1, w2, e.v1final, e.v2final)) return 0;

            if (e.v1final) mate[e.v1 - e.v0] = 0;
            if (e.v2final) mate[e.v2 - e.v0] = 0;

            if (++i == n) return -1;
            shiftMate(mate, e.v0, graph.edgeInfo(i).v0);
        }

        assert(i < n);
        return n - i;
    }
};
