/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: PathCounter.hpp 412 2013-02-15 01:13:19Z iwashita $
 */

#pragma once

#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../util/BigNumber.hpp"
#include "../util/demangle.hpp"
#include "../util/MessageHandler.hpp"
#include "../util/MemoryPool.hpp"
#include "../util/MyHashTable.hpp"
#include "../util/MyList.hpp"
#include "../util/MyVector.hpp"
#include "DdSpec.hpp"

template<typename S>
class PathCounter {
    //typedef typename std::remove_const<typename std::remove_reference<S>::type>::type Spec;
    typedef S Spec;
    typedef uint64_t Word;

    struct Hasher {
        Spec const& spec;

        Hasher(Spec const& spec)
                : spec(spec) {
        }

        size_t operator()(Word const* p) const {
            return spec.hash_code(state(p));
        }

        size_t operator()(Word const* p, Word const* q) const {
            return spec.equal_to(state(p), state(q));
        }
    };

    typedef MyHashTable<Word*,Hasher,Hasher> UniqTable;

    Spec& spec;
    Hasher const hasher;
    int const stateWords;

    static int numWords(int n) {
        if (n < 0) throw std::runtime_error(
                "storage size is not initialized!!!");
        return (n + sizeof(Word) - 1) / sizeof(Word);
    }

    static void* state(Word* p) {
        return p;
    }

    static void const* state(Word const* p) {
        return p;
    }

    BigNumber number(Word* p) const {
        return BigNumber(p + stateWords);
    }

    uint64_t& number64(Word* p) const {
        return p[stateWords];
    }

public:
//    PathCounter(S&& s): spec(std::forward<S>(s)), hasher(spec), stateWords(numWords(spec.datasize())) {
//    }
    PathCounter(S& s)
            : spec(s), hasher(spec), stateWords(numWords(spec.datasize())) {
    }

    std::string count() {
        MessageHandler mh;
        mh.begin("counting") << " paths of " << typenameof(spec);

        std::vector<Word> tmp(stateWords + 1);
        Word* ptmp = tmp.data();
        int const n = spec.get_root(state(ptmp));
        if (n <= 0) {
            mh << " ...";
            mh.end(0);
            return (n == 0) ? "0" : "1";
        }
        mh << "\n";

        uint64_t totalStorage[n / 63 + 1];
        BigNumber total(totalStorage);
        total.store(0);
        size_t maxWidth = 0;
        //std::cerr << "\nLevel,Width\n";

        std::vector<MemoryPool> pools(n + 1);
        MyVector<MyList<Word> > vnodeTable(n + 1);
        MyVector<UniqTable> uniqTable;

        uniqTable.reserve(n + 1);
        for (int i = 0; i <= n; ++i) {
            uniqTable.push_back(UniqTable(hasher, hasher));
        }

        int numberWords = 1;
        Word* p0 = vnodeTable[n].alloc_front(stateWords + 1);
        spec.get_copy(state(p0), state(ptmp));
        spec.destruct(state(ptmp));
        number(p0).store(1);

        for (int i = n; i > 0; --i) {
            MyList<Word>& vnodes = vnodeTable[i];
            size_t m = vnodes.size();

            //std::cerr << i << "," << m << "\n";
            maxWidth = std::max(maxWidth, m);
            MyList<Word>& nextVnodes = vnodeTable[i - 1];
            UniqTable& nextUniq = uniqTable[i - 1];
            int const nextWords = stateWords + numberWords + 1;
            Word* pp = nextVnodes.alloc_front(nextWords);
            //if (nextUniq.size() < m) nextUniq.rehash(m);

            for (; !vnodes.empty(); vnodes.pop_front()) {
                Word* p = vnodes.front();
                if (number(p).equals(0)) {
                    spec.destruct(state(p));
                    continue;
                }

                for (int b = 0; b <= 1; ++b) {
                    spec.get_copy(state(pp), state(p));
                    int ii = spec.get_child(state(pp), i, b);

                    if (ii <= 0) {
                        spec.destruct(state(pp));
                        if (ii != 0) {
                            total.add(number(p));
                        }
                    }
                    else if (ii < i - 1) {
                        Word* qq = vnodeTable[ii].alloc_front(
                                nextWords + (i - ii) / 63);
                        spec.get_copy(state(qq), state(pp));
                        spec.destruct(state(pp));

                        Word* qqq = uniqTable[ii].add(qq);

                        if (qqq == qq) {
                            number(qqq).store(number(p));
                        }
                        else {
                            spec.destruct(state(qq));
                            int w = number(qqq).add(number(p));
                            if (numberWords < w) {
                                numberWords = w; //FIXME might be broken at long skip
                            }
                            vnodeTable[ii].pop_front();
                        }
                    }
                    else {
                        assert(ii == i - 1);
                        Word* ppp = nextUniq.add(pp);

                        if (ppp == pp) {
                            number(ppp).store(number(p));
                            pp = nextVnodes.alloc_front(nextWords);
                        }
                        else {
                            spec.destruct(state(pp));
                            int w = number(ppp).add(number(p));
                            if (numberWords < w) {
                                numberWords = w; //FIXME might be broken at long skip
                            }
                        }
                    }
                }

                spec.destruct(state(p));
            }

            nextVnodes.pop_front();
            nextUniq.clear();
            pools[i].clear();
            spec.destructLevel(i);
            mh << ".";
        }

        mh.end(maxWidth);
        return total;
    }

    std::string countFast() {
        MessageHandler mh;
        mh.begin("counting") << " paths of " << typenameof(spec);

        std::vector<Word> tmp(stateWords + 1);
        Word* ptmp = tmp.data();
        int const n = spec.get_root(state(ptmp));
        if (n <= 0) {
            mh << " ...";
            mh.end(0);
            return (n == 0) ? "0" : "1";
        }
        mh << "\n";

        uint64_t totalStorage[n / 63 + 1];
        BigNumber total(totalStorage);
        total.store(0);
        size_t maxWidth = 0;
        //std::cerr << "\nLevel,Width\n";

        std::vector<MemoryPool> pools(n + 1);
        MyVector<MyList<Word> > vnodeTable(n + 1);

        int numberWords = 1;
        Word* p0 = vnodeTable[n].alloc_front(stateWords + 1);
        spec.get_copy(state(p0), state(ptmp));
        spec.destruct(state(ptmp));
        number(p0).store(1);

        for (int i = n; i > 0; --i) {
            MyList<Word>& vnodes = vnodeTable[i];
            size_t m = 0;

            {
                UniqTable uniq(vnodes.size(), hasher, hasher);

                for (MyList<Word>::iterator t = vnodes.begin();
                        t != vnodes.end(); ++t) {
                    Word* p = *t;
                    Word* pp = uniq.add(p);

                    if (pp == p) {
                        ++m;
                    }
                    else {
                        int w = number(pp).add(number(p));
                        if (numberWords < w) {
                            numberWords = w; //FIXME might be broken at long skip
                        }
                        number(p).store(0);
                    }
                }
            }

            //std::cerr << i << "," << m << "\n";
            maxWidth = std::max(maxWidth, m);
            MyList<Word>& nextVnodes = vnodeTable[i - 1];
            int const nextWords = stateWords + numberWords + 1;
            Word* pp = nextVnodes.alloc_front(nextWords);

            for (; !vnodes.empty(); vnodes.pop_front()) {
                Word* p = vnodes.front();
                if (number(p).equals(0)) {
                    spec.destruct(state(p));
                    continue;
                }

                for (int b = 0; b <= 1; ++b) {
                    spec.get_copy(state(pp), state(p));
                    int ii = spec.get_child(state(pp), i, b);

                    if (ii <= 0) {
                        spec.destruct(state(pp));
                        if (ii != 0) {
                            total.add(number(p));
                        }
                    }
                    else if (ii < i - 1) {
                        Word* ppp = vnodeTable[ii].alloc_front(
                                nextWords + (i - ii) / 63);
                        spec.get_copy(state(ppp), state(pp));
                        spec.destruct(state(pp));
                        number(ppp).store(number(p));
                    }
                    else {
                        assert(ii == i - 1);
                        number(pp).store(number(p));
                        pp = nextVnodes.alloc_front(nextWords);
                    }
                }

                spec.destruct(state(p));
            }

            nextVnodes.pop_front();
            pools[i].clear();
            spec.destructLevel(i);
            mh << ".";
        }

        mh.end(maxWidth);
        return total;
    }

    uint64_t count64() {
        MessageHandler mh;
        mh.begin("counting") << " paths of " << typenameof(spec);

        std::vector<Word> tmp(stateWords + 1);
        Word* ptmp = tmp.data();
        int const n = spec.get_root(state(ptmp));
        if (n <= 0) {
            mh << " ...";
            mh.end(0);
            return (n == 0) ? 0 : 1;
        }
        mh << "\n";

        uint64_t total = 0;
        size_t maxWidth = 0;
        //std::cerr << "\nLevel,Width\n";

        std::vector<MemoryPool> pools(n + 1);
        MyVector<MyList<Word> > vnodeTable(n + 1);
        MyVector<UniqTable> uniqTable;

        uniqTable.reserve(n + 1);
        for (int i = 0; i <= n; ++i) {
            uniqTable.push_back(UniqTable(hasher, hasher));
        }

        Word* p0 = vnodeTable[n].alloc_front(stateWords + 1);
        spec.get_copy(state(p0), state(ptmp));
        spec.destruct(state(ptmp));
        number64(p0) = 1;

        for (int i = n; i > 0; --i) {
            MyList<Word>& vnodes = vnodeTable[i];
            size_t m = vnodes.size();

            //std::cerr << i << "," << m << "\n";
            maxWidth = std::max(maxWidth, m);
            MyList<Word>& nextVnodes = vnodeTable[i - 1];
            UniqTable& nextUniq = uniqTable[i - 1];
            Word* pp = nextVnodes.alloc_front(stateWords + 1);
            //if (nextUniq.size() < m) nextUniq.rehash(m);

            for (; !vnodes.empty(); vnodes.pop_front()) {
                Word* p = vnodes.front();
                if (number64(p) == 0) {
                    spec.destruct(state(p));
                    continue;
                }

                for (int b = 0; b <= 1; ++b) {
                    spec.get_copy(state(pp), state(p));
                    int ii = spec.get_child(state(pp), i, b);

                    if (ii <= 0) {
                        spec.destruct(state(pp));
                        if (ii != 0) {
                            total += number64(p);
                        }
                    }
                    else if (ii < i - 1) {
                        Word* qq = vnodeTable[ii].alloc_front(stateWords + 1);
                        spec.get_copy(state(qq), state(pp));
                        spec.destruct(state(pp));

                        Word* qqq = uniqTable[ii].add(qq);

                        if (qqq == qq) {
                            number64(qqq) = number64(p);
                        }
                        else {
                            spec.destruct(state(qq));
                            number64(qqq) += number64(p);
                            vnodeTable[ii].pop_front();
                        }
                    }
                    else {
                        assert(ii == i - 1);
                        Word* ppp = nextUniq.add(pp);

                        if (ppp == pp) {
                            number64(ppp) = number64(p);
                            pp = nextVnodes.alloc_front(stateWords + 1);
                        }
                        else {
                            spec.destruct(state(pp));
                            number64(ppp) += number64(p);
                        }
                    }
                }

                spec.destruct(state(p));
            }

            nextVnodes.pop_front();
            nextUniq.clear();
            pools[i].clear();
            spec.destructLevel(i);
            mh << ".";
        }

        mh.end(maxWidth);
        return total;
    }
};

template<typename S>
std::string countPaths(S& spec, bool fast = false) {
    PathCounter<S> pc(spec);
    return fast ? pc.countFast() : pc.count();
}

template<typename S>
uint64_t countPaths64(S& spec) {
    return PathCounter<S>(spec).count64();
}
