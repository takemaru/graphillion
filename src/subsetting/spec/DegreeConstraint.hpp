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
#include <iostream>
#include <map>
#include <vector>

#include "../DdSpec.hpp"
#include "../util/Graph.hpp"
#include "../util/IntSubset.hpp"

namespace tdzdd {

class DegreeConstraint: public PodArrayDdSpec<DegreeConstraint,int16_t,2> {
    typedef int16_t Mate;

    Graph const& graph;
    std::vector<IntSubset const*> constraints;
    int const n;
    int const mateSize;
    bool const lookahead;

    void shiftMate(Mate* mate, int d) const {
        assert(d >= 0);
        if (d > 0) {
            std::memmove(mate, mate + d, (mateSize - d) * sizeof(*mate));
            for (int k = mateSize - d; k < mateSize; ++k) {
                mate[k] = 0;
            }
        }
    }

    bool takable(IntSubset const* c, Mate degree, bool final) const {
        if (c == 0) return true;
        if (degree >= c->upperBound()) return false;
        return !final || c->contains(degree + 1);
    }

    bool leavable(IntSubset const* c, Mate degree, bool final) const {
        if (c == 0) return true;
        return !final || c->contains(degree);
    }

public:
    DegreeConstraint(Graph const& graph, IntSubset const* c = 0, bool lookahead = true)
            : graph(graph), n(graph.edgeSize()),
              mateSize(graph.maxFrontierSize()), lookahead(lookahead) {
        setArraySize(mateSize);

        int m = graph.vertexSize();
        constraints.resize(m + 1);
        for (int v = 1; v <= m; ++v) {
            constraints[v] = c;
        }
    }

    void setConstraint(Graph::VertexNumber v, IntSubset const* c) {
        if (v < 1 || graph.vertexSize() < v) throw std::runtime_error(
                "ERROR: Vertex number is out of range");
        constraints[v] = c;
    }

    void setConstraint(std::string v, IntSubset const* c) {
        constraints[graph.getVertex(v)] = c;
    }

    int getRoot(Mate* mate) const {
        for (int k = 0; k < mateSize; ++k) {
            mate[k] = 0;
        }
        return n;
    }

    int getChild(Mate* mate, int level, int take) const {
        assert(1 <= level && level <= n);
        int i = n - level;
        Graph::EdgeInfo const& e = graph.edgeInfo(i);
        Mate& w1 = mate[e.v1 - e.v0];
        Mate& w2 = mate[e.v2 - e.v0];
        IntSubset const* c1 = constraints[e.v1];
        IntSubset const* c2 = constraints[e.v2];
        assert(e.v1 <= e.v2);

        if (take) {
            if (!takable(c1, w1, e.v1final)) return 0;
            if (!takable(c2, w2, e.v2final)) return 0;
            if (c1) ++w1;
            if (c2) ++w2;
        }
        else {
            if (!leavable(c1, w1, e.v1final)) return 0;
            if (!leavable(c2, w2, e.v2final)) return 0;
        }

        if (e.v1final) w1 = 0;
        if (e.v2final) w2 = 0;

        if (++i == n) return -1;
        shiftMate(mate, graph.edgeInfo(i).v0 - e.v0);

        while (lookahead) {
            Graph::EdgeInfo const& e = graph.edgeInfo(i);
            Mate& w1 = mate[e.v1 - e.v0];
            Mate& w2 = mate[e.v2 - e.v0];
            IntSubset const* c1 = constraints[e.v1];
            IntSubset const* c2 = constraints[e.v2];
            assert(e.v1 <= e.v2);

            if (takable(c1, w1, e.v1final) && takable(c2, w2, e.v2final)) break;
            if (!leavable(c1, w1, e.v1final)) return 0;
            if (!leavable(c2, w2, e.v2final)) return 0;

            if (e.v1final) w1 = 0;
            if (e.v2final) w2 = 0;

            if (++i == n) return -1;
            shiftMate(mate, graph.edgeInfo(i).v0 - e.v0);
        }

        assert(i < n);
        return n - i;
    }
};

} // namespace tdzdd
