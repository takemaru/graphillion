/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: NAryZdd.hpp 414 2013-02-15 04:04:57Z iwashita $
 */

#pragma once

#include <cassert>
#include <iostream>

#include "../dd/DdSpec.hpp"

template<bool oneHot>
class NAryZddBase: public StatelessDdSpec<NAryZddBase<oneHot> > {
    int const m;
    int const topLevel;

public:
    NAryZddBase(int m, int n)
            : m(m), topLevel(m * n) {
        assert(m >= 1);
        assert(n >= 1);
    }

    int getRoot() const {
        return topLevel;
    }

    int getChild(int level, bool take) const {
        if (take) {
            level = ((level - 1) / m) * m;
            if (level == 0) return -1;
        }
        else {
            --level;
            if (oneHot && level % m == 0) return 0;
        }
        return level;
    }
};

struct NAryZdd: public NAryZddBase<false> {
    NAryZdd(int arity, int length)
            : NAryZddBase<false>(arity - 1, length) {
    }
};

struct OneHotNAryZdd: public NAryZddBase<true> {
    OneHotNAryZdd(int arity, int length)
            : NAryZddBase<true>(arity, length) {
    }
};
