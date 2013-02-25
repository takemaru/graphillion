/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: DdSpecExamples.hpp 414 2013-02-15 04:04:57Z iwashita $
 */

#pragma once

#include <cassert>
#include <map>

#include "../dd/DdSpec.hpp"
#include "../util/Graph.hpp"

struct SimpathZddByMap: public ScalarDdSpec<SimpathZddByMap,
        std::map<int16_t,int16_t> > {

    Graph const& graph;
    int const n;

public:
    SimpathZddByMap(Graph const& graph)
            : graph(graph), n(graph.edgeSize()) {
    }

    int getRoot(State& mate) const {
        for (Graph::VertexNumber v = 1; v <= graph.vertexSize(); ++v) {
            Graph::VertexNumber w = graph.virtualMate(v);
            if (w >= 1) mate[v] = w;
        }
        return n;
    }

    int getChild(State& mate, int level, bool take) const {
        Graph::EdgeInfo const& e = graph.edgeInfo(n - level);
        int const v1 = e.v1;
        int const v2 = e.v2;
        State::iterator t1 = mate.find(v1);
        State::iterator t2 = mate.find(v2);
        bool const u1 = (t1 == mate.end());
        bool const u2 = (t2 == mate.end());
        int const w1 = u1 ? v1 : t1->second;
        int const w2 = u2 ? v2 : t2->second;

        if (take) {
            if (w1 == 0 || w2 == 0) return 0;
            if (e.v1final && u1) return 0;
            if (e.v2final && u2) return 0;

            if (w1 == v2) {
                assert(w2 == v1);
                for (State::const_iterator t = mate.begin(); t != mate.end();
                        ++t) {
                    if (t->first != v1 && t->first != v2 && t->second > 0) return 0;
                }
                return -1;
            }

            mate[v1] = mate[v2] = 0;
            mate[w1] = w2, mate[w2] = w1;
        }

        if (e.v1final && !u1) {
            if (mate[v1] != 0) return 0;
            mate.erase(v1);
        }

        if (e.v2final && !u2) {
            if (mate[v2] != 0) return 0;
            mate.erase(v2);
        }

        return level - 1;
    }

    size_t hashCode(State const& mate) const {
        size_t h = 0;
        for (State::const_iterator t = mate.begin(); t != mate.end();
                ++t) {
            h += t->first;
            h *= 314159257;
            h += t->second;
            h *= 271828171;
        }
        return h;
    }
};
