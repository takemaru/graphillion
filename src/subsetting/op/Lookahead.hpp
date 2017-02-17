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
#include <vector>

#include "../DdSpec.hpp"

namespace tdzdd {

template<typename S>
class BddLookahead: public DdSpecBase<BddLookahead<S>,S::ARITY> {
    typedef S Spec;

    Spec spec;
    std::vector<char> work0;
    std::vector<char> work1;

    int lookahead(void* p, int level) {
        while (level >= 1) {
            spec.get_copy(work0.data(), p);
            int level0 = spec.get_child(work0.data(), level, 0);

            for (int b = 1; b < Spec::ARITY; ++b) {
                spec.get_copy(work1.data(), p);
                int level1 = spec.get_child(work1.data(), level, b);
                if (!(level0 == level1
                        && (level0 <= 0
                                || spec.equal_to(work0.data(), work1.data(),
                                        level0)))) {
                    spec.destruct(work0.data());
                    spec.destruct(work1.data());
                    return level;
                }
                spec.destruct(work1.data());
            }

            spec.destruct(p);
            spec.get_copy(p, work0.data());
            spec.destruct(work0.data());
            level = level0;
        }

        return level;
    }

public:
    BddLookahead(S const& s)
            : spec(s), work0(spec.datasize()), work1(spec.datasize()) {
    }

    int datasize() const {
        return spec.datasize();
    }

    int get_root(void* p) {
        return lookahead(p, spec.get_root(p));
    }

    int get_child(void* p, int level, int b) {
        return lookahead(p, spec.get_child(p, level, b));
    }

    void get_copy(void* to, void const* from) {
        spec.get_copy(to, from);
    }

    int merge_states(void* p1, void* p2) {
        return spec.merge_states(p1, p2);
    }

    void destruct(void* p) {
        spec.destruct(p);
    }

    void destructLevel(int level) {
        spec.destructLevel(level);
    }

    size_t hash_code(void const* p, int level) const {
        return spec.hash_code(p, level);
    }

    bool equal_to(void const* p, void const* q, int level) const {
        return spec.equal_to(p, q, level);
    }

    void print_state(std::ostream& os, void const* p, int level) const {
        spec.print_state(os, p, level);
    }

    void print_level(std::ostream& os, int level) const {
        spec.print_level(os, level);
    }
};

template<typename S>
class ZddLookahead: public DdSpecBase<ZddLookahead<S>,S::ARITY> {
    typedef S Spec;

    Spec spec;
    std::vector<char> work;

    int lookahead(void* p, int level) {
        void* const q = work.data();
        while (level >= 1) {
            for (int b = 1; b < Spec::ARITY; ++b) {
                spec.get_copy(q, p);
                if (spec.get_child(q, level, b) != 0) {
                    spec.destruct(q);
                    return level;
                }
                spec.destruct(q);
            }
            level = spec.get_child(p, level, 0);
        }

        return level;
    }

public:
    ZddLookahead(S const& s)
            : spec(s), work(spec.datasize()) {
    }

    int datasize() const {
        return spec.datasize();
    }

    int get_root(void* p) {
        return lookahead(p, spec.get_root(p));
    }

    int get_child(void* p, int level, int b) {
        return lookahead(p, spec.get_child(p, level, b));
    }

    void get_copy(void* to, void const* from) {
        spec.get_copy(to, from);
    }

    int merge_states(void* p1, void* p2) {
        return spec.merge_states(p1, p2);
    }

    void destruct(void* p) {
        spec.destruct(p);
    }

    void destructLevel(int level) {
        spec.destructLevel(level);
    }

    size_t hash_code(void const* p, int level) const {
        return spec.hash_code(p, level);
    }

    bool equal_to(void const* p, void const* q, int level) const {
        return spec.equal_to(p, q, level);
    }

    void print_state(std::ostream& os, void const* p, int level) const {
        spec.print_state(os, p, level);
    }

    void print_level(std::ostream& os, int level) const {
        spec.print_level(os, level);
    }
};

} // namespace tdzdd
