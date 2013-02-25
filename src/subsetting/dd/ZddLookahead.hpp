/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: ZddLookahead.hpp 414 2013-02-15 04:04:57Z iwashita $
 */

#pragma once

#include <cassert>
#include <iostream>
#include <vector>

#include "DdSpec.hpp"

template<typename S>
class ZddLookahead: public DdSpec<ZddLookahead<S> > {
    //typedef typename std::remove_const<typename std::remove_reference<S>::type>::type Spec;
    typedef S Spec;

    Spec& spec;
    std::vector<char> work;

public:
    ZddLookahead(S& s)
            : spec(s) {
    }

    int datasize() const {
        return spec.datasize();
    }

    int get_root(void* p) {
        return spec.get_root(p);
    }

    int get_child(void* p, int level, bool b) {
        level = spec.get_child(p, level, b);

        while (level >= 1) {
            work.resize(spec.datasize());
            spec.get_copy(work.data(), p);
            if (spec.get_child(work.data(), level, true) != 0) break;
            level = spec.get_child(p, level, false);
        }

        return level;
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

//template<typename S>
//ZddLookahead<S> zddLookahead(S const& spec) {
//    return ZddLookahead<S>(spec);
//}
