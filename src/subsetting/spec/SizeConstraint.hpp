/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: SizeConstraint.hpp 420 2013-02-25 05:30:06Z iwashita $
 */

#pragma once

#include <cassert>
#include <iostream>

#include "../dd/DdSpec.hpp"
#include "../util/IntSubset.hpp"

class SizeConstraint: public ScalarDdSpec<SizeConstraint,int> {
    int const n;
    IntSubset const* const constraint;

public:
    SizeConstraint(int n, IntSubset const* constraint)
            : n(n), constraint(constraint) {
        assert(n >= 1);
    }

    int getRoot(int& count) const {
        count = 0;
        return (constraint && n < constraint->lowerBound()) ? 0 : n;
    }

    int getChild(int& count, int level, bool take) const {
        if (constraint == 0) return (--level >= 1) ? level : -1;

        if (take) {
            if (count >= constraint->upperBound()) return 0;
            ++count;
        }
        else {
            if (count + level <= constraint->lowerBound()) return 0;
        }

        return (--level >= 1) ? level : constraint->contains(count) ? -1 : 0;
    }
};
