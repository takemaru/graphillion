/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: ItemLowerLimitZdd.hpp 414 2013-02-15 04:04:57Z iwashita $
 */

#pragma once

#include <cassert>
#include <iostream>

#include "../dd/DdSpec.hpp"

class ItemLowerLimitZdd: public ScalarDdSpec<ItemLowerLimitZdd,int> {
    int const n;
    int const limit;

public:
    ItemLowerLimitZdd(int n, int limit)
            : n(n), limit(limit) {
        assert(n >= 1);
    }

    int getRoot(int& state) const {
        state = limit;
        return (n < state) ? 0 : n;
    }

    int getChild(int& state, int level, bool b) const {
        assert(state >= 0);
        if (b) {
            if (state > 0) --state;
        }
        else {
            if (state >= level) return 0;
        }
        --level;
        return (level >= 1) ? level : -1;
    }
};
