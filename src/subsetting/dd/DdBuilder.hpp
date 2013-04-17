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

#ifdef _OPENMP
#include <omp.h>
#endif

#include "DdNode.hpp"
#include "DdNodeTable.hpp"
#include "../util/MemoryPool.hpp"
#include "../util/MyHashTable.hpp"
#include "../util/MyList.hpp"
#include "../util/MyVector.hpp"

class DdBuilderBase {
protected:
    static int const headerSize = 1;

    union SpecNode {
        DdNodeId* source;
        uint64_t code;
    };

    static DdNodeId*& source(SpecNode* p) {
        return p[0].source;
    }

    static DdNodeId& nodeId(SpecNode* p) {
        return *reinterpret_cast<DdNodeId*>(&p[0].code);
    }

    static void* state(SpecNode* p) {
        return p + headerSize;
    }

    static void const* state(SpecNode const* p) {
        return p + headerSize;
    }

    static int getSpecNodeSize(int n) {
        if (n < 0) throw std::runtime_error(
                "storage size is not initialized!!!");
        return headerSize + (n + sizeof(SpecNode) - 1) / sizeof(SpecNode);
    }

    template<typename SPEC>
    struct Hasher {
        SPEC const& spec;

        Hasher(SPEC const& spec)
                : spec(spec) {
        }

        size_t operator()(SpecNode const* p) const {
            return spec.hash_code(state(p));
        }

        size_t operator()(SpecNode const* p, SpecNode const* q) const {
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
    typedef MyHashTable<SpecNode*,Hasher<Spec>,Hasher<Spec> > UniqTable;

    DdNodeTable& output;
    Spec& spec;
    Hasher<Spec> const hasher;
    int const specNodeSize;

    MyVector<MyList<SpecNode> > snodeTable;

public:
    DdBuilder(Spec& s, DdNodeTable& output)
            : output(output), spec(s), hasher(spec),
              specNodeSize(getSpecNodeSize(spec.datasize())) {
    }

    /**
     * Initialization.
     * @param root the root node.
     */
    void initialize(DdNodeId& root) {
        std::vector<SpecNode> tmp(specNodeSize);
        SpecNode* ptmp = tmp.data();
        int n = spec.get_root(state(ptmp));

        if (n <= 0) {
            root = DdNodeId(-n);
            n = 0;
        }
        else {
            root = DdNodeId(n, 0);
            snodeTable.resize(n + 1);
            SpecNode* p0 = snodeTable[n].alloc_front(specNodeSize);
            spec.get_copy(state(p0), state(ptmp));
            source(p0) = &root;
        }

        spec.destruct(state(ptmp));
        output.init(n + 1);
    }

    /**
     * Build one level.
     * @param i level.
     */
    void construct(int i) {
        assert(0 < i && i < output.numRows());
        assert(output.numRows() - snodeTable.size() == 0);

        MyList<SpecNode>& vnodes = snodeTable[i];
        size_t m = 0;

        {
            UniqTable uniq(vnodes.size() * 2, hasher, hasher);

            for (MyList<SpecNode>::iterator t = vnodes.begin();
                    t != vnodes.end(); ++t) {
                SpecNode* p = *t;
                SpecNode* pp = uniq.add(p);

                if (pp == p) {
                    nodeId(p) = *source(p) = DdNodeId(i, m++);
                }
                else {
                    *source(p) = nodeId(pp);
                    nodeId(p) = 0;
                }
            }
        }

        DdNode* q = output.initRow(i, m);
        SpecNode* pp = snodeTable[i - 1].alloc_front(specNodeSize);

        for (; !vnodes.empty(); vnodes.pop_front()) {
            SpecNode* p = vnodes.front();
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
                    pp = snodeTable[ii].alloc_front(specNodeSize);
                }
                else {
                    assert(ii < i - 1);
                    SpecNode* ppp = snodeTable[ii].alloc_front(specNodeSize);
                    spec.get_copy(state(ppp), state(pp));
                    spec.destruct(state(pp));
                    source(ppp) = &q->branch[b];
                }
            }

            spec.destruct(state(p));
            ++q;
        }

        assert(q == output[i] + m);
        snodeTable[i - 1].pop_front();
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
    typedef MyHashTable<SpecNode*,Hasher<Spec>,Hasher<Spec> > UniqTable;

    DdNodeTable& output;
    Spec& spec;
    Hasher<Spec> const hasher;
    int const specNodeSize;

    MyVector<MyList<SpecNode> > snodeTable;
    MyVector<UniqTable> uniqTable;

public:
    InstantDdBuilder(Spec& s, DdNodeTable& output)
            : output(output), spec(s), hasher(spec),
              specNodeSize(getSpecNodeSize(spec.datasize())) {
    }

    /**
     * Initialization.
     * @param root the root node.
     */
    void initialize(DdNodeId& root) {
        std::vector<SpecNode> tmp(specNodeSize);
        SpecNode* ptmp = tmp.data();
        int n = spec.get_root(state(ptmp));

        if (n <= 0) {
            root = DdNodeId(-n);
            n = 0;
        }
        else {
            root = DdNodeId(n, 0);
            snodeTable.resize(n + 1);
            SpecNode* p0 = snodeTable[n].alloc_front(specNodeSize);
            spec.get_copy(state(p0), state(ptmp));

            uniqTable.reserve(n + 1);
            for (int i = 0; i <= n; ++i) {
                uniqTable.push_back(UniqTable(hasher, hasher));
            }
        }

        spec.destruct(state(ptmp));
        output.init(n + 1);
    }

    /**
     * Build one level.
     * @param i level.
     */
    void construct(int i) {
        if (i <= 0) return;
        assert(i < output.numRows());
        assert(output.numRows() - snodeTable.size() == 0);

        MyList<SpecNode>& vnodes = snodeTable[i];
        size_t m = vnodes.size();
        DdNode* q = output.initRow(i, m) + m; // reverse of snodeTable
        SpecNode* pp = snodeTable[i - 1].alloc_front(specNodeSize);

        for (; !vnodes.empty(); vnodes.pop_front()) {
            SpecNode* p = vnodes.front();
            --q;

            if (dumpDot) {
                DdNodeId f(i, q - output[i]);
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
                    SpecNode* pp1 = uniqTable[ii].add(pp);
                    if (pp1 == pp) {
                        size_t jj = snodeTable[ii].size() - 1;
                        nodeId(pp1) = DdNodeId(ii, jj);
                        pp = snodeTable[ii].alloc_front(specNodeSize);
                    }
                    else {
                        spec.destruct(state(pp));
                    }
                    q->branch[b] = nodeId(pp1);
                }
                else {
                    assert(ii < i - 1);
                    SpecNode* pp2 = snodeTable[ii].alloc_front(specNodeSize);
                    spec.get_copy(state(pp2), state(pp));
                    spec.destruct(state(pp));

                    SpecNode* pp1 = uniqTable[ii].add(pp2);
                    if (pp1 == pp2) {
                        size_t j = snodeTable[ii].size() - 1;
                        nodeId(pp1) = DdNodeId(ii, j);
                    }
                    else {
                        spec.destruct(state(pp2));
                        snodeTable[ii].pop_front();
                    }
                    q->branch[b] = nodeId(pp1);
                }

                if (dumpDot) {
                    DdNodeId f(i, q - output[i]);
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

        assert(q == output[i]);
        snodeTable[i - 1].pop_front();
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
    typedef MyHashTable<SpecNode*,Hasher<Spec>,Hasher<Spec> > UniqTable;

    DdNodeTable const& input;
    DdNodeTable& output;
    Spec& spec;
    Hasher<Spec> const hasher;
    int const specNodeSize;
    DdNodeProperty<MyListOnPool<SpecNode> > work;

    std::vector<SpecNode> tmp;
    std::vector<MemoryPool> pools;

public:
    ZddSubsetter(DdNodeTable const& input, Spec& s, DdNodeTable& output)
            : input(input), output(output), spec(s), hasher(spec),
              specNodeSize(getSpecNodeSize(spec.datasize())), work(input) {
    }

    /**
     * Initialization.
     * @param root the root node.
     */
    void initialize(DdNodeId& root) {
        tmp.resize(specNodeSize);
        SpecNode* ptmp = tmp.data();
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
            assert(n == root.row);

            pools.resize(n + 1);

            SpecNode* p0 = work[n][root.col].alloc_front(pools[n],
                    specNodeSize);
            spec.get_copy(state(p0), state(ptmp));
            source(p0) = &root;
        }

        spec.destruct(state(ptmp));
        output.init(n + 1);
    }

    /**
     * Build one level.
     * @param i level.
     */
    void subset(int i) {
        assert(0 < i && i < output.numRows());
        assert(output.numRows() - pools.size() == 0);

        SpecNode* const ptmp = tmp.data();
        size_t const m = input.rowSize(i);
        size_t mm = 0;

        for (size_t j = 0; j < m; ++j) {
            MyListOnPool<SpecNode>& list = work[i][j];
            size_t n = list.size();

            if (n >= 2) {
                UniqTable uniq(n * 2, hasher, hasher);

                for (MyListOnPool<SpecNode>::iterator t = list.begin();
                        t != list.end(); ++t) {
                    SpecNode* p = *t;
                    SpecNode* pp = uniq.add(p);

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
                SpecNode* p = list.front();
                nodeId(p) = *source(p) = DdNodeId(i, mm++);
            }
        }

        DdNode* q = output.initRow(i, mm);

        for (size_t j = 0; j < m; ++j) {
            MyListOnPool<SpecNode>& list = work[i][j];

            for (MyListOnPool<SpecNode>::iterator t = list.begin();
                    t != list.end(); ++t) {
                SpecNode* p = *t;
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
                        SpecNode* pp = work[ii][f.col].alloc_front(pools[ii],
                                specNodeSize);
                        spec.get_copy(state(pp), a[b]);
                        source(pp) = &q->branch[b];
                    }

                    spec.destruct(state(ptmp));
                }

                spec.destruct(state(p));
                ++q;
            }
        }

        assert(q == output[i] + mm);
        work.clear(i);
        pools[i].clear();
        spec.destructLevel(i);
    }

private:
    int downTable(DdNodeId& f, bool b, int zerosupLevel) const {
        if (zerosupLevel < 0) zerosupLevel = 0;

        f = input[f.row][f.col].branch[b];
        while (f.row > zerosupLevel) {
            f = input[f.row][f.col].branch[0];
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

class DdBuilderMPBase {
protected:
    static int const headerSize = 2;

    union SpecNode {
        DdNodeId* ddSrcPtr;
        uint64_t code;
    };

    struct Work {
        uint16_t parentRow;
        uint16_t nextRow[2];
        size_t parentCol;
        size_t nextCol[2];
        size_t numSibling;
        size_t outColBase;
        MyListOnPool<SpecNode> snodes[2];

        Work()
                : parentRow(), nextRow(), parentCol(), nextCol(), numSibling(0),
                  outColBase() {
        }
    };

    static DdNodeId*& ddSrcPtr(SpecNode* p) {
        return p[0].ddSrcPtr;
    }

    static DdNodeId& ddNodeId(SpecNode* p) {
        return *reinterpret_cast<DdNodeId*>(&p[1].code);
    }

    static DdNodeId ddNodeId(SpecNode const* p) {
        return *reinterpret_cast<DdNodeId const*>(&p[1].code);
    }

    static int row(SpecNode const* p) {
        return ddNodeId(p).row;
    }

    static size_t col(SpecNode const* p) {
        return ddNodeId(p).col;
    }

    static void* state(SpecNode* p) {
        return p + headerSize;
    }

    static void const* state(SpecNode const* p) {
        return p + headerSize;
    }

    static int getSpecNodeSize(int n) {
        if (n < 0) throw std::runtime_error(
                "storage size is not initialized!!!");
        return headerSize + (n + sizeof(SpecNode) - 1) / sizeof(SpecNode);
    }

    template<typename SPEC>
    struct Hasher {
        SPEC const& spec;

        Hasher(SPEC const& spec)
                : spec(spec) {
        }

        size_t operator()(SpecNode const* p) const {
            return spec.hash_code(state(p));
        }

        size_t operator()(SpecNode const* p, SpecNode const* q) const {
            return spec.equal_to(state(p), state(q));
        }
    };
};

/**
 * Multi-threaded top-down ZDD subset builder.
 */
template<typename S>
class ZddSubsetterMP: DdBuilderMPBase {
    //typedef typename std::remove_const<typename std::remove_reference<S>::type>::type Spec;
    typedef S Spec;
    typedef MyHashTable<SpecNode*,Hasher<Spec>,Hasher<Spec> > UniqTable;

    DdNodeTable const& input;
    DdNodeTable& output;
    Spec& spec;
    Hasher<Spec> const hasher;
    int const specNodeSize;
    DdNodeProperty<Work> work;

    int const maxThreads;
    std::vector<std::vector<MemoryPool> > pools;

    std::vector<SpecNode> tmpVector;
    SpecNode* tmp;

    ElapsedTimeCounter etc1;
    ElapsedTimeCounter etc2;
    ElapsedTimeCounter etc3;
    ElapsedTimeCounter etc4;
    ElapsedTimeCounter etc5;

public:
    ZddSubsetterMP(DdNodeTable const& input, Spec& s, DdNodeTable& output)
            : input(input), output(output), spec(s), hasher(spec),
              specNodeSize(getSpecNodeSize(spec.datasize())), work(input),
#ifdef _OPENMP
              maxThreads(omp_get_max_threads()),
#else
              maxThreads(1),
#endif
              pools(maxThreads),
              tmpVector(specNodeSize), tmp(tmpVector.data()) {
    }

    ~ZddSubsetterMP() {
        MessageHandler mh;
        mh << "MP1: " << etc1 << "\n";
        mh << "MP2: " << etc2 << "\n";
        mh << "MP3: " << etc3 << "\n";
        mh << "MP4: " << etc4 << "\n";
        mh << "MP5: " << etc5 << "\n";
    }

    int numThreads() const {
        return maxThreads;
    }

    /**
     * Initialization.
     * @param root the root node.
     */
    void initialize(DdNodeId& root) {
        ddNodeId(tmp).row = spec.get_root(state(tmp));

        int n = (root == 1) ? -1 : root.row;

        while (row(tmp) != 0 && n != 0 && row(tmp) != n) {
            if (row(tmp) < n) {
                assert(n >= 1);
                n = downTable(root, 0, row(tmp));
            }
            else {
                assert(row(tmp) >= 1);
                downSpec(tmp, row(tmp), 0, n);
            }
        }

        if (row(tmp) <= 0 || n <= 0) {
            assert(row(tmp) == 0 || n == 0 || (row(tmp) == -1 && n == -1));
            root = DdNodeId(0, row(tmp) != 0 && n != 0);
            ddNodeId(tmp) = 0;
            output.init(1);
        }
        else {
            assert(n == root.row);
            assert(row(tmp) == n);

            for (int t = 0; t < maxThreads; ++t) {
                pools[t].resize(n + 1);
            }

            output.init(n + 1);
            output.initRow(n, 1);

            for (int b = 0; b <= 1; ++b) {
                int ii = input[root.row][root.col].branch[b].row;
                MyListOnPool<SpecNode>& snodes =
                        work[root.row][root.col].snodes[b];
                SpecNode* p = snodes.alloc_front(pools[0][ii], specNodeSize);
                ddSrcPtr(p) = &output[n][0].branch[b];
                ddNodeId(p) = root;
                spec.get_copy(state(p), state(tmp));
            }

            root = DdNodeId(n, 0);
        }

        spec.destruct(state(tmp));
    }

    /**
     * Build one level.
     * @param i level.
     */
    void subset(int const i) {
        assert(0 < i && i < output.numRows());
        assert(output.numRows() - pools[0].size() == 0);

        etc1.start();
        size_t const m = input.rowSize(i);

#ifdef _OPENMP
        { // allocate lower lows
            MyVector<int> const& levels = input.lowerLevels(i);

            for (int const* it = levels.begin(); it != levels.end(); ++it) {
                work[*it];
            }
        }
#endif

        etc1.stop();
        etc2.start();

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
        for (size_t j = 0; j < m; ++j) {
            Work& w = work[i][j];

            for (int b = 0; b <= 1; ++b) {
                DdNodeId f = input[i][j].branch[b];
                MyListOnPool<SpecNode>& snodes = w.snodes[b];

                if (f == 0) {
                    for (MyListOnPool<SpecNode>::iterator it = snodes.begin();
                            it != snodes.end(); ++it) {
                        SpecNode* p = *it;
                        *ddSrcPtr(p) = 0;
                    }
                }
                else if (f == 1) {
                    for (MyListOnPool<SpecNode>::iterator it = snodes.begin();
                            it != snodes.end(); ++it) {
                        SpecNode* p = *it;
                        downSpec(p, i, 1, 0);
                        assert(row(p) == 0 || row(p) == 65535);
                        *ddSrcPtr(p) = (row(p) == 0) ? 0 : 1;
                    }
                }
            }
        }

        etc2.stop();
        etc3.start();

#ifdef _OPENMP
#pragma omp parallel
#endif
        {
#ifdef _OPENMP
            size_t nt = omp_get_num_threads();
            size_t tn = omp_get_thread_num();
#endif
            for (size_t j = 0; j < m; ++j) {
                Work& w = work[i][j];

                for (int b = 0; b <= 1; ++b) {
                    DdNodeId f = input[i][j].branch[b];
                    if (f.row == 0) continue;
#ifdef _OPENMP
                    if (f.hash() % nt != tn) continue;
#endif
                    Work& ww = work[f.row][f.col];
                    w.nextRow[b] = ww.parentRow;
                    w.nextCol[b] = ww.parentCol;
                    ww.parentRow = i;
                    ww.parentCol = (j << 1) | b;
                    ww.numSibling += w.snodes[b].size();
                }
            }
        }

        int const ii = i - 1;
        size_t const mm = input.rowSize(ii);

        etc3.stop();
        etc4.start();

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
        for (size_t jj = 0; jj < mm; ++jj) {
            UniqTable uniq(hasher, hasher);
            Work& ww = work[ii][jj];
            int i = ww.parentRow;
            int j = ww.parentCol >> 1;
            int b = ww.parentCol & 1;
            size_t k = 0;

            if (ww.numSibling >= 2) uniq.initialize(ww.numSibling * 2);

            while (i > 0) {
                Work& w = work[i][j];

                for (MyListOnPool<SpecNode>::iterator it = w.snodes[b].begin();
                        it != w.snodes[b].end(); ++it) {
                    SpecNode* p = *it;

                    downSpec(p, i, b, ii);

                    if (row(p) == ii) {
                        if (ww.numSibling >= 2) {
                            SpecNode* q = uniq.add(p);
                            if (q != p) { // state convergence
                                ddNodeId(p) = ddNodeId(q);
                                continue;
                            }
                        }

                        ddNodeId(p) = DdNodeId(ii, k++);
                    }
                }

                i = w.nextRow[b];
                j = w.nextCol[b] >> 1;
                b = w.nextCol[b] & 1;
            }

            ww.numSibling = k;
        }

        etc4.stop();
        etc1.start();

        size_t nn = 0;
        for (size_t jj = 0; jj < mm; ++jj) {
            Work& ww = work[ii][jj];
            ww.outColBase = nn;
            nn += ww.numSibling;
        }
        output.initRow(ii, nn);

        etc1.stop();
        etc5.start();

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
        for (size_t jj = 0; jj < mm; ++jj) {
#ifdef _OPENMP
            int const t = omp_get_thread_num();
#else
            int const t = 0;
#endif
            DdNode const& inNode = input[ii][jj];
            Work& ww = work[ii][jj];
            int i = ww.parentRow;
            int j = ww.parentCol >> 1;
            int b = ww.parentCol & 1;
            size_t k = 0;

            while (i > 0) {
                Work& w = work[i][j];

                for (MyListOnPool<SpecNode>::iterator it = w.snodes[b].begin();
                        it != w.snodes[b].end(); ++it) {
                    SpecNode* p = *it;

                    if (row(p) == ii) {
                        DdNodeId dst(ii, ww.outColBase + col(p));

                        *ddSrcPtr(p) = dst;
                        if (col(p) < k) continue;
                        ++k;

                        for (int bb = 0; bb <= 1; ++bb) {
                            int iii = inNode.branch[bb].row;
                            if (iii == 0) iii = ii;

                            SpecNode* pp = ww.snodes[bb].alloc_front(
                                    pools[t][iii], specNodeSize);
                            ddSrcPtr(pp) = &output[dst.row][dst.col].branch[bb];
                            ddNodeId(pp) = dst;
                            spec.get_copy(state(pp), state(p));
                        }
                    }
                    else {
                        assert(row(p) < ii);
                        int iii = inNode.branch[0].row;
                        if (iii == 0) iii = ii;

                        SpecNode* pp = ww.snodes[0].alloc_front(pools[t][iii],
                                specNodeSize);
                        ddSrcPtr(pp) = ddSrcPtr(p);
                        ddNodeId(pp) = ddNodeId(p);
                        spec.get_copy(state(pp), state(p));
                    }
                }

                i = w.nextRow[b];
                j = w.nextCol[b] >> 1;
                b = w.nextCol[b] & 1;
            }
        }
        etc5.stop();
        etc1.start();

        for (int t = 0; t < maxThreads; ++t) {
            pools[t][i].clear();
        }

        {
            MyVector<int> const& levels = input.higherLevels(i);

            for (int const* it = levels.begin(); it != levels.end(); ++it) {
                work.clear(*it);
                spec.destructLevel(*it);
            }
        }
        etc1.stop();
    }

private:
    int downTable(DdNodeId& f, bool b, int zerosupLevel) const {
        if (zerosupLevel < 0) zerosupLevel = 0;

        f = input[f.row][f.col].branch[b];
        while (f.row > zerosupLevel) {
            f = input[f.row][f.col].branch[0];
        }
        return (f == 1) ? -1 : f.row;
    }

    void downSpec(SpecNode* p, int i, bool b, int zerosupLevel) {
        if (zerosupLevel < 0) zerosupLevel = 0;
        int level = row(p);

        if (i > zerosupLevel) {
            while (level > i) {
                level = spec.get_child(state(p), level, 0);
            }

            if (level == i) {
                level = spec.get_child(state(p), level, b);
            }
        }

        while (level > zerosupLevel) {
            level = spec.get_child(state(p), level, 0);
        }

        ddNodeId(p).row = level; // can be 65535
    }
};
