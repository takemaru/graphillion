/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: DdBuilder.hpp 412 2013-02-15 01:13:19Z iwashita $
 */

#pragma once

#include <cassert>
#include <ostream>
#include <stdexcept>
#include <vector>

#include "DdNode.hpp"
#include "DdNodeTable.hpp"
#include "../util/MemoryPool.hpp"
#include "../util/MyHashTable.hpp"
#include "../util/MyList.hpp"
#include "../util/MyVector.hpp"

class DdBuilderBase {
protected:
    static int const headerSize = 1;

    union Word {
        DdNodeId* source;
        //DdNodeId nodeId; We can't use unrestricted union before C++11.
        uint64_t code;
    };

    static DdNodeId*& source(Word* p) {
        return p->source;
    }

    static DdNodeId& nodeId(Word* p) {
        return *reinterpret_cast<DdNodeId*>(&p->code);
    }

    static void* state(Word* p) {
        return p + headerSize;
    }

    static void const* state(Word const* p) {
        return p + headerSize;
    }

//    union Word {
//        DdNodeId* source;
//        DdNodeId nodeId;
//
//        Word() {
//        }
//
//        void* state() {
//            return this + headerSize;
//        }
//
//        void const* state() const {
//            return this + headerSize;
//        }
//    };

    static int getVnodeWords(int n) {
        if (n < 0) throw std::runtime_error(
                "storage size is not initialized!!!");
        return headerSize + (n + sizeof(Word) - 1) / sizeof(Word);
    }

    template<typename SPEC>
    struct Hasher {
        SPEC const& spec;

        Hasher(SPEC const& spec)
                : spec(spec) {
        }

        size_t operator()(Word const* p) const {
            return spec.hash_code(state(p));
        }

        size_t operator()(Word const* p, Word const* q) const {
            return spec.equal_to(state(p), state(q));
        }
    };
};

/**
 * Basic top-down DD builder.
 */
template<typename S>
class DdBuilder: DdBuilderBase {
    typedef S Spec;
    typedef MyHashTable<Word*,Hasher<Spec>,Hasher<Spec> > UniqTable;

    DdNodeTable& resultTable;
    Spec& spec;
    Hasher<Spec> const hasher;
    int const vnodeWords;

    MyVector<MyList<Word> > vnodeTable;

public:
    DdBuilder(Spec& s, DdNodeTable& nodeTable)
            : resultTable(nodeTable), spec(s), hasher(spec),
              vnodeWords(getVnodeWords(spec.datasize())) {
    }

    /**
     * Initialization.
     * @param root the root node.
     */
    void initialize(DdNodeId& root) {
        std::vector<Word> tmp(vnodeWords);
        Word* ptmp = tmp.data();
        int n = spec.get_root(state(ptmp));

        if (n <= 0) {
            root = DdNodeId(-n);
            n = 0;
        }
        else {
            root = DdNodeId(n, 0);
            vnodeTable.resize(n + 1);
            Word* p0 = vnodeTable[n].alloc_front(vnodeWords);
            spec.get_copy(state(p0), state(ptmp));
            source(p0) = &root;
        }

        spec.destruct(state(ptmp));
        resultTable.init(n + 1);
    }

    /**
     * Build one level.
     * @param i level.
     */
    void construct(int i) {
        assert(0 < i && i < resultTable.numRows());
        assert(resultTable.numRows() - vnodeTable.size() == 0);

        MyList<Word>& vnodes = vnodeTable[i];
        size_t m = 0;

        {
            UniqTable uniq(vnodes.size() * 2, hasher, hasher);

            for (MyList<Word>::iterator t = vnodes.begin(); t != vnodes.end();
                    ++t) {
                Word* p = *t;
                Word* pp = uniq.add(p);

                if (pp == p) {
                    nodeId(p) = *source(p) = DdNodeId(i, m++);
                }
                else {
                    *source(p) = nodeId(pp);
                    nodeId(p) = 0;
                }
            }
        }

        DdNode* q = resultTable.initRow(i, m);
        Word* pp = vnodeTable[i - 1].alloc_front(vnodeWords);

        for (; !vnodes.empty(); vnodes.pop_front()) {
            Word* p = vnodes.front();
            if (nodeId(p) == 0) {
                spec.destruct(state(p));
                continue;
            }

            for (int b = 0; b <= 1; ++b) {
                spec.get_copy(state(pp), state(p));
                int ii = spec.get_child(state(pp), i, b);

                if (ii <= 0) {
                    spec.destruct(state(pp));
                    q->branch[b] = DdNodeId(-ii);
                }
                else if (ii == i - 1) {
                    source(pp) = &q->branch[b];
                    pp = vnodeTable[ii].alloc_front(vnodeWords);
                }
                else {
                    assert(ii < i - 1);
                    Word* ppp = vnodeTable[ii].alloc_front(vnodeWords);
                    spec.get_copy(state(ppp), state(pp));
                    spec.destruct(state(pp));
                    source(ppp) = &q->branch[b];
                }
            }

            spec.destruct(state(p));
            ++q;
        }

        assert(q == resultTable[i] + m);
        vnodeTable[i - 1].pop_front();
        spec.destructLevel(i);
    }
};

/**
 * Another top-down DD builder.
 * A node table for the <I>i</I>-th level becomes available instantly
 * after @p construct(i) is called, and is destructable at any time.
 */
template<typename S, bool dumpDot = false>
class InstantDdBuilder: DdBuilderBase {
    //typedef typename std::remove_const<typename std::remove_reference<S>::type>::type Spec;
    typedef S Spec;
    typedef MyHashTable<Word*,Hasher<Spec>,Hasher<Spec> > UniqTable;

    DdNodeTable& resultTable;
    Spec& spec;
    Hasher<Spec> const hasher;
    int const vnodeWords;

    MyVector<MyList<Word> > vnodeTable;
    MyVector<UniqTable> uniqTable;

public:
    InstantDdBuilder(Spec& s, DdNodeTable& resultTable)
            : resultTable(resultTable), spec(s), hasher(spec),
              vnodeWords(getVnodeWords(spec.datasize())) {
    }

    /**
     * Initialization.
     * @param root the root node.
     */
    void initialize(DdNodeId& root) {
        std::vector<Word> tmp(vnodeWords);
        Word* ptmp = tmp.data();
        int n = spec.get_root(state(ptmp));

        if (n <= 0) {
            root = DdNodeId(-n);
            n = 0;
        }
        else {
            root = DdNodeId(n, 0);
            vnodeTable.resize(n + 1);
            Word* p0 = vnodeTable[n].alloc_front(vnodeWords);
            spec.get_copy(state(p0), state(ptmp));

            uniqTable.reserve(n + 1);
            for (int i = 0; i <= n; ++i) {
                uniqTable.push_back(UniqTable(hasher, hasher));
            }
        }

        spec.destruct(state(ptmp));
        resultTable.init(n + 1);
    }

    /**
     * Build one level.
     * @param i level.
     */
    void construct(int i) {
        if (i <= 0) return;
        assert(i < resultTable.numRows());
        assert(resultTable.numRows() - vnodeTable.size() == 0);

        MyList<Word>& vnodes = vnodeTable[i];
        size_t m = vnodes.size();
        DdNode* q = resultTable.initRow(i, m) + m; // reverse of vnodeTable
        Word* pp = vnodeTable[i - 1].alloc_front(vnodeWords);

        for (; !vnodes.empty(); vnodes.pop_front()) {
            Word* p = vnodes.front();
            --q;

            if (dumpDot) {
                DdNodeId f(i, q - resultTable[i]);
                std::cout << "  \"" << f << "\" [label=\"" << i << " ";
                spec.print(std::cout, state(p));
                std::cout << "\"];\n";
            }

            for (int b = 0; b <= 1; ++b) {
                spec.get_copy(state(pp), state(p));
                int ii = spec.get_child(state(pp), i, b);

                if (ii <= 0) {
                    spec.destruct(state(pp));
                    q->branch[b] = DdNodeId(-ii);
                }
                else if (ii == i - 1) {
                    Word* pp1 = uniqTable[ii].add(pp);
                    if (pp1 == pp) {
                        size_t jj = vnodeTable[ii].size() - 1;
                        nodeId(pp1) = DdNodeId(ii, jj);
                        pp = vnodeTable[ii].alloc_front(vnodeWords);
                    }
                    else {
                        spec.destruct(state(pp));
                    }
                    q->branch[b] = nodeId(pp1);
                }
                else {
                    assert(ii < i - 1);
                    Word* pp2 = vnodeTable[ii].alloc_front(vnodeWords);
                    spec.get_copy(state(pp2), state(pp));
                    spec.destruct(state(pp));

                    Word* pp1 = uniqTable[ii].add(pp2);
                    if (pp1 == pp2) {
                        size_t j = vnodeTable[ii].size() - 1;
                        nodeId(pp1) = DdNodeId(ii, j);
                    }
                    else {
                        spec.destruct(state(pp2));
                        vnodeTable[ii].pop_front();
                    }
                    q->branch[b] = nodeId(pp1);
                }

                if (dumpDot) {
                    DdNodeId f(i, q - resultTable[i]);
                    DdNodeId g = q->branch[b];
                    if (g != 0) {
                        std::cout << "  \"" << f << "\" -> \"" << g
                                << "\" [style=" << (b ? "solid" : "dashed")
                                << "];\n";
                    }
                }
            }

            spec.destruct(state(p));
        }

        if (dumpDot) {
            if (m >= 2) {
                std::cout << "  {rank=same";
                for (size_t j = 0; j < m; ++j) {
                    std::cout << "; \"" << DdNodeId(i, j) << "\"";
                }
                std::cout << "}\n";
            }
        }

        assert(q == resultTable[i]);
        vnodeTable[i - 1].pop_front();
        uniqTable[i - 1].clear();
        spec.destructLevel(i);
    }
};

template<typename S>
void dumpDot(S& spec) {
    DdNodeTable nodeTable;
    InstantDdBuilder<S,true> idb(spec, nodeTable);
    DdNodeId root;
    idb.initialize(root);

    std::cout << "digraph {\n";

    for (int i = root.row; i >= 1; --i) {
        idb.construct(i);
        nodeTable.clear(i);
    }

    if (root == 0) {
        std::cout << "  \"" << DdNodeId(0)
                << "\" [shape=square,label=\"0\"];\n";
    }
    else {
        std::cout << "  \"" << DdNodeId(1)
                << "\" [shape=square,label=\"1\"];\n";
    }

    std::cout << "}\n";
    std::cout.flush();
}

/**
 * Top-down ZDD subset builder.
 */
template<typename S>
class ZddSubsetter: DdBuilderBase {
    //typedef typename std::remove_const<typename std::remove_reference<S>::type>::type Spec;
    typedef S Spec;
    typedef MyHashTable<Word*,Hasher<Spec>,Hasher<Spec> > UniqTable;

    DdNodeTable const& baseTable;
    DdNodeTable& resultTable;
    Spec& spec;
    Hasher<Spec> const hasher;
    int const vnodeWords;
    DdNodeProperty<MyListOnPool<Word> > work;

    std::vector<Word> tmp;
    std::vector<MemoryPool> pools;

public:
    ZddSubsetter(DdNodeTable const& baseTable, Spec& s,
            DdNodeTable& resultTable)
            : baseTable(baseTable), resultTable(resultTable), spec(s),
              hasher(spec), vnodeWords(getVnodeWords(spec.datasize())),
              work(baseTable) {
    }

    /**
     * Initialization.
     * @param root the root node.
     */
    void initialize(DdNodeId& root) {
        tmp.resize(vnodeWords);
        Word* ptmp = tmp.data();
        int n = spec.get_root(state(ptmp));

        int k = (root == 1) ? -1 : root.row;

        while (n != 0 && k != 0 && n != k) {
            if (n < k) {
                assert(k >= 1);
                k = downTable(root, 0, n);
            }
            else {
                assert(n >= 1);
                n = downSpec(state(ptmp), n, 0, k);
            }
        }

        if (n <= 0 || k <= 0) {
            assert(n == 0 || k == 0 || (n == -1 && k == -1));
            root = DdNodeId(0, n != 0 && k != 0);
            n = 0;
        }
        else {
            assert(n == k);

            root = DdNodeId(n, 0);
            pools.resize(n + 1);

            Word* p0 = work[n][0].alloc_front(pools[n], vnodeWords);
            spec.get_copy(state(p0), state(ptmp));
            source(p0) = &root;
        }

        spec.destruct(state(ptmp));
        resultTable.init(n + 1);
    }

    /**
     * Build one level.
     * @param i level.
     */
    void subset(int i) {
        assert(0 < i && i < resultTable.numRows());
        assert(resultTable.numRows() - pools.size() == 0);

        Word* ptmp = tmp.data();
        size_t const m = baseTable.rowSize(i);
        size_t mm = 0;

        {
            UniqTable uniq(hasher, hasher);

            for (size_t j = 0; j < m; ++j) {
                MyListOnPool<Word>& list = work[i][j];
                size_t n = list.size();

                if (n >= 2) {
                    uniq.initialize(n * 2);

                    for (MyListOnPool<Word>::iterator t = list.begin();
                            t != list.end(); ++t) {
                        Word* p = *t;
                        Word* pp = uniq.add(p);

                        if (pp == p) {
                            nodeId(p) = *source(p) = DdNodeId(i, mm++);
                        }
                        else {
                            *source(p) = nodeId(pp);
                            nodeId(p) = 0;
                        }
                    }
                }
                else if (n == 1) {
                    Word* p = list.front();
                    nodeId(p) = *source(p) = DdNodeId(i, mm++);
                }
            }
        }

        DdNode* q = resultTable.initRow(i, mm);

        for (size_t j = 0; j < m; ++j) {
            MyListOnPool<Word>& list = work[i][j];

            for (MyListOnPool<Word>::iterator t = list.begin(); t != list.end();
                    ++t) {
                Word* p = *t;
                if (nodeId(p) == 0) {
                    spec.destruct(state(p));
                    continue;
                }

                void* a[2] = {state(p), state(ptmp)};
                spec.get_copy(a[1], a[0]);

                for (int b = 0; b <= 1; ++b) {
                    DdNodeId f(i, j);
                    int kk = downTable(f, b, i - 1);
                    int ii = downSpec(a[b], i, b, kk);

                    while (ii != 0 && kk != 0 && ii != kk) {
                        if (ii < kk) {
                            assert(kk >= 1);
                            kk = downTable(f, 0, ii);
                        }
                        else {
                            assert(ii >= 1);
                            ii = downSpec(a[b], ii, 0, kk);
                        }
                    }

                    if (ii <= 0 || kk <= 0) {
                        bool val = ii != 0 && kk != 0;
                        q->branch[b] = val;
                    }
                    else {
                        assert(ii == f.row && ii == kk && ii < i);
                        Word* pp = work[ii][f.col].alloc_front(pools[ii],
                                vnodeWords);
                        spec.get_copy(state(pp), a[b]);
                        source(pp) = &q->branch[b];
                    }

                    spec.destruct(state(ptmp));
                }

                spec.destruct(state(p));
                ++q;
            }
        }

        assert(q == resultTable[i] + mm);
        work.clear(i);
        pools[i].clear();
        spec.destructLevel(i);
    }

private:
    int downTable(DdNodeId& f, bool b, int zerosupLevel) const {
        if (zerosupLevel < 0) zerosupLevel = 0;

        f = baseTable[f.row][f.col].branch[b];
        while (f.row > zerosupLevel) {
            f = baseTable[f.row][f.col].branch[0];
        }
        return (f == 1) ? -1 : f.row;
    }

    int downSpec(void* p, int level, bool b, int zerosupLevel) {
        if (zerosupLevel < 0) zerosupLevel = 0;
        assert(level > zerosupLevel);

        int i = spec.get_child(p, level, b);
        while (i > zerosupLevel) {
            i = spec.get_child(p, i, 0);
        }
        return i;
    }
};
