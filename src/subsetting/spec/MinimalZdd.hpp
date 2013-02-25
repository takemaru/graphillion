/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: MinimalZdd.hpp 415 2013-02-22 12:55:13Z iwashita $
 */

#pragma once

#include <cassert>
#include <iostream>
#include <utility>

#include "../dd/DdSpec.hpp"
#include "../util/MemoryPool.hpp"
#include "../util/MyVector.hpp"

struct MinimalZddState {
    struct NodePair {
        DdNodeId case0;
        DdNodeId case1;
    };

    DdNodeId f;
    NodePair* array;
    int size;

    MinimalZddState()
            : f(0), array(0), size(0) {
    }
};

template<typename S>
class MinimalZdd: public ScalarDdSpec<MinimalZdd<S>,MinimalZddState> {
    S& spec;

    MyVector<MemoryPool> pools;

public:
    MinimalZdd(StructuralDdSpec<S>& spec)
            : spec(spec.entity()) {
    }

    int getRoot(MinimalZddState& s) {
        s.f = spec.getRoot();
        return s.f == 1 ? -1 : row;
    }

    int getChild(MinimalZddState& s, int level, bool take) {
        assert(s.f.row == level);
        s.f = spec.getChild(s.f, take);
        return s.f == 1 ? -1 : row;
    }

//    int down(bool take, int fromIndex, int toIndex) {
//        if (fromIndex == 0 && toIndex == f->getIndex() && !take) returntoIndex;
//        assert(fromIndex == f->getIndex());
//
//        for (int i = 0; i < size; ++i) {
//            auto& c0 = array[i].case0;
//            auto& c1 = array[i].case1;
//            if (c0 == 0) continue;
//            assert(c0->getIndex() >= fromIndex);
//            assert(c1->getIndex() >= fromIndex);
//
//            if (c0->getIndex() == fromIndex) {
//                c0 = take ? c0->getChild1() : c0->getChild0();
//            }
//            if (c1->getIndex() == fromIndex) {
//                c1 = take ? c1->getChild1() : c1->getChild0();
//            }
//            if (c0->isConst0() || c1->isConst0()) {
//                c0 = c1 = 0;
//            }
//            else {
//                if (c0 == c1) return 0;
//            }
//        }
//
//        if (take) {
//            auto f0 = f->getChild0();
//            auto f1 = f->getChild1();
//            if (!f0->isConst0() && !f1->isConst0()) {
//                if (f0 == f1) return 0;
//                int i = size++;
//                if (size > capacity) abort();
//                array[i] = NodePair {f0, f1};
//            }
//            f = f1;
//        }
//        else {
//            f = f->getChild0();
//        }
//
//        return toIndex;
//    }
};
