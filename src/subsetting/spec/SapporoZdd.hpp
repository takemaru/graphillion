/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: SapporoZdd.hpp 426 2013-02-26 06:50:04Z iwashita $
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

/**
 * ZBDD wrapper.
 * ZBDD nodes at level @a i + @p offset are converted to
 * TdZdd nodes at level @a i.
 */
class SapporoZdd: public ScalarDdSpec<SapporoZdd,ZBDD> {
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

    int getChild(ZBDD& f, int level, bool take) const {
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
