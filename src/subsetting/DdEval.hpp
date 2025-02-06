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

namespace tdzdd {

/**
 * Collection of child node values/levels for
 * DdEval::evalNode function interface.
 * @tparam T data type of work area for each node.
 * @tparam ARITY the number of children for each node.
 */
template<typename T, int ARITY>
class DdValues {
    T const* value[ARITY];
    int level[ARITY];

public:
    /**
     * Returns the value of the b-th child.
     * @param b branch index.
     * @return value of the b-th child.
     */
    T const& get(int b) const {
        assert(0 <= b && b < ARITY);
        return *value[b];
    }

    /**
     * Returns the level of the b-th child.
     * @param b branch index.
     * @return level of the b-th child.
     */
    int getLevel(int b) const {
        assert(0 <= b && b < ARITY);
        return level[b];
    }

    void setReference(int b, T const& v) {
        assert(0 <= b && b < ARITY);
        value[b] = &v;
    }

    void setLevel(int b, int i) {
        assert(0 <= b && b < ARITY);
        level[b] = i;
    }

    friend std::ostream& operator<<(std::ostream& os, DdValues const& o) {
        os << "(";
        for (int b = 0; b < ARITY; ++b) {
            if (b != 0) os << ",";
            os << o.value(b) << "@" << o.level(b);
        }
        return os << ")";
    }
};

/**
 * Base class of DD evaluators.
 *
 * Every implementation must define the following functions:
 * - void evalTerminal(T& v, int id)
 * - void evalNode(T& v, int level, DdValues<T,ARITY> const& values)
 *
 * Optionally, the following functions can be overloaded:
 * - bool showMessages()
 * - void initialize(int level)
 * - R getValue(T const& work)
 * - void destructLevel(int i)
 *
 * @tparam E the class implementing this class.
 * @tparam T data type of work area for each node.
 * @tparam R data type of return value.
 */
template<typename E, typename T, typename R = T>
class DdEval {
public:
    E& entity() {
        return *static_cast<E*>(this);
    }

    E const& entity() const {
        return *static_cast<E const*>(this);
    }

    /**
     * Declares thread-safety.
     * @return true if this class is thread-safe.
     */
    bool isThreadSafe() const {
        return true;
    }

    /**
     * Declares preference to show messages.
     * @return true if messages are preferred.
     */
    bool showMessages() const {
        return false;
    }

    /**
     * Initialization.
     * @param level the maximum level of the DD.
     */
    void initialize(int level) {
    }

    /**
     * Makes a result value.
     * @param v work area value for the root node.
     * @return final value of the evaluation.
     */
    R getValue(T const& v) {
        return R(v);
    }

    /**
     * Destructs i-th level of data storage.
     * @param i the level to be destructerd.
     */
    void destructLevel(int i) {
    }
};

} // namespace tdzdd
