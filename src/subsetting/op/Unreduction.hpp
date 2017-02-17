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

#include "../DdSpec.hpp"

namespace tdzdd {

template<typename T, typename S, bool ZDD>
class Unreduction_: public PodArrayDdSpec<T,size_t,S::ARITY> {
protected:
    typedef S Spec;
    typedef size_t Word;

    static size_t const levelWords = (sizeof(int) + sizeof(Word) - 1)
            / sizeof(Word);

    Spec spec;
    int const stateWords;
    int numVars;

    static int wordSize(int size) {
        return (size + sizeof(Word) - 1) / sizeof(Word);
    }

    int& level(void* p) const {
        return *static_cast<int*>(p);
    }

    int level(void const* p) const {
        return *static_cast<int const*>(p);
    }

    void* state(void* p) const {
        return static_cast<Word*>(p) + levelWords;
    }

    void const* state(void const* p) const {
        return static_cast<Word const*>(p) + levelWords;
    }

public:
    Unreduction_(S const& s, int numVars)
            : spec(s), stateWords(wordSize(spec.datasize())), numVars(numVars) {
        Unreduction_::setArraySize(levelWords + stateWords);
    }

    int getRoot(Word* p) {
        level(p) = spec.get_root(state(p));
        if (level(p) == 0) return 0;
        if (level(p) >= numVars) numVars = level(p);
        return (numVars > 0) ? numVars : -1;
    }

    int getChild(Word* p, int i, int value) {
        if (level(p) == i) {
            level(p) = spec.get_child(state(p), i, value);
            if (level(p) == 0) return 0;
        }
        else if (ZDD && value) {
            return 0;
        }

        --i;
        assert(level(p) <= i);
        return (i > 0) ? i : level(p);
    }

    void get_copy(void* to, void const* from) {
        level(to) = level(from);
        spec.get_copy(state(to), state(from));
    }

    void destruct(void* p) {
        spec.destruct(state(p));
    }

    void destructLevel(int level) {
        spec.destructLevel(level);
    }

    int merge_states(void* p1, void* p2) {
        return spec.merge_states(state(p1), state(p2));
    }

    size_t hash_code(void const* p, int i) const {
        size_t h = size_t(level(p)) * 314159257;
        if (level(p) > 0) h += spec.hash_code(state(p), level(p)) * 271828171;
        return h;
    }

    bool equal_to(void const* p, void const* q, int i) const {
        if (level(p) != level(q)) return false;
        if (level(p) > 0 && !spec.equal_to(state(p), state(q), level(p))) return false;
        return true;
    }

    void print_state(std::ostream& os, void const* p, int l) const {
        Word const* q = static_cast<Word const*>(p);
        os << "<" << level(q) << ",";
        spec.print_state(os, state(q), l);
        os << ">";
    }
};

template<typename S>
struct BddUnreduction: public Unreduction_<BddUnreduction<S>,S,false> {
    BddUnreduction(S const& s, int numVars)
            : Unreduction_<BddUnreduction<S>,S,false>(s, numVars) {
    }
};

template<typename S>
struct ZddUnreduction: public Unreduction_<ZddUnreduction<S>,S,true> {
    ZddUnreduction(S const& s, int numVars)
            : Unreduction_<ZddUnreduction<S>,S,true>(s, numVars) {
    }
};

} // namespace tdzdd
