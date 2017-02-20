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

#ifndef B_64
#if __SIZEOF_POINTER__ == 8
#define B_64
#endif
#endif

#include "../DdEval.hpp"

/**
 * Exporter to ZBDD.
 * TdZdd nodes at level @a i are converted to
 * ZBDD nodes at level @a i + @p offset.
 * When the ZBDD variables are not enough, they are
 * created automatically by BDD_NewVar().
 */
struct ToZBDD: public tdzdd::DdEval<ToZBDD,ZBDD> {
    int const offset;

public:
    ToZBDD(int offset = 0)
            : offset(offset) {
    }

    void initialize(int topLevel) const {
        while (BDD_VarUsed() < topLevel + offset) {
            BDD_NewVar();
        }
    }

    void evalTerminal(ZBDD& f, int value) const {
        f = ZBDD(value);
    }

    void evalNode(ZBDD& f, int level, tdzdd::DdValues<ZBDD,2> const& values) const {
        f = values.get(0);
        if (level + offset > 0) {
            ZBDD f1 = values.get(1);
            f += f1.Change(BDD_VarOfLev(level + offset));
        }
    }
};
