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

class IntRange: public IntSubset {
    int const min;
    int const max;
    int const step;

public:
    IntRange(int min = 0, int max = INT_MAX, int step = 1)
            : min(min), max(max), step(step) {
    }

    bool contains(int x) const {
        if (x < min || max < x) return false;
        return (x - min) % step == 0;
    }

    int lowerBound() const {
        return min;
    }

    int upperBound() const {
        return max;
    }
};
