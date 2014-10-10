/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2014 Japan Science and Technology Agency
 * $Id: BinaryOperation.hpp 518 2014-03-18 05:29:23Z iwashita $
 */

#pragma once

#include <cassert>
#include <iostream>

#include "../dd/DdSpec.hpp"

template<typename S, typename S1, typename S2>
class BinaryOperation: public PodArrayDdSpec<S,size_t> {
protected:
    typedef S1 Spec1;
    typedef S2 Spec2;
    typedef size_t Word;

    static size_t const levelWords = (sizeof(int[2]) + sizeof(Word) - 1)
            / sizeof(Word);

    Spec1& spec1;
    Spec2& spec2;
    int const stateWords1;
    int const stateWords2;

    static int wordSize(int size) {
        return (size + sizeof(Word) - 1) / sizeof(Word);
    }

    void setLevel1(void* p, int level) const {
        static_cast<int*>(p)[0] = level;
    }

    int level1(void const* p) const {
        return static_cast<int const*>(p)[0];
    }

    void setLevel2(void* p, int level) const {
        static_cast<int*>(p)[1] = level;
    }

    int level2(void const* p) const {
        return static_cast<int const*>(p)[1];
    }

    void* state1(void* p) const {
        return static_cast<Word*>(p) + levelWords;
    }

    void const* state1(void const* p) const {
        return static_cast<Word const*>(p) + levelWords;
    }

    void* state2(void* p) const {
        return static_cast<Word*>(p) + levelWords + stateWords1;
    }

    void const* state2(void const* p) const {
        return static_cast<Word const*>(p) + levelWords + stateWords1;
    }

public:
    BinaryOperation(S1& s1, S2& s2)
            : spec1(s1), spec2(s2), stateWords1(wordSize(spec1.datasize())),
              stateWords2(wordSize(spec2.datasize())) {
        BinaryOperation::setArraySize(levelWords + stateWords1 + stateWords2);
    }

    void get_copy(void* to, void const* from) {
        setLevel1(to, level1(from));
        setLevel2(to, level2(from));
        spec1.get_copy(state1(to), state1(from));
        spec2.get_copy(state2(to), state2(from));
    }

    void destruct(void* p) {
        spec1.destruct(state1(p));
        spec2.destruct(state2(p));
    }

    void destructLevel(int level) {
        spec1.destructLevel(level);
        spec2.destructLevel(level);
    }

    size_t hash_code(void const* p) const {
        size_t h = size_t(level1(p)) * 314159257
                + size_t(level2(p)) * 271828171;
        if (level1(p) > 0) h += spec1.hash_code(state1(p)) * 171828143;
        if (level2(p) > 0) h += spec2.hash_code(state2(p)) * 141421333;
        return h;
    }

    bool equal_to(void const* p, void const* q) const {
        if (level1(p) != level1(q)) return false;
        if (level2(p) != level2(q)) return false;
        if (level1(p) > 0 && !spec1.equal_to(state1(p), state1(q))) return false;
        if (level2(p) > 0 && !spec2.equal_to(state2(p), state2(q))) return false;
        return true;
    }
};

template<typename S1, typename S2>
struct BddAnd: public BinaryOperation<BddAnd<S1,S2>,S1,S2> {
    typedef BinaryOperation<BddAnd,S1,S2> base;
    typedef typename base::Word Word;

    BddAnd(S1 const& s1, S2 const& s2)
            : base(s1, s2) {
    }

    int getRoot(Word* p) {
        int i1 = base::spec1.get_root(state1(p));
        if (i1 == 0) return 0;
        int i2 = base::spec2.get_root(state2(p));
        if (i2 == 0) return 0;
        setLevel1(p, i1);
        setLevel2(p, i2);
        return std::max(level1(p), level2(p));
    }

    int getChild(Word* p, int level, int take) {
        assert(level1(p) <= level && level2(p) <= level);
        if (level1(p) == level) {
            int i1 = base::spec1.get_child(state1(p), level, take);
            if (i1 == 0) return 0;
            setLevel1(p, i1);
        }
        if (level2(p) == level) {
            int i2 = base::spec2.get_child(state2(p), level, take);
            if (i2 == 0) return 0;
            setLevel2(p, i2);
        }
        return std::max(level1(p), level2(p));
    }

    void print_state(std::ostream& os, void const* p) const {
        Word const* q = static_cast<Word const*>(p);
        os << "<" << level1(q) << ",";
        base::spec1.print_state(os, state1(q));
        os << ">∧<" << level2(q) << ",";
        base::spec2.print_state(os, state2(q));
        os << ">";
    }
};

template<typename S1, typename S2>
struct BddOr: public BinaryOperation<BddOr<S1,S2>,S1,S2> {
    typedef BinaryOperation<BddOr,S1,S2> base;
    typedef typename base::Word Word;

    BddOr(S1 const& s1, S2 const& s2)
            : base(s1, s2) {
    }

    int getRoot(Word* p) {
        int i1 = base::spec1.get_root(state1(p));
        if (i1 < 0) return -1;
        int i2 = base::spec2.get_root(state2(p));
        if (i2 < 0) return -1;
        setLevel1(p, i1);
        setLevel2(p, i2);
        return std::max(level1(p), level2(p));
    }

    int getChild(Word* p, int level, int take) {
        assert(level1(p) <= level && level2(p) <= level);

        if (level1(p) == level) {
            int i1 = base::spec1.get_child(state1(p), level, take);
            if (i1 < 0) return -1;
            setLevel1(p, i1);
        }

        if (level2(p) == level) {
            int i2 = base::spec2.get_child(state2(p), level, take);
            if (i2 < 0) return -1;
            setLevel2(p, i2);
        }

        return std::max(level1(p), level2(p));
    }

    void print_state(std::ostream& os, void const* p) const {
        Word const* q = static_cast<Word const*>(p);
        os << "<" << level1(q) << ",";
        base::spec1.print_state(os, state1(q));
        os << ">∨<" << level2(q) << ",";
        base::spec2.print_state(os, state2(q));
        os << ">";
    }
};

template<typename S1, typename S2>
class ZddIntersection: public PodArrayDdSpec<ZddIntersection<S1,S2>,size_t> {
    typedef S1 Spec1;
    typedef S2 Spec2;
    typedef size_t Word;

    Spec1 spec1;
    Spec2 spec2;
    int const stateWords1;
    int const stateWords2;

    static int wordSize(int size) {
        return (size + sizeof(Word) - 1) / sizeof(Word);
    }

    void* state1(void* p) const {
        return p;
    }

    void const* state1(void const* p) const {
        return p;
    }

    void* state2(void* p) const {
        return static_cast<Word*>(p) + stateWords1;
    }

    void const* state2(void const* p) const {
        return static_cast<Word const*>(p) + stateWords1;
    }

public:
    ZddIntersection(S1 const& s1, S2 const& s2)
            : spec1(s1), spec2(s2), stateWords1(wordSize(spec1.datasize())),
              stateWords2(wordSize(spec2.datasize())) {
        ZddIntersection::setArraySize(stateWords1 + stateWords2);
    }

    int getRoot(Word* p) {
        int i1 = spec1.get_root(state1(p));
        if (i1 == 0) return 0;
        int i2 = spec2.get_root(state2(p));
        if (i2 == 0) return 0;

        while (i1 != i2) {
            if (i1 > i2) {
                i1 = spec1.get_child(state1(p), i1, 0);
                if (i1 == 0) return 0;
            }
            else {
                i2 = spec2.get_child(state2(p), i2, 0);
                if (i2 == 0) return 0;
            }
        }

        return i1;
    }

    int getChild(Word* p, int level, int take) {
        int i1 = spec1.get_child(state1(p), level, take);
        if (i1 == 0) return 0;
        int i2 = spec2.get_child(state2(p), level, take);
        if (i2 == 0) return 0;

        while (i1 != i2) {
            if (i1 > i2) {
                i1 = spec1.get_child(state1(p), i1, 0);
                if (i1 == 0) return 0;
            }
            else {
                i2 = spec2.get_child(state2(p), i2, 0);
                if (i2 == 0) return 0;
            }
        }

        return i1;
    }

    void get_copy(void* to, void const* from) {
        spec1.get_copy(state1(to), state1(from));
        spec2.get_copy(state2(to), state2(from));
    }

    void destruct(void* p) {
        spec1.destruct(state1(p));
        spec2.destruct(state2(p));
    }

    void destructLevel(int level) {
        spec1.destructLevel(level);
        spec2.destructLevel(level);
    }

    size_t hash_code(void const* p) const {
        return spec1.hash_code(state1(p)) * 314159257
                + spec2.hash_code(state2(p)) * 271828171;
    }

    bool equal_to(void const* p, void const* q) const {
        return spec1.equal_to(state1(p), state1(q))
                && spec2.equal_to(state2(p), state2(q));
    }

    void print_state(std::ostream& os, void const* p) const {
        Word const* q = static_cast<Word const*>(p);
        os << "<";
        spec1.print_state(os, state1(q));
        os << ">∩<";
        spec2.print_state(os, state2(q));
        os << ">";
    }
};

template<typename S1, typename S2>
struct ZddUnion: public BinaryOperation<ZddUnion<S1,S2>,S1,S2> {
    typedef BinaryOperation<ZddUnion,S1,S2> base;
    typedef typename base::Word Word;

    ZddUnion(S1 const& s1, S2 const& s2)
            : base(s1, s2) {
    }

    int getRoot(Word* p) {
        int i1 = base::spec1.get_root(state1(p));
        int i2 = base::spec2.get_root(state2(p));
        if (i1 == 0 && i2 == 0) return 0;
        if (i1 <= 0 && i2 <= 0) return -1;
        setLevel1(p, i1);
        setLevel2(p, i2);
        return std::max(level1(p), level2(p));
    }

    int getChild(Word* p, int level, int take) {
        assert(level1(p) <= level && level2(p) <= level);

        if (level1(p) == level) {
            int i1 = base::spec1.get_child(state1(p), level, take);
            if (i1 < 0) return -1;
            setLevel1(p, i1);
        }
        else if (take) {
            setLevel1(p, 0);
        }

        if (level2(p) == level) {
            int i2 = base::spec2.get_child(state2(p), level, take);
            if (i2 < 0) return -1;
            setLevel2(p, i2);
        }
        else if (take) {
            setLevel2(p, 0);
        }

        if (level1(p) == 0 && level2(p) == 0) return 0;
        if (level1(p) <= 0 && level2(p) <= 0) return -1;
        return std::max(level1(p), level2(p));
    }

    void print_state(std::ostream& os, void const* p) const {
        Word const* q = static_cast<Word const*>(p);
        os << "<" << level1(q) << ",";
        base::spec1.print_state(os, state1(q));
        os << ">∪<" << level2(q) << ",";
        base::spec2.print_state(os, state2(q));
        os << ">";
    }
};

/**
 * Returns a BDD specification for logical AND of two BDD specifications.
 * @param spec1 the first BDD specification.
 * @param spec2 the second BDD specification.
 * @return BDD specification for logical AND of @p spec1 and @p spec2.
 */
template<typename S1, typename S2>
BddAnd<S1,S2> bddAnd(S1 const& spec1, S2 const& spec2) {
    return BddAnd<S1,S2>(spec1, spec2);
}

/**
 * Returns a BDD specification for logical OR of two BDD specifications.
 * @param spec1 the first BDD specification.
 * @param spec2 the second BDD specification.
 * @return BDD specification for logical OR of @p spec1 and @p spec2.
 */
template<typename S1, typename S2>
BddOr<S1,S2> bddOr(S1 const& spec1, S2 const& spec2) {
    return BddOr<S1,S2>(spec1, spec2);
}

/**
 * Returns a ZDD specification for set intersection of two ZDD specifications.
 * @param spec1 the first ZDD specification.
 * @param spec2 the second ZDD specification.
 * @return ZDD specification for set intersection of @p spec1 and @p spec2.
 */
template<typename S1, typename S2>
ZddIntersection<S1,S2> zddIntersection(S1 const& spec1, S2 const& spec2) {
    return ZddIntersection<S1,S2>(spec1, spec2);
}

/**
 * Returns a ZDD specification for set union of two ZDD specifications.
 * @param spec1 the first ZDD specification.
 * @param spec2 the second ZDD specification.
 * @return ZDD specification for set union of @p spec1 and @p spec2.
 */
template<typename S1, typename S2>
ZddUnion<S1,S2> zddUnion(S1 const& spec1, S2 const& spec2) {
    return ZddUnion<S1,S2>(spec1, spec2);
}
