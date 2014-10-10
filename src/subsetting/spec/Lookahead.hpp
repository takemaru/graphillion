/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2014 Japan Science and Technology Agency
 * $Id: BinaryOperation.hpp 518 2014-03-18 05:29:23Z iwashita $
 */

#pragma once

#include <cassert>
#include <iostream>
#include <vector>

#include "../dd/DdSpec.hpp"

template<typename S>
class ZddLookahead: public DdSpec<ZddLookahead<S> > {
    typedef S Spec;

    Spec spec;
    std::vector<char> work;

    int lookahead(void* p, int level) {
        void* const q = work.data();
        while (level >= 1) {
            for (int b = 1; b < 2; ++b) {
                spec.get_copy(q, p);
                if (spec.get_child(q, level, b) != 0) {
                    spec.destruct(q);
                    return level;
                }
                spec.destruct(q);
            }
            level = spec.get_child(p, level, 0);
        }

        return level;
    }

public:
    ZddLookahead(S const& s)
            : spec(s), work(spec.datasize()) {
    }

    int datasize() const {
        return spec.datasize();
    }

    int get_root(void* p) {
        return lookahead(p, spec.get_root(p));
    }

    int get_child(void* p, int level, int b) {
        return lookahead(p, spec.get_child(p, level, b));
    }

    void get_copy(void* to, void const* from) {
        spec.get_copy(to, from);
    }

    void destruct(void* p) {
        spec.destruct(p);
    }

    void destructLevel(int level) {
        spec.destructLevel(level);
    }

    size_t hash_code(void const* p) const {
        return spec.hash_code(p);
    }

    bool equal_to(void const* p, void const* q) const {
        return spec.equal_to(p, q);
    }
};
