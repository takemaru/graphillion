/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: DdEval.hpp 410 2013-02-14 06:33:04Z iwashita $
 */

#pragma once

#include <cassert>
#include <iostream>

/*
 * Every implementation must have the following functions:
 * - void evalTerminal(T& v, bool one);
 * - void evalNode(T& v, int i, T const& v0, int i0, T const& v1, int i1);
 */

/**
 * Base class of DD evaluatores.
 * @param S the class implementing this class.
 * @param T data type of work area for each node.
 * @param R data type of return value.
 */
template<typename S, typename T, typename R = T>
struct DdEval {
    typedef S Self;
    typedef T Val;
    typedef R RetVal;

    S& entity() {
        return *static_cast<S*>(this);
    }

    S const& entity() const {
        return *static_cast<S const*>(this);
    }

    /**
     * Initialization.
     * @param level the maximum level of the DD.
     */
    void initialize(int level) {
    }

    /**
     * Makes a result value.
     */
    R getValue(T const& work) {
        return R(work);
    }

    /**
     * Destructs i-th level of data storage.
     * @param i the level to be destructerd.
     */
    void destructLevel(int i) {
    }
};
