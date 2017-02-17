/*
 * TdZdd: a Top-down/Breadth-first Decision Diagram Manipulation Framework
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2014 ERATO MINATO Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <cassert>
#include <iostream>

#include "../DdSpec.hpp"
#include "../util/IntSubset.hpp"

namespace tdzdd {

class SizeConstraint: public DdSpec<SizeConstraint,int,2> {
    int const n;
    IntSubset const* const constraint;

public:
    SizeConstraint(int n, IntSubset const& constraint)
            : n(n), constraint(&constraint) {
        assert(n >= 1);
    }

    SizeConstraint(int n, IntSubset const* constraint)
            : n(n), constraint(constraint) {
        assert(n >= 1);
    }

    int getRoot(int& count) const {
        count = 0;
        return (constraint && n < constraint->lowerBound()) ? 0 : n;
    }

    int getChild(int& count, int level, int value) const {
        if (constraint == 0) return (--level >= 1) ? level : -1;

        if (value) {
            if (count >= constraint->upperBound()) return 0;
            ++count;
        }
        else {
            if (count + level <= constraint->lowerBound()) return 0;
        }

        return (--level >= 1) ? level : constraint->contains(count) ? -1 : 0;
    }
};

} // namespace tdzdd
