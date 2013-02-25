/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: MinNumItems.hpp 412 2013-02-15 01:13:19Z iwashita $
 */

#pragma once

#include <climits>

#include "../dd/DdEval.hpp"

struct MinNumItems: public DdEval<MinNumItems,int> {
    static void evalTerminal(int& n, bool one) {
        n = one ? 0 : INT_MAX;
    }

    static void evalNode(int& n, int, int n0, int, int n1, int) {
        n = std::min(n0, n1 + 1);
    }
};
