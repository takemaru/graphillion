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

#include <climits>

namespace tdzdd {

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

} // namespace tdzdd
