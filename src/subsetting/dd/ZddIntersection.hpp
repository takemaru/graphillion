/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: ZddIntersection.hpp 411 2013-02-14 09:16:45Z iwashita $
 */

#pragma once

#include <cassert>
#include <iostream>

#include "DdSpec.hpp"

template<typename S1, typename S2>
class ZddIntersection: public PodArrayDdSpec<ZddIntersection<S1,S2>,size_t> {
    typedef S1 Spec1;
    typedef S2 Spec2;
    typedef size_t Word;

    Spec1& spec1;
    Spec2& spec2;
    int const stateWords1;
    int const stateWords2;

    static int wordSize(int size) {
        return (size + sizeof(Word) - 1) / sizeof(Word);
    }

    void* state1(void* p) const {
        return p;
    }

    void const* state1(void const* p) const {
        return p;
    }

    void* state2(void* p) const {
        return static_cast<Word*>(p) + stateWords1;
    }

    void const* state2(void const* p) const {
        return static_cast<Word const*>(p) + stateWords1;
    }

public:
    ZddIntersection(S1& s1, S2& s2)
            : spec1(s1), spec2(s2), stateWords1(wordSize(spec1.datasize())),
              stateWords2(wordSize(spec2.datasize())) {
        ZddIntersection::setArraySize(stateWords1 + stateWords2);
    }

    int getRoot(Word* p) {
        int i1 = spec1.get_root(state1(p));
        if (i1 == 0) return 0;
        int i2 = spec2.get_root(state2(p));
        if (i2 == 0) return 0;

        while (i1 != i2) {
            if (i1 > i2) {
                i1 = spec1.get_child(state1(p), i1, 0);
                if (i1 == 0) return 0;
            }
            else {
                i2 = spec2.get_child(state2(p), i2, 0);
                if (i2 == 0) return 0;
            }
        }

        return i1;
    }

    int getChild(Word* p, int level, bool take) {
        int i1 = spec1.get_child(state1(p), level, take);
        if (i1 == 0) return 0;
        int i2 = spec2.get_child(state2(p), level, take);
        if (i2 == 0) return 0;

        while (i1 != i2) {
            if (i1 > i2) {
                i1 = spec1.get_child(state1(p), i1, 0);
                if (i1 == 0) return 0;
            }
            else {
                i2 = spec2.get_child(state2(p), i2, 0);
                if (i2 == 0) return 0;
            }
        }

        return i1;
    }

    void get_copy(void* to, void const* from) {
        spec1.get_copy(state1(to), state1(from));
        spec2.get_copy(state2(to), state2(from));
    }

    void destruct(void* p) {
        spec1.destruct(state1(p));
        spec2.destruct(state2(p));
    }

    void destructLevel(int level) {
        spec1.destructLevel(level);
        spec2.destructLevel(level);
    }

    size_t hash_code(void const* p) const {
        return spec1.hash_code(state1(p)) * 314159257
                + spec2.hash_code(state2(p)) * 271828171;
    }

    bool equal_to(void const* p, void const* q) const {
        return spec1.equal_to(state1(p), state1(q))
                && spec2.equal_to(state2(p), state2(q));
    }

    std::ostream& print(std::ostream& os, void const* p) const {
        Word const* q = static_cast<Word const*>(p);
        os << "[";
        spec1.print(os, state1(q));
        spec2.print(os, state2(q));
        return os << "]";
    }
};

//template<typename S1, typename S2>
//ZddIntersection<S1,S2> zddIntersection(S1 const& spec1, S2 const& spec2) {
//    return ZddIntersection<S1,S2>(spec1, spec2);
//}
