/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2013 Japan Science and Technology Agency
 * $Id: IntSubset.hpp 415 2013-02-22 12:55:13Z iwashita $
 */

#pragma once

#include <climits>

struct IntSubset {
    virtual ~IntSubset() {
    }

    virtual bool contains(int x) const = 0;

    virtual int lowerBound() const {
        return 0;
    }

    virtual int upperBound() const {
        return INT_MAX;
    }
};
