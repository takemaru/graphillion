/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: ToZBDD.hpp 423 2013-02-25 06:53:46Z iwashita $
 */

#pragma once

#ifndef B_64
#if __SIZEOF_POINTER__ == 8
#define B_64
#endif
#endif
#include <SAPPOROBDD/ZBDD.h>

#include "../dd/DdEval.hpp"

struct ToZBDD: public DdEval<ToZBDD,ZBDD> {
    static void evalTerminal(ZBDD& f, bool one) {
        f = ZBDD(one ? 1 : 0);
    }

    static void evalNode(ZBDD& f, int level, ZBDD& f0, int i0, ZBDD& f1, int i1) {
        while (BDD_VarUsed() < level) {
            BDD_NewVar();
        }
        f = f0 + f1.Change(BDD_VarOfLev(level));
    }
};
