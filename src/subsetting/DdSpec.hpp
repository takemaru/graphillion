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
#include <stdexcept>

#include "dd/DdBuilder.hpp"
#include "dd/DepthFirstSearcher.hpp"
#include "util/demangle.hpp"
#include "util/MessageHandler.hpp"

namespace tdzdd {

/**
 * Base class of DD specs.
 *
 * Every implementation must have the following functions:
 * - int datasize() const
 * - int get_root(void* p)
 * - int get_child(void* p, int level, int value)
 * - void get_copy(void* to, void const* from)
 * - int merge_states(void* p1, void* p2)
 * - void destruct(void* p)
 * - void destructLevel(int level)
 * - size_t hash_code(void const* p, int level) const
 * - bool equal_to(void const* p, void const* q, int level) const
 * - void print_state(std::ostream& os, void const* p, int level) const
 *
 * Optionally, the following functions can be overloaded:
 * - void printLevel(std::ostream& os, int level) const
 *
 * A return code of get_root(void*) or get_child(void*, int, bool) is:
 * 0 when the node is the 0-terminal, -1 when it is the 1-terminal, or
 * the node level when it is a non-terminal.
 * A return code of merge_states(void*, void*) is: 0 when the states are
 * merged into the first one, 1 when they cannot be merged and the first
 * one should be forwarded to the 0-terminal, 2 when they cannot be merged
 * and the second one should be forwarded to the 0-terminal.
 *
 * @tparam S the class implementing this class.
 * @tparam AR arity of the nodes.
 */
template<typename S, int AR>
class DdSpecBase {
public:
    static int const ARITY = AR;

    S& entity() {
        return *static_cast<S*>(this);
    }

    S const& entity() const {
        return *static_cast<S const*>(this);
    }

    void printLevel(std::ostream& os, int level) const {
        os << level;
    }

    /**
     * Returns a random instance using simple depth-first search
     * without caching.
     * It does not guarantee that the selection is uniform.
     * merge_states(void*, void*) is not supported.
     * @return a collection of (item, value) pairs.
     * @exception std::runtime_error no instance exists.
     */
    std::vector<std::pair<int,int> > findOneInstance() const {
        return DepthFirstSearcher<S>(entity()).findOneInstance();
    }

public:
    /**
     * Dumps the node table in Graphviz (dot) format.
     * @param os the output stream.
     * @param title title label.
     */
    void dumpDot(std::ostream& os = std::cout, std::string title =
            typenameof<S>()) const {
        DdDumper<S> dumper(entity());
        dumper.dump(os, title);
    }

    /**
     * Dumps the node table in Graphviz (dot) format.
     * @param os the output stream.
     * @param o the ZDD.
     * @return os itself.
     */
    friend std::ostream& operator<<(std::ostream& os, DdSpecBase const& o) {
        o.dumpDot(os);
        return os;
    }

private:
    template<typename T, typename I>
    static size_t rawHashCode_(void const* p) {
        size_t h = 0;
        I const* a = static_cast<I const*>(p);
        for (size_t i = 0; i < sizeof(T) / sizeof(I); ++i) {
            h += a[i];
            h *= 314159257;
        }
        return h;
    }

    template<typename T, typename I>
    static size_t rawEqualTo_(void const* p1, void const* p2) {
        I const* a1 = static_cast<I const*>(p1);
        I const* a2 = static_cast<I const*>(p2);
        for (size_t i = 0; i < sizeof(T) / sizeof(I); ++i) {
            if (a1[i] != a2[i]) return false;
        }
        return true;
    }

protected:
    template<typename T>
    static size_t rawHashCode(T const& o) {
        if (sizeof(T) % sizeof(size_t) == 0) {
            return rawHashCode_<T,size_t>(&o);
        }
        if (sizeof(T) % sizeof(unsigned int) == 0) {
            return rawHashCode_<T,unsigned int>(&o);
        }
        if (sizeof(T) % sizeof(unsigned short) == 0) {
            return rawHashCode_<T,unsigned short>(&o);
        }
        return rawHashCode_<T,unsigned char>(&o);
    }

    template<typename T>
    static size_t rawEqualTo(T const& o1, T const& o2) {
        if (sizeof(T) % sizeof(size_t) == 0) {
            return rawEqualTo_<T,size_t>(&o1, &o2);
        }
        if (sizeof(T) % sizeof(unsigned int) == 0) {
            return rawEqualTo_<T,unsigned int>(&o1, &o2);
        }
        if (sizeof(T) % sizeof(unsigned short) == 0) {
            return rawEqualTo_<T,unsigned short>(&o1, &o2);
        }
        return rawEqualTo_<T,unsigned char>(&o1, &o2);
    }
};

/**
 * Abstract class of DD specifications without states.
 *
 * Every implementation must have the following functions:
 * - int getRoot()
 * - int getChild(int level, int value)
 *
 * Optionally, the following functions can be overloaded:
 * - void printLevel(std::ostream& os, int level) const
 *
 * @tparam S the class implementing this class.
 * @tparam AR arity of the nodes.
 */
template<typename S, int AR>
class StatelessDdSpec: public DdSpecBase<S,AR> {
public:
    int datasize() const {
        return 0;
    }

    int get_root(void* p) {
        return this->entity().getRoot();
    }

    int get_child(void* p, int level, int value) {
        assert(0 <= value && value < S::ARITY);
        return this->entity().getChild(level, value);
    }

    void get_copy(void* to, void const* from) {
    }

    int merge_states(void* p1, void* p2) {
        return 0;
    }

    void destruct(void* p) {
    }

    void destructLevel(int level) {
    }

    size_t hash_code(void const* p, int level) const {
        return 0;
    }

    bool equal_to(void const* p, void const* q, int level) const {
        return true;
    }

    void print_state(std::ostream& os, void const* p, int level) const {
        os << "*";
    }
};

/**
 * Abstract class of DD specifications using scalar states.
 *
 * Every implementation must have the following functions:
 * - int getRoot(T& state)
 * - int getChild(T& state, int level, int value)
 *
 * Optionally, the following functions can be overloaded:
 * - void construct(void* p)
 * - void getCopy(void* p, T const& state)
 * - void mergeStates(T& state1, T& state2)
 * - size_t hashCode(T const& state) const
 * - bool equalTo(T const& state1, T const& state2) const
 * - void printLevel(std::ostream& os, int level) const
 * - void printState(std::ostream& os, State const& s) const
 *
 * @tparam S the class implementing this class.
 * @tparam T data type.
 * @tparam AR arity of the nodes.
 */
template<typename S, typename T, int AR>
class DdSpec: public DdSpecBase<S,AR> {
public:
    typedef T State;

private:
    static State& state(void* p) {
        return *static_cast<State*>(p);
    }

    static State const& state(void const* p) {
        return *static_cast<State const*>(p);
    }

public:
    int datasize() const {
        return sizeof(State);
    }

    void construct(void* p) {
        new (p) State();
    }

    int get_root(void* p) {
        this->entity().construct(p);
        return this->entity().getRoot(state(p));
    }

    int get_child(void* p, int level, int value) {
        assert(0 <= value && value < S::ARITY);
        return this->entity().getChild(state(p), level, value);
    }

    void getCopy(void* p, State const& s) {
        new (p) State(s);
    }

    void get_copy(void* to, void const* from) {
        this->entity().getCopy(to, state(from));
    }

    int mergeStates(State& s1, State& s2) {
        return 0;
    }

    int merge_states(void* p1, void* p2) {
        return this->entity().mergeStates(state(p1), state(p2));
    }

    void destruct(void* p) {
        state(p).~State();
    }

    void destructLevel(int level) {
    }

    size_t hashCode(State const& s) const {
        return this->rawHashCode(s);
    }

    size_t hashCodeAtLevel(State const& s, int level) const {
        return this->entity().hashCode(s);
    }

    size_t hash_code(void const* p, int level) const {
        return this->entity().hashCodeAtLevel(state(p), level);
    }

    bool equalTo(State const& s1, State const& s2) const {
        return this->rawEqualTo(s1, s2);
    }

    bool equalToAtLevel(State const& s1, State const& s2, int level) const {
        return this->entity().equalTo(s1, s2);
    }

    bool equal_to(void const* p, void const* q, int level) const {
        return this->entity().equalToAtLevel(state(p), state(q), level);
    }

    void printState(std::ostream& os, State const& s) const {
        os << s;
    }

    void printStateAtLevel(std::ostream& os, State const& s, int level) const {
        this->entity().printState(os, s);
    }

    void print_state(std::ostream& os, void const* p, int level) const {
        this->entity().printStateAtLevel(os, state(p), level);
    }
};

/**
 * Abstract class of DD specifications using POD array states.
 * The size of array must be set by setArraySize(int n) in the constructor
 * and cannot be changed.
 * If you want some arbitrary-sized data storage for states,
 * use pointers to those storages in DdSpec instead.
 *
 * Every implementation must have the following functions:
 * - int getRoot(T* array)
 * - int getChild(T* array, int level, int value)
 *
 * Optionally, the following functions can be overloaded:
 * - void mergeStates(T* array1, T* array2)
 * - void printLevel(std::ostream& os, int level) const
 * - void printState(std::ostream& os, State const* array) const
 *
 * @tparam S the class implementing this class.
 * @tparam T data type of array elements.
 * @tparam AR arity of the nodes.
 */
template<typename S, typename T, int AR>
class PodArrayDdSpec: public DdSpecBase<S,AR> {
public:
    typedef T State;

private:
    typedef size_t Word;

    int arraySize;
    int dataWords;

    static State* state(void* p) {
        return static_cast<State*>(p);
    }

    static State const* state(void const* p) {
        return static_cast<State const*>(p);
    }

protected:
    void setArraySize(int n) {
        assert(0 <= n);
        if (arraySize >= 0)
            throw std::runtime_error(
                    "Cannot set array size twice; use setArraySize(int) only once in the constructor of DD spec.");
        arraySize = n;
        dataWords = (n * sizeof(State) + sizeof(Word) - 1) / sizeof(Word);
    }

    int getArraySize() const {
        return arraySize;
    }

public:
    PodArrayDdSpec() :
            arraySize(-1), dataWords(-1) {
    }

    int datasize() const {
        if (dataWords < 0)
            throw std::runtime_error(
                    "Array size is unknown; please set it by setArraySize(int) in the constructor of DD spec.");
        return dataWords * sizeof(Word);
    }

    int get_root(void* p) {
        return this->entity().getRoot(state(p));
    }

    int get_child(void* p, int level, int value) {
        assert(0 <= value && value < S::ARITY);
        return this->entity().getChild(state(p), level, value);
    }

    void get_copy(void* to, void const* from) {
        Word const* pa = static_cast<Word const*>(from);
        Word const* pz = pa + dataWords;
        Word* qa = static_cast<Word*>(to);
        while (pa != pz) {
            *qa++ = *pa++;
        }
    }

    int mergeStates(T* a1, T* a2) {
        return 0;
    }

    int merge_states(void* p1, void* p2) {
        return this->entity().mergeStates(state(p1), state(p2));
    }

    void destruct(void* p) {
    }

    void destructLevel(int level) {
    }

    size_t hash_code(void const* p, int level) const {
        Word const* pa = static_cast<Word const*>(p);
        Word const* pz = pa + dataWords;
        size_t h = 0;
        while (pa != pz) {
            h += *pa++;
            h *= 314159257;
        }
        return h;
    }

    bool equal_to(void const* p, void const* q, int level) const {
        Word const* pa = static_cast<Word const*>(p);
        Word const* qa = static_cast<Word const*>(q);
        Word const* pz = pa + dataWords;
        while (pa != pz) {
            if (*pa++ != *qa++) return false;
        }
        return true;
    }

    void printState(std::ostream& os, State const* a) const {
        os << "[";
        for (int i = 0; i < arraySize; ++i) {
            if (i > 0) os << ",";
            os << a[i];
        }
        os << "]";
    }

    void printStateAtLevel(std::ostream& os, State const* a, int level) const {
        this->entity().printState(os, a);
    }

    void print_state(std::ostream& os, void const* p, int level) const {
        this->entity().printStateAtLevel(os, state(p), level);
    }
};

/**
 * Abstract class of DD specifications using both scalar and POD array states.
 *
 * Every implementation must have the following functions:
 * - int getRoot(TS& scalar, TA* array)
 * - int getChild(TS& scalar, TA* array, int level, int value)
 *
 * Optionally, the following functions can be overloaded:
 * - void construct(void* p)
 * - void getCopy(void* p, TS const& state)
 * - void mergeStates(TS& s1, TA* a1, TS& s2, TA* a2)
 * - size_t hashCode(TS const& state) const
 * - bool equalTo(TS const& state1, TS const& state2) const
 * - void printLevel(std::ostream& os, int level) const
 * - void printState(std::ostream& os, TS const& s, TA const* a) const
 *
 * @tparam S the class implementing this class.
 * @tparam TS data type of scalar.
 * @tparam TA data type of array elements.
 * @tparam AR arity of the nodes.
 */
template<typename S, typename TS, typename TA, int AR>
class HybridDdSpec: public DdSpecBase<S,AR> {
public:
    typedef TS S_State;
    typedef TA A_State;

private:
    typedef size_t Word;
    static int const S_WORDS = (sizeof(S_State) + sizeof(Word) - 1)
            / sizeof(Word);

    int arraySize;
    int dataWords;

    static S_State& s_state(void* p) {
        return *static_cast<S_State*>(p);
    }

    static S_State const& s_state(void const* p) {
        return *static_cast<S_State const*>(p);
    }

    static A_State* a_state(void* p) {
        return reinterpret_cast<A_State*>(static_cast<Word*>(p) + S_WORDS);
    }

    static A_State const* a_state(void const* p) {
        return reinterpret_cast<A_State const*>(static_cast<Word const*>(p)
                + S_WORDS);
    }

protected:
    void setArraySize(int n) {
        assert(0 <= n);
        arraySize = n;
        dataWords = S_WORDS
                + (n * sizeof(A_State) + sizeof(Word) - 1) / sizeof(Word);
    }

    int getArraySize() const {
        return arraySize;
    }

public:
    HybridDdSpec() :
            arraySize(-1), dataWords(-1) {
    }

    int datasize() const {
        return dataWords * sizeof(Word);
    }

    void construct(void* p) {
        new (p) S_State();
    }

    int get_root(void* p) {
        this->entity().construct(p);
        return this->entity().getRoot(s_state(p), a_state(p));
    }

    int get_child(void* p, int level, int value) {
        assert(0 <= value && value < S::ARITY);
        return this->entity().getChild(s_state(p), a_state(p), level, value);
    }

    void getCopy(void* p, S_State const& s) {
        new (p) S_State(s);
    }

    void get_copy(void* to, void const* from) {
        this->entity().getCopy(to, s_state(from));
        Word const* pa = static_cast<Word const*>(from);
        Word const* pz = pa + dataWords;
        Word* qa = static_cast<Word*>(to);
        pa += S_WORDS;
        qa += S_WORDS;
        while (pa != pz) {
            *qa++ = *pa++;
        }
    }

    int mergeStates(S_State& s1, A_State* a1, S_State& s2, A_State* a2) {
        return 0;
    }

    int merge_states(void* p1, void* p2) {
        return this->entity().mergeStates(s_state(p1), a_state(p1), s_state(p2),
                a_state(p2));
    }

    void destruct(void* p) {
    }

    void destructLevel(int level) {
    }

    size_t hashCode(S_State const& s) const {
        return this->rawHashCode(s);
    }

    size_t hashCodeAtLevel(S_State const& s, int level) const {
        return this->entity().hashCode(s);
    }

    size_t hash_code(void const* p, int level) const {
        size_t h = this->entity().hashCodeAtLevel(s_state(p), level);
        h *= 271828171;
        Word const* pa = static_cast<Word const*>(p);
        Word const* pz = pa + dataWords;
        pa += S_WORDS;
        while (pa != pz) {
            h += *pa++;
            h *= 314159257;
        }
        return h;
    }

    bool equalTo(S_State const& s1, S_State const& s2) const {
        return this->rawEqualTo(s1, s2);
    }

    bool equalToAtLevel(S_State const& s1, S_State const& s2, int level) const {
        return this->entity().equalTo(s1, s2);
    }

    bool equal_to(void const* p, void const* q, int level) const {
        if (!this->entity().equalToAtLevel(s_state(p), s_state(q), level))
            return false;
        Word const* pa = static_cast<Word const*>(p);
        Word const* qa = static_cast<Word const*>(q);
        Word const* pz = pa + dataWords;
        pa += S_WORDS;
        qa += S_WORDS;
        while (pa != pz) {
            if (*pa++ != *qa++) return false;
        }
        return true;
    }

    void printState(std::ostream& os, S_State const& s,
            A_State const* a) const {
        os << "[" << s << ":";
        for (int i = 0; i < arraySize; ++i) {
            if (i > 0) os << ",";
            os << a[i];
        }
        os << "]";
    }

    void printStateAtLevel(std::ostream& os, S_State const& s, A_State const* a,
            int level) const {
        this->entity().printState(os, s, a);
    }

    void print_state(std::ostream& os, void const* p, int level) const {
        this->entity().printStateAtLevel(os, s_state(p), a_state(p), level);
    }
};

/* for backward compatibility */
template<typename S, typename TS, typename TA, int AR>
class PodHybridDdSpec: public HybridDdSpec<S,TS,TA,AR> {
};

} // namespace tdzdd
