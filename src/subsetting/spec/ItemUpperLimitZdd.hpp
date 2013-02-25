/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: ItemUpperLimitZdd.hpp 414 2013-02-15 04:04:57Z iwashita $
 */

#pragma once

#include <cassert>
#include <iostream>

#include "../dd/DdSpec.hpp"

class ItemUpperLimitZdd: public ScalarDdSpec<ItemUpperLimitZdd,int> {
    int const n;
    int const limit;

public:
    ItemUpperLimitZdd(int n, int limit)
            : n(n), limit(limit) {
        assert(n >= 1);
    }

    int getRoot(int& state) const {
        state = limit;
        return (state < 0) ? 0 : (state == 0) ? -1 : n;
    }

    int getChild(int& state, int level, bool b) const {
        assert(state >= 1);
        if (b) {
            --state;
            if (state == 0) return -1;
        }
        --level;
        if (state > level) state = level;
        return (level >= 1) ? level : -1;
    }
};
