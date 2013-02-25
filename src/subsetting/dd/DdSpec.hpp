/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: DdSpec.hpp 415 2013-02-22 12:55:13Z iwashita $
 */

#pragma once

#include <cassert>
#include <functional>
#include <iostream>

#include "DdNodeId.hpp"

/*
 * Every implementation must have the following functions:
 * - int datasize() const
 * - int get_root(void* p)
 * - int get_child(void* p, int level, bool take)
 * - void get_copy(void* to, void const* from)
 * - void destruct(void* p)
 * - void destructLevel(int level)
 * - size_t hash_code(void const* p) const
 * - bool equal_to(void const* p, void const* q) const
 */

/**
 * Base class of DD specs.
 * @param S the class implementing this class.
 */
template<typename S>
struct DdSpec {
    S& entity() {
        return *static_cast<S*>(this);
    }

    S const& entity() const {
        return *static_cast<S const*>(this);
    }
};

/**
 * Abstract class of DD specifications without states.
 * Every implementation must have the following functions:
 * - int getRoot()
 * - int getChild(int level, bool take)
 * @param S the class implementing this class.
 */
template<typename S>
class StatelessDdSpec: public DdSpec<S> {
public:
    int datasize() const {
        return 0;
    }

    int get_root(void* p) {
        return this->entity().getRoot();
    }

    int get_child(void* p, int level, bool take) {
        return this->entity().getChild(level, take);
    }

    void get_copy(void* to, void const* from) {
    }

    void destruct(void* p) {
    }

    void destructLevel(int level) {
    }

    size_t hash_code(void const* p) const {
        return 0;
    }

    bool equal_to(void const* p, void const* q) const {
        return true;
    }

    std::ostream& print(std::ostream& os, void const* p) const {
        return os << "[]";
    }
};

/**
 * Abstract class of DD specifications using scalar states.
 * Every implementation must have the following functions:
 * - int getRoot(T& state)
 * - int getChild(T& state, int level, bool take)
 * Optionally, the following functions can be overloaded:
 * - void construct(void* p)
 * - void getCopy(void* p, T const& state)
 * - size_t hashCode(T const& state) const
 * - bool equalTo(T const& state1, T const& state2) const
 * @param S the class implementing this class.
 * @param T data type.
 */
template<typename S, typename T>
class ScalarDdSpec: public DdSpec<S> {
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

    int get_child(void* p, int level, bool take) {
        return this->entity().getChild(state(p), level, take);
    }

    void getCopy(void* p, T const& s) {
        new (p) State(s);
    }

    void get_copy(void* to, void const* from) {
        this->entity().getCopy(to, state(from));
    }

    void destruct(void* p) {
        state(p).~State();
    }

    void destructLevel(int level) {
    }

    size_t hashCode(State const& s) const {
        //return std::hash<State>()(s);
        return static_cast<size_t>(s);
    }

    size_t hash_code(void const* p) const {
        return this->entity().hashCode(state(p));
    }

    bool equalTo(State const& s1, State const& s2) const {
        return std::equal_to<State>()(s1, s2);
    }

    bool equal_to(void const* p, void const* q) const {
        return this->entity().equalTo(state(p), state(q));
    }

    std::ostream& print(std::ostream& os, State const& s) const {
        return os << "[" << s << "]";
    }

    std::ostream& print(std::ostream& os, void const* p) const {
        return print(os, state(p));
    }
};

/**
 * Abstract class of DD specifications using POD array states.
 * Every implementation must have the following functions:
 * - int getRoot(T* array)
 * - int getChild(T* array, int level, bool take)
 * @param S the class implementing this class.
 * @param T data type of array elements.
 */
template<typename S, typename T>
class PodArrayDdSpec: public DdSpec<S> {
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
        arraySize = n;
        dataWords = (n * sizeof(State) + sizeof(Word) - 1) / sizeof(Word);
    }

public:
    PodArrayDdSpec()
            : arraySize(-1), dataWords(-1) {
    }

    int datasize() const {
        return dataWords * sizeof(Word);
    }

    int get_root(void* p) {
        return this->entity().getRoot(state(p));
    }

    int get_child(void* p, int level, bool take) {
        return this->entity().getChild(state(p), level, take);
    }

    void get_copy(void* to, void const* from) {
        Word const* pa = static_cast<Word const*>(from);
        Word const* pz = pa + dataWords;
        Word* qa = static_cast<Word*>(to);
        while (pa != pz) {
            *qa++ = *pa++;
        }
    }

    void destruct(void* p) {
    }

    void destructLevel(int level) {
    }

    size_t hash_code(void const* p) const {
        Word const* pa = static_cast<Word const*>(p);
        Word const* pz = pa + dataWords;
        size_t h = 0;
        while (pa != pz) {
            h += *pa++;
            h *= 314159257;
        }
        return h;
    }

    bool equal_to(void const* p, void const* q) const {
        Word const* pa = static_cast<Word const*>(p);
        Word const* qa = static_cast<Word const*>(q);
        Word const* pz = pa + dataWords;
        while (pa != pz) {
            if (*pa++ != *qa++) return false;
        }
        return true;
    }

    std::ostream& print(std::ostream& os, State const* a) const {
        os << "[";
        for (int i = 0; i < arraySize; ++i) {
            if (i > 0) os << ",";
            os << a[i];
        }
        return os << "]";
    }

    std::ostream& print(std::ostream& os, void const* p) const {
        return print(os, state(p));
    }
};

/**
 * Abstract class of DD specifications using non-POD array states.
 * Every implementation must have the following functions:
 * - int getRoot(T* array)
 * - int getChild(T* array, int level, bool take)
 * Optionally, the following functions can be overloaded:
 * - void construct(void* p)
 * - void getCopy(void* p, T const& state)
 * - size_t hashCode(T const* p) const
 * - bool equalTo(T const* p, T const* q) const
 * @param S the class implementing this class.
 * @param T data type of array elements.
 */
template<typename S, typename T>
class ArrayDdSpec: public DdSpec<S> {
public:
    typedef T State;

private:
    int arraySize;

    static State* state(void* p) {
        return static_cast<State*>(p);
    }

    static State const* state(void const* p) {
        return static_cast<State const*>(p);
    }

protected:
    void setArraySize(int n) {
        assert(0 <= n);
        arraySize = n;
    }

public:
    ArrayDdSpec()
            : arraySize(-1) {
    }

    int datasize() const {
        return arraySize * sizeof(State);
    }

    void construct(void* p) {
        new (p) State();
    }

    int get_root(void* p) {
        for (int i = 0; i < arraySize; ++i) {
            this->entity().construct(state(p) + i);
        }

        return this->entity().getRoot(state(p));
    }

    int get_child(void* p, int level, bool take) {
        return this->entity().getChild(state(p), level, take);
    }

    void getCopy(void* p, T const& s) {
        new (p) State(s);
    }

    void get_copy(void* to, void const* from) {
        for (int i = 0; i < arraySize; ++i) {
            this->entity().getCopy(state(to) + i, state(from)[i]);
        }
    }

    void destruct(void* p) {
        for (int i = 0; i < arraySize; ++i) {
            state(p)[i].~State();
        }
    }

    void destructLevel(int level) {
    }

    size_t hashCode(State const& state) const {
        //return static_cast<size_t>(state);
        return state.hash();
    }

    size_t hash_code(void const* p) const {
        size_t h = 0;
        for (int i = 0; i < arraySize; ++i) {
            h += hashCode(state(p)[i]);
            h *= 314159257;
        }
        return h;
    }

    bool equalTo(State const& state1, State const& state2) const {
        return std::equal_to<State>()(state1, state2);
    }

    bool equal_to(void const* p, void const* q) const {
        for (int i = 0; i < arraySize; ++i) {
            if (!equalTo(state(p)[i], state(q)[i])) return false;
        }
        return true;
    }

    std::ostream& print(std::ostream& os, State const* a) const {
        os << "[";
        for (int i = 0; i < arraySize; ++i) {
            if (i > 0) os << ",";
            os << a[i];
        }
        return os << "]";
    }

    std::ostream& print(std::ostream& os, void const* p) const {
        return print(os, state(p));
    }
};

/**
 * Abstract class of DD specifications using both POD scalar and POD array states.
 * Every implementation must have the following functions:
 * - int getRoot(TS& scalar, TA* array)
 * - int getChild(TS& scalar, TA* array, int level, bool take)
 * @param S the class implementing this class.
 * @param TS data type of scalar.
 * @param TA data type of array elements.
 */
template<typename S, typename TS, typename TA>
class PodHybridDdSpec: public DdSpec<S> {
public:
    typedef TS S_State;
    typedef TA A_State;

private:
    typedef size_t Word;

    struct States {
        S_State s;
        A_State a[1];
    };

    int arraySize;
    int dataWords;

    static S_State& s_state(void* p) {
        return static_cast<States*>(p)->s;
    }

    static S_State const& s_state(void const* p) {
        return static_cast<States const*>(p)->s;
    }

    static A_State* a_state(void* p) {
        return static_cast<States*>(p)->a;
    }

    static A_State const* a_state(void const* p) {
        return static_cast<States const*>(p)->a;
    }

protected:
    void setArraySize(int n) {
        assert(0 <= n);
        arraySize = n;
        dataWords = (sizeof(States) + (n - 1) * sizeof(A_State) + sizeof(Word)
                - 1) / sizeof(Word);
    }

public:
    PodHybridDdSpec()
            : arraySize(-1), dataWords(-1) {
    }

    int datasize() const {
        return dataWords * sizeof(Word);
    }

    int get_root(void* p) {
        return this->entity().getRoot(s_state(p), a_state(p));
    }

    int get_child(void* p, int level, bool take) {
        return this->entity().getChild(s_state(p), a_state(p), level, take);
    }

    void get_copy(void* to, void const* from) {
        Word const* pa = static_cast<Word const*>(from);
        Word const* pz = pa + dataWords;
        Word* qa = static_cast<Word*>(to);
        while (pa != pz) {
            *qa++ = *pa++;
        }
    }

    void destruct(void* p) {
    }

    void destructLevel(int level) {
    }

    size_t hash_code(void const* p) const {
        Word const* pa = static_cast<Word const*>(p);
        Word const* pz = pa + dataWords;
        size_t h = 0;
        while (pa != pz) {
            h += *pa++;
            h *= 314159257;
        }
        return h;
    }

    bool equal_to(void const* p, void const* q) const {
        Word const* pa = static_cast<Word const*>(p);
        Word const* qa = static_cast<Word const*>(q);
        Word const* pz = pa + dataWords;
        while (pa != pz) {
            if (*pa++ != *qa++) return false;
        }
        return true;
    }

    std::ostream& print(std::ostream& os, S_State const& s,
            A_State const* a) const {
        os << "[";
        os << s << ":";
        for (int i = 0; i < arraySize; ++i) {
            if (i > 0) os << ",";
            os << a[i];
        }
        return os << "]";
    }

    std::ostream& print(std::ostream& os, void const* p) const {
        return print(os, s_state(p), a_state(p));
    }
};

/**
 * Abstract class of DD specifications using @p DdNodeId states.
 * Every implementation must have the following functions:
 * - DdNodeId getRoot()
 * - DdNodeId getChild(DdNodeId f, bool take)
 * @param S the class implementing this class.
 */
template<typename S>
class StructuralDdSpec: public DdSpec<S> {
    static DdNodeId& state(void* p) {
        return *static_cast<DdNodeId*>(p);
    }

    static DdNodeId const& state(void const* p) {
        return *static_cast<DdNodeId const*>(p);
    }

public:
    int datasize() const {
        return sizeof(DdNodeId);
    }

    int get_root(void* p) {
        DdNodeId f = state(p) = this->entity().getRoot();
        return (f == 1) ? -1 : f.row;
    }

    int get_child(void* p, int level, bool take) {
        assert(level > 0 && level == state(p).row);
        DdNodeId f = state(p) = this->entity().getChild(state(p), take);
        return (f == 1) ? -1 : f.row;
    }

    void get_copy(void* to, void const* from) {
        state(to) = state(from);
    }

    void destruct(void* p) {
    }

    void destructLevel(int level) {
    }

    size_t hash_code(void const* p) const {
        return state(p).hash();
    }

    bool equal_to(void const* p, void const* q) const {
        return state(p) == state(q);
    }

    std::ostream& print(std::ostream& os, void const* p) const {
        return os << "[" << state(p) << "]";
    }
};
