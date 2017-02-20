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
#include <ostream>

#ifndef B_64
#if __SIZEOF_POINTER__ == 8
#define B_64
#endif
#endif

#include "../DdSpec.hpp"

/**
 * ZBDD wrapper.
 * ZBDD nodes at level @a i + @p offset are converted to
 * TdZdd nodes at level @a i.
 */
class SapporoZdd: public tdzdd::DdSpec<SapporoZdd,ZBDD,2> {
    ZBDD const root;
    int const offset;

    int var2level(int var) const {
        return BDD_LevOfVar(var) - offset;
    }

    int level2var(int level) const {
        return BDD_VarOfLev(level + offset);
    }

public:
    SapporoZdd(ZBDD const& f, int offset = 0)
            : root(f), offset(offset) {
    }

    int getRoot(ZBDD& f) const {
        f = root;

        int level = BDD_LevOfVar(f.Top()) - offset;
        if (level >= 1) return level;

        while (BDD_LevOfVar(f.Top()) >= 1) {
            f = f.OffSet(BDD_VarOfLev(f.Top()));
        }
        return (f == 1) ? -1 : 0;
    }

    int getChild(ZBDD& f, int level, int take) const {
        int var = BDD_VarOfLev(level + offset);
        f = take ? f.OnSet0(var) : f.OffSet(var);

        int nextLevel = BDD_LevOfVar(f.Top()) - offset;
        assert(nextLevel < level);
        if (nextLevel >= 1) return nextLevel;

        while (BDD_LevOfVar(f.Top()) >= 1) {
            f = f.OffSet(BDD_VarOfLev(f.Top()));
        }
        return (f == 1) ? -1 : 0;
    }

    size_t hashCode(ZBDD const& f) const {
        return const_cast<ZBDD*>(&f)->GetID();
    }
};
