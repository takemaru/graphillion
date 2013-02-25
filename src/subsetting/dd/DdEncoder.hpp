/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: DdEncoder.hpp 412 2013-02-15 01:13:19Z iwashita $
 */

#pragma once

#include <cassert>
#include <ostream>

#include "DdBuilder.hpp"
#include "DdNodeTable.hpp"
#include "DdSpec.hpp"
#include "../util/MyHashTable.hpp"
#include "../util/MyList.hpp"
#include "../util/MyVector.hpp"

/**
 * Wrapper for mapping the spec's state to @p DdNodeId.
 * @param S ZDD spec.
 */
template<typename S>
class DdEncoder: public StructuralDdSpec<DdEncoder<S> > {
    DdNodeTable nodeTable;
    DdNodeId root;
    InstantDdBuilder<S> idb;
    int readyLevel;

    DdEncoder(DdEncoder const& o);
    DdEncoder& operator=(DdEncoder const& o);

public:
    explicit DdEncoder(S& spec)
            : idb(spec, nodeTable), readyLevel(0) {
    }

    DdNodeId getRoot() {
        idb.initialize(root);
        readyLevel = root.row;
        idb.construct(readyLevel);
        return root;
    }

    DdNodeId getChild(DdNodeId f, bool b) {
        assert(0 < f.row && f.row < nodeTable.numRows());

        while (readyLevel > f.row) {
            idb.construct(--readyLevel);
        }

        assert(f.col < nodeTable.rowSize(f.row));
        return nodeTable[f.row][f.col].branch[b];
    }

    void destructLevel(int i) {
        nodeTable.clear(i);
    }
};

//template<typename S>
//class DdEncoder2: public StructuralDdSpec<DdEncoder2<S> > {
//    typedef typename std::remove_const<typename std::remove_reference<S>::type>::type Spec;
//
//    struct Hasher {
//        Spec const& spec;
//
//        Hasher(Spec const& spec)
//                : spec(spec) {
//        }
//
//        size_t operator()(void const* p) const {
//            return spec.hash_code(p);
//        }
//
//        size_t operator()(void const* p, void const* q) const {
//            return spec.equal_to(p, q);
//        }
//    };
//
//    Spec spec;
//    Hasher const hasher;
//    int stateSize;
//
//    MyVector<MyList<char> > stateStorage;
//    MyVector<MyHashMap<void const*,size_t,Hasher,Hasher> > state2code;
//    MyVector<MyVector<void*> > code2state;
//
//public:
////    explicit DdEncoder2(S&& s): spec(std::forward<S>(s)), hasher(spec), stateSize(spec.datasize()) {
////    }
//    explicit DdEncoder2(S const& s)
//            : spec(s), hasher(spec), stateSize(spec.datasize()) {
//    }
//
//    DdNodeId getRoot() {
//        stateStorage.clear();
//        state2code.clear();
//        code2state.clear();
//
//        std::vector<char> tmp(stateSize);
//        void* ptmp = tmp.data();
//        int i = spec.get_root(ptmp);
//
//        if (i > 0) {
//            stateStorage.resize(i + 1);
//            state2code.resize(i + 1, hasher, hasher);
//            code2state.resize(i + 1);
//            void* p = stateStorage[i].alloc_front(stateSize);
//            spec.get_copy(p, ptmp);
//
//            code2state[i].emplace_back(p);
//            state2code[i][p] = code2state[i].size();
//        }
//
//        spec.destruct(ptmp);
//        return (i <= 0) ? DdNodeId(0, -i) : DdNodeId(i, 0);
//    }
//
//    DdNodeId getChild(DdNodeId f, bool b) {
//        int i = f.row;
//        size_t j = f.col;
//        assert(0 < i && size_t(i) < code2state.size());
//        assert(j < code2state[i].size());
//        void* p = code2state[i][j];
//        void* pp = stateStorage[i - 1].alloc_front(stateSize);
//        spec.get_copy(pp, p);
//        int ii = spec.get_child(pp, i, b);
//
//        if (ii <= 0) {
//            spec.destruct(pp);
//            stateStorage[i - 1].pop_front();
//            return DdNodeId(0, -ii);
//        }
//
//        if (ii < i - 1) {
//            void* tmp = pp;
//            pp = stateStorage[ii].alloc_front(stateSize);
//            spec.get_copy(pp, tmp);
//            spec.destruct(tmp);
//            stateStorage[i - 1].pop_front();
//        }
//
//        size_t& codePlus1 = state2code[ii][pp];
//        if (codePlus1 == 0) {
//            code2state[ii].emplace_back(pp);
//            codePlus1 = code2state[ii].size();
//        }
//        else {
//            spec.destruct(pp);
//            stateStorage[ii].pop_front();
//        }
//        size_t jj = codePlus1 - 1;
//
//        return DdNodeId(ii, jj);
//    }
//
//    void destructLevel(int i) {
//        for (MyList<char>& ss = stateStorage[i]; !ss.empty(); ss.pop_front()) {
//            spec.destruct(ss.front());
//        }
//        state2code[i].clear();
//        code2state[i].clear();
//    }
//};

//template<typename S>
//DdEncoder<S> encode(S const& spec) {
//    return DdEncoder<S>(spec);
//}
