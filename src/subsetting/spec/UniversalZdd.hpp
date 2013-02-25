/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: UniversalZdd.hpp 412 2013-02-15 01:13:19Z iwashita $
 */

#pragma once

#include <cassert>
#include <iostream>

#include "../dd/DdSpec.hpp"

class UniversalZdd: public StatelessDdSpec<UniversalZdd> {
    int const n;

public:
    UniversalZdd(int n)
            : n(n) {
    }

    int getRoot() const {
        return n;
    }

    int getChild(int level, bool b) const {
        --level;
        return (level >= 1) ? level : -1;
    }
};
