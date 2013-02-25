/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: SapporoZdd.hpp 423 2013-02-25 06:53:46Z iwashita $
 */

#pragma once

#include <cassert>
#include <ostream>

#ifndef B_64
#if __SIZEOF_POINTER__ == 8
#define B_64
#endif
#endif

#include "../dd/DdSpec.hpp"

class SapporoZdd: public ScalarDdSpec<SapporoZdd,ZBDD> {
    ZBDD root;

public:
    SapporoZdd(ZBDD const& f)
            : root(f) {
    }

    int getRoot(ZBDD& f) const {
        f = root;
        return f == 1 ?  -1 : BDD_LevOfVar(f.Top());
    }

    int getChild(ZBDD& f, int level, bool take) const {
        f = take ? f.OnSet0(BDD_VarOfLev(level)) : f.OffSet(BDD_VarOfLev(level));
        assert(BDD_LevOfVar(f.Top()) < level);
        return f == 1 ?  -1 : BDD_LevOfVar(f.Top());
    }

    size_t hashCode(ZBDD const& f) const {
        return const_cast<ZBDD*>(&f)->GetID();
    }
};
