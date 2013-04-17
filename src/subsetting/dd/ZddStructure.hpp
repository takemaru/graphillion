/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: ZddStructure.hpp 427 2013-02-26 07:11:13Z iwashita $
 */

#pragma once

#include <cassert>
#include <ostream>
#include <vector>

#include "DdBuilder.hpp"
#include "DdEval.hpp"
#include "DdNode.hpp"
#include "DdNodeTable.hpp"
#include "DdSpec.hpp"
#include "../util/demangle.hpp"
#include "../util/MessageHandler.hpp"

class ZddStructure: public StructuralDdSpec<ZddStructure> {
    DdNodeTableHandler nodeTable; ///< The diagram structure.
    DdNodeId root;         ///< The root node ID.

public:
    /**
     * Default constructor.
     */
    ZddStructure()
            : root(0) {
    }

//    /*
//     * Imports the ZDD that can be broken.
//     * @param o the ZDD.
//     */
//    void moveFrom(ZddStructure& o) {
//        nodeTable.moveAssign(o.nodeTable);
//    }

    /**
     * Universal ZDD constructor.
     * @param n the number of variables.
     */
    ZddStructure(int n)
            : nodeTable(n + 1) {
        DdNodeTable& table = nodeTable.privateEntity();
        DdNodeId f(1);

        for (int i = 1; i <= n; ++i) {
            table.initRow(i, 1);
            table[i][0].branch[0] = f;
            table[i][0].branch[1] = f;
            f = DdNodeId(i, 0);
        }

        root = f;
    }

    /**
     * DD construction.
     * @param spec DD spec.
     * @param doReduce call reduce() if true.
     */
    template<typename SPEC>
    ZddStructure(DdSpec<SPEC>& spec, bool doReduce = true) {
        construct_(spec.entity());
        if (doReduce) reduce();
    }

//    /**
//     * DD construction.
//     * @param spec DD spec.
//     */
//    template<typename SPEC>
//    ZddStructure(DdSpec<SPEC> && spec, bool doReduce = true) {
//        construct_(std::move(spec.entity()));
//        if (doReduce) reduce();
//    }

    /**
     * ZDD subsetting.
     * @param spec ZDD spec.
     * @param doReduce call reduce() if true.
     */
    template<typename SPEC>
    void subset(DdSpec<SPEC>& spec, bool doReduce = true) {
        subset_(spec.entity());
        if (doReduce) reduce();
    }

//    /**
//     * ZDD subsetting.
//     * @param spec ZDD spec.
//     */
//    template<typename SPEC>
//    void subset(DdSpec<SPEC>&& spec, bool doReduce = true) {
//        subset_(std::move(spec.entity()));
//        if (doReduce) reduce();
//    }

private:
    template<typename SPEC>
    void construct_(SPEC& spec) {
        MessageHandler mh;
        mh.begin("construction") << " of " << typenameof(spec);
        DdBuilder<SPEC> zc(spec, nodeTable.privateEntity());
        zc.initialize(root);

        if (root.row > 0) {
            mh << "\n";
            for (int i = root.row; i > 0; --i) {
                mh << ".";
                zc.construct(i);
            }
        }
        else {
            mh << " ...";
        }

        mh.end(nodeTable->totalSize());
    }

    template<typename SPEC>
    void subset_(SPEC& spec) {
        MessageHandler mh;
        mh.begin("subsetting") << " by " << typenameof(spec);
        DdNodeTableHandler tmpTable;
#ifdef _OPENMPXXX
        ZddSubsetterMP<SPEC> zs(*nodeTable, spec, tmpTable.privateEntity());
#else
        ZddSubsetter<SPEC> zs(*nodeTable, spec, tmpTable.privateEntity());
#endif
        zs.initialize(root);

        if (root.row > 0) {
#ifdef _OPENMPXXX
            mh << " (#thread = " << zs.numThreads() << ")\n";
#else
            mh << "\n";
#endif
            for (int i = root.row; i > 0; --i) {
                mh << ".";
                zs.subset(i);
                nodeTable.derefLevel(i);
            }
        }
        else {
            mh << " ...";
        }

        nodeTable = tmpTable;
        mh.end(nodeTable->totalSize());
    }

public:
    /**
     * Reduce as a ZDD.
     */
    void reduce() {
        DdNodeTable& table = nodeTable.privateEntity();
        MessageHandler mh;
        mh.begin("reduction") << " ";
        int dots = 0;

        int const n = table.numRows() - 1;
        DdNodeTableHandler tmpTableHandler(n + 1);
        DdNodeTable& tmpTable = tmpTableHandler.privateEntity();

        /* apply zero-suppress rule */
        for (int i = 2; i <= n; ++i) {
            size_t const m = table.rowSize(i);
            DdNode* const tt = table[i];

#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
            for (size_t j = 0; j < m; ++j) {
                for (int c = 0; c <= 1; ++c) {
                    DdNodeId& fc = tt[j].branch[c];
                    if (fc.row == 0) continue;

                    DdNodeId fc1 = table[fc.row][fc.col].branch[1];
                    if (fc1 == 0) {
                        fc = table[fc.row][fc.col].branch[0];
                    }
                }
            }
        }
        table.makeIndex();

        DdNodeProperty<DdNodeId> newIdTable(table);
        newIdTable[0][0] = 0;
        newIdTable[0][1] = 1;

        table.initRow(0, 2);

        for (int i = 1; i <= n; ++i) {
            for (; n * dots < 10 * i; ++dots) {
                mh << ".";
            }

            size_t const m = table.rowSize(i);
            DdNode* const tt = table[i];

            DdNodeId* newId = newIdTable[i];

            for (size_t j = m - 1; j + 1 > 0; --j) {
                DdNodeId const mark(i, m);
                DdNodeId& f0 = tt[j].branch[0];
                DdNodeId& f1 = tt[j].branch[1];

                if (f0.row != 0) f0 = newIdTable[f0.row][f0.col];
                if (f1.row != 0) f1 = newIdTable[f1.row][f1.col];

                if (f1 == 0) {
                    newId[j] = f0;
                }
                else {
                    DdNodeId& f00 = table[f0.row][f0.col].branch[0];
                    DdNodeId& f01 = table[f0.row][f0.col].branch[1];

                    if (f01 != mark) {        // the first touch from this level
                        f01 = mark;             // mark f0 as touched
                        newId[j] = DdNodeId(n + 1, m); // tail of f0-equivalent list
                    }
                    else {
                        newId[j] = f00;         // next of f0-equivalent list
                    }
                    f00 = DdNodeId(n + 1, j);  // new head of f0-equivalent list
                }
            }

            {
                MyVector<int> const& levels = table.lowerLevels(i);
                for (int const* t = levels.begin(); t != levels.end(); ++t) {
                    newIdTable.clear(*t);
                }
            }
            size_t mm = 0;

            for (size_t j = 0; j < m; ++j) {
                DdNodeId const f(i, j);
                assert(newId[j].row <= i || newId[j].row == n + 1);
                if (newId[j].row <= n) continue;

                for (size_t k = j; k < m;) { // for each g in f0-equivalent list
                    assert(j <= k);
                    DdNodeId const g(i, k);
                    DdNodeId& g0 = tt[k].branch[0];
                    DdNodeId& g1 = tt[k].branch[1];
                    DdNodeId& g10 = table[g1.row][g1.col].branch[0];
                    DdNodeId& g11 = table[g1.row][g1.col].branch[1];
                    assert(newId[k].row >= n);
                    size_t next = newId[k].col;

                    if (g11 != f) { // the first touch to g1 in f0-equivalent list
                        g11 = f;        // mark g1 as touched
                        g10 = g;     // record g as a canonical node for <f0,g1>
                        newId[k] = DdNodeId(i, mm++);
                    }
                    else {
                        g0 = g10;       // make a forward link
                        g1 = 0;         // mark g as forwarded
                        newId[k] = 0;
                    }

                    k = next;
                }
            }

            {
                MyVector<int> const& levels = table.lowerLevels(i);
                for (int const* t = levels.begin(); t != levels.end(); ++t) {
                    nodeTable.derefLevel(*t);
                }
            }

            DdNode* nt = tmpTable.initRow(i, mm);

            for (size_t j = 0; j < m; ++j) {
                if (tt[j].branch[1] != 0) { // not forwarded
                    assert(newId[j].row == i);
                    size_t k = newId[j].col;
                    nt[k] = tt[j];
                }
                else {
                    assert(newId[j].row <= i);
                    DdNodeId f = tt[j].branch[0];
                    if (f.row == i) {
                        assert(newId[j] == 0);
                        newId[j] = newId[f.col];
                    }
                }
            }
        }

        nodeTable = tmpTableHandler;
        root = newIdTable[root.row][root.col];
        mh.end(nodeTable->totalSize());
    }

    /**
     * Reduce as a ZDD.
     */
    void reduceMP() {
        MessageHandler mh;
        mh.begin("reduction") << " ";
        int dots = 0;

        DdNodeTable& table = nodeTable.privateEntity();
        int const n = table.numRows() - 1;
        DdNodeTableHandler tmpTableHandler(n + 1);
        DdNodeTable& tmpTable = tmpTableHandler.privateEntity();

        ElapsedTimeCounter etc1;
        etc1.start();
        /* apply zero-suppress rule */
        for (int i = 2; i <= n; ++i) {
            size_t const m = table.rowSize(i);
            DdNode* const tt = table[i];

#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
            for (size_t j = 0; j < m; ++j) {
                for (int c = 0; c <= 1; ++c) {
                    DdNodeId& fc = tt[j].branch[c];
                    if (fc.row == 0) continue;

                    DdNodeId fc1 = table[fc.row][fc.col].branch[1];
                    if (fc1 == 0) {
                        fc = table[fc.row][fc.col].branch[0];
                    }
                }
            }
        }
        mh << "\nMP1: " << etc1.stop() << "\n";
        table.makeIndex();

        DdNodeProperty<DdNodeId> newIdTable(table);
        newIdTable[0][0] = 0;
        newIdTable[0][1] = 1;

        table.initRow(0, 2);

        ElapsedTimeCounter etc2;
        ElapsedTimeCounter etc3;
        ElapsedTimeCounter etc4;
        ElapsedTimeCounter etc5;

        for (int i = 1; i <= n; ++i) {
            for (; n * dots < 10 * i; ++dots) {
                mh << ".";
            }

            size_t const m = table.rowSize(i);
            DdNode* const tt = table[i];

            DdNodeId* newId = newIdTable[i];

//            etc2.start();
//#ifdef _OPENMP
//#pragma omp parallel for schedule(static)
//#endif
//            for (size_t j = 0; j < m; ++j) {
//                DdNodeId& f0 = tt[j].branch[0];
//                DdNodeId& f1 = tt[j].branch[1];
//
//                if (f0.row != 0) f0 = newIdTable[f0.row][f0.col];
//                if (f1.row != 0) f1 = newIdTable[f1.row][f1.col];
//
//                if (f1 == 0) {
//                    newId[j] = f0;
//                    assert(f0 < DdNodeId(i,j));
//                }
//            }
//            etc2.stop();
//
//            {
//                MyVector<int> const& levels = table.lowerLevels(i);
//                for (int const* t = levels.begin(); t != levels.end(); ++t) {
//                    newIdTable.clear(*t);
//                }
//            }
//
//            etc3.start();
//            for (size_t j = m - 1; j + 1 > 0; --j) {
//                DdNodeId f1 = tt[j].branch[1];
//                if (f1 == 0) continue;
//
//                DdNodeId f0 = tt[j].branch[0];
//                DdNodeId& f00 = table[f0.row][f0.col].branch[0];
//
//                if (f00.row != i) {         // the first touch from this level
//                    newId[j] = DdNodeId(i, m); // tail of f0-equivalent list
//                }
//                else {
//                    newId[j] = f00;         // next of f0-equivalent list
//                }
//                f00 = DdNodeId(i, j);       // new head of f0-equivalent list
//                assert(newId[j].row == i);
//                assert(newId[j] > f00);
//            }
//            etc3.stop();

            etc2.start();
//#ifdef _OPENMP
//#pragma omp parallel for ordered schedule(dynamic)
//#endif
            for (size_t j = m - 1; j < m; --j) {
                DdNodeId const mark(i, m);
                DdNodeId& f0 = tt[j].branch[0];
                DdNodeId& f1 = tt[j].branch[1];

                if (f0.row != 0) f0 = newIdTable[f0.row][f0.col];
                if (f1.row != 0) f1 = newIdTable[f1.row][f1.col];
                assert(f0.row < i);
                assert(f1.row < i);

                if (f1 == 0) {
                    newId[j] = f0;
                }
                else {
                    DdNodeId& f00 = table[f0.row][f0.col].branch[0];
                    DdNodeId& f01 = table[f0.row][f0.col].branch[1];

                    if (f01 != mark) {        // the first touch from this level
                        f01 = mark;             // mark f0 as touched
                        newId[j] = DdNodeId(n + 1, m); // tail of f0-equivalent list
                    }
                    else {
                        newId[j] = f00;         // next of f0-equivalent list
                    }
                    f00 = DdNodeId(n + 1, j);  // new head of f0-equivalent list
                }
            }
            etc2.stop();

            {
                MyVector<int> const& levels = table.lowerLevels(i);
                for (int const* t = levels.begin(); t != levels.end(); ++t) {
                    newIdTable.clear(*t);
                }
            }

            size_t mm = 0;

            etc4.start();
//#ifdef _OPENMP
//#pragma omp parallel for reduction(+:mm) schedule(static)
//#endif
            for (size_t j = 0; j < m; ++j) {
                DdNodeId const f(i, j);
                assert(newId[j].row <= i || newId[j].row == n + 1);
                if (newId[j].row <= n) continue;

                for (size_t k = j; k < m;) { // for each g in f0-equivalent list
                    assert(j <= k);
                    DdNodeId const g(i, k);
                    DdNodeId& g0 = tt[k].branch[0];
                    DdNodeId& g1 = tt[k].branch[1];
                    DdNodeId& g10 = table[g1.row][g1.col].branch[0];
                    DdNodeId& g11 = table[g1.row][g1.col].branch[1];
                    assert(newId[k].row >= n);
                    size_t next = newId[k].col;

                    if (g11 != f) { // the first touch to g1 in f0-equivalent list
                        g11 = f;        // mark g1 as touched
                        g10 = g;     // record g as a canonical node for <f0,g1>
                        newId[k] = DdNodeId(i, mm++);
                    }
                    else {
                        g0 = g10;       // make a forward link
                        g1 = 0;         // mark g as forwarded
                        newId[k] = 0;
                    }

                    k = next;
                }
            }
            etc4.stop();

            {
                MyVector<int> const& levels = table.lowerLevels(i);
                for (int const* t = levels.begin(); t != levels.end(); ++t) {
                    nodeTable.derefLevel(*t);
                }
            }

            tmpTable.initRow(i, mm);

            DdNode* nt = tmpTable[i];

            etc5.start();
//#ifdef _OPENMP
//#pragma omp parallel for schedule(static)
//#endif
            for (size_t j = 0; j < m; ++j) {
                if (tt[j].branch[1] != 0) { // not forwarded
                    assert(newId[j].row == i);
                    size_t k = newId[j].col;
                    nt[k] = tt[j];
                }
                else {
                    assert(newId[j].row <= i);
                    DdNodeId f = tt[j].branch[0];
                    if (f.row == i) {
                        assert(newId[j] == 0);
                        newId[j] = newId[f.col];
                    }
                }
            }
            etc5.stop();
        }
        mh << "\nMP2: " << etc2 << "\n";
        mh << "\nMP3: " << etc3 << "\n";
        mh << "\nMP4: " << etc4 << "\n";
        mh << "\nMP5: " << etc5 << "\n";

        nodeTable = tmpTableHandler;
        root = newIdTable[root.row][root.col];
        mh.end(nodeTable->totalSize());
    }

    DdNodeId getRoot() const {
        return root;
    }

    DdNodeId getChild(DdNodeId f, bool b) const {
        return (*nodeTable)[f.row][f.col].branch[b];
    }

    /**
     * Gets the number of ZDD variables.
     * @return the number of ZDD variables.
     */
    int numVars() const {
        return root.row;
    }

    /**
     * Gets the number of nonterminal nodes.
     * @return the number of nonterminal nodes.
     */
    size_t size() const {
        return nodeTable->totalSize();
    }

    /**
     * Evaluates the DD from the bottom to the top.
     * @param eval the driver class that implements <tt>evalTerminal(bool b, Val& v)</tt> and
     *   <tt>evalNode(int i, Val& v, int i0, Val const& v0, int i1, Val const& v1)</tt> methods.
     * @return value at the root.
     */
    template<typename T>
    typename T::RetVal evaluate(T eval) const {
        typedef typename T::Val Val;
        int n = root.row;
        eval.initialize(n);

        Val t0, t1;
        eval.evalTerminal(t0, 0);
        eval.evalTerminal(t1, 1);
        if (root == 0) return t0;
        if (root == 1) return t1;

        DdNodeProperty<Val> work(*nodeTable);
        work[0][0] = t0;
        work[0][1] = t1;

        for (int i = 1; i <= n; ++i) {
            size_t m = nodeTable->rowSize(i);
            for (size_t j = 0; j < m; ++j) {
                DdNodeId f0 = (*nodeTable)[i][j].branch[0];
                DdNodeId f1 = (*nodeTable)[i][j].branch[1];
                eval.evalNode(work[i][j], i, work[f0.row][f0.col], f0.row,
                        work[f1.row][f1.col], f1.row);
            }

            MyVector<int> const& levels = nodeTable->lowerLevels(i);
            for (int const* t = levels.begin(); t != levels.end(); ++t) {
                work.clear(*t);
                eval.destructLevel(*t);
            }
        }

        return eval.getValue(work[root.row][root.col]);
    }

    class const_iterator {
        struct Selection {
            DdNodeId node;
            bool val;

            Selection()
                    : val(false) {
            }

            Selection(DdNodeId node, bool val)
                    : node(node), val(val) {
            }

            bool operator==(Selection const& o) const {
                return node == o.node && val == o.val;
            }
        };

        ZddStructure const& dd;
        int cursor;
        std::vector<Selection> path;
        std::vector<int> itemSet;

    public:
        const_iterator(ZddStructure const& dd, bool begin)
                : dd(dd), cursor(begin ? -1 : -2), path(), itemSet() {
            if (begin) next(dd.root);
        }

        const_iterator& operator++() {
            next(DdNodeId(0, 0));
            return *this;
        }

        std::vector<int> const& operator*() const {
            return itemSet;
        }

        std::vector<int> const* operator->() const {
            return &itemSet;
        }

        bool operator==(const_iterator const& o) const {
            return cursor == o.cursor && path == o.path;
        }

        bool operator!=(const_iterator const& o) const {
            return !operator==(o);
        }

    private:
        void next(DdNodeId f) {
            for (;;) {
                while (f != 0) { /* down */
                    if (f == 1) return;
                    DdNode const& s = (*dd.nodeTable)[f.row][f.col];

                    if (s.branch[0] != 0) {
                        cursor = path.size();
                        path.push_back(Selection(f, false));
                        f = s.branch[0];
                    }
                    else {
                        itemSet.push_back(f.row);
                        path.push_back(Selection(f, true));
                        f = s.branch[1];
                    }
                }

                for (; cursor >= 0; --cursor) { /* up */
                    Selection& sel = path[cursor];
                    DdNode const& ss =
                            (*dd.nodeTable)[sel.node.row][sel.node.col];
                    if (sel.val == false && ss.branch[1] != 0) {
                        f = sel.node;
                        sel.val = true;
                        int i = f.row;
                        path.resize(cursor + 1);
                        while (!itemSet.empty() && itemSet.back() <= i) {
                            itemSet.pop_back();
                        }
                        itemSet.push_back(i);
                        f = (*dd.nodeTable)[f.row][f.col].branch[1];
                        break;
                    }
                }

                if (cursor < 0) { /* end() state */
                    cursor = -2;
                    path.clear();
                    itemSet.clear();
                    return;
                }
            }
        }
    };

    const_iterator begin() const {
        return const_iterator(*this, true);
    }

    const_iterator end() const {
        return const_iterator(*this, false);
    }

    /**
     * Dumps the node table in Sapporo ZDD format.
     * @param os the output stream.
     */
    void dumpSapporo(std::ostream& os) const {
        int const n = nodeTable->numRows() - 1;
        size_t const l = nodeTable->totalSize();

        os << "_i " << n << "\n";
        os << "_o " << 1 << "\n";
        os << "_n " << l << "\n";

        DdNodeProperty<size_t> nodeId(*nodeTable);
        size_t k = 0;

        for (int i = 1; i <= n; ++i) {
            size_t const m = nodeTable->rowSize(i);
            DdNode const* p = (*nodeTable)[i];

            for (size_t j = 0; j < m; ++j, ++p) {
                k += 2;
                nodeId[i][j] = k;
                os << k << " " << i;

                for (int c = 0; c <= 1; ++c) {
                    DdNodeId fc = p->branch[c];
                    if (fc == 0) {
                        os << " F";
                    }
                    else if (fc == 1) {
                        os << " T";
                    }
                    else {
                        os << " " << nodeId[fc.row][fc.col];
                    }
                }

                os << "\n";
            }

            MyVector<int> const& levels = nodeTable->lowerLevels(i);
            for (int const* t = levels.begin(); t != levels.end(); ++t) {
                nodeId.clear(*t);
            }
        }

        os << nodeId[root.row][root.col] << "\n";
        assert(k == l * 2);
    }

    /**
     * Dumps the node table in Graphviz (dot) format.
     * @param os the output stream.
     * @param labeler the function that maps an index to a node label.
     */
    template<typename L>
    void dumpDot(std::ostream& os, L labeler) const {
        os << "digraph {\n";

        for (int i = root.row; i >= 0; --i) {
            size_t m = nodeTable->rowSize(i);
            os << "  " << i << "[shape=none];\n";
            if (i < root.row) {
                os << "  " << (i + 1) << " -> " << i << "[style=invis];\n";
            }

            if (i >= 1) {
                for (size_t j = 0; j < m; ++j) {
                    DdNode const& s = (*nodeTable)[i][j];
                    DdNodeId const f(i, j);
                    DdNodeId const f0 = s.branch[0];
                    DdNodeId const f1 = s.branch[1];

                    os << "  \"" << f << "\" [label=\"" << labeler(f)
                            << "\"];\n";
                    if (f0 != 0) {
                        os << "  \"" << f << "\" -> \"" << f0
                                << "\" [style=dashed];\n";
                    }
                    if (f1 != 0) {
                        os << "  \"" << f << "\" -> \"" << f1
                                << "\" [style=solid];\n";
                    }
                }

                os << "  {rank=same; " << i;
                for (size_t j = 0; j < m; ++j) {
                    os << "; \"" << DdNodeId(i, j) << "\"";
                }
                os << "}\n";
            }
            else if (root == 0) {
                os << "  \"" << DdNodeId(0)
                        << "\" [shape=square,label=\"0\"];\n";
                os << "  {rank=same; 0; \"" << DdNodeId(0) << "\"}\n";
            }
            else {
                os << "  \"" << DdNodeId(1)
                        << "\" [shape=square,label=\"1\"];\n";
                os << "  {rank=same; 0; \"" << DdNodeId(1) << "\"}\n";
            }
        }

        os << "}\n";
        os.flush();
    }

    /**
     * Dumps the node table in Graphviz (dot) format.
     * @param os the output stream.
     */
    void dumpDot(std::ostream& os) const {
        dumpDot(os, DotLabeler());
    }

private:
    struct DotLabeler {
        DdNodeId operator()(DdNodeId f) const {
            return f;
        }
    };

public:
    /**
     * Dumps the node table in Graphviz (dot) format.
     * @param os the output stream.
     * @param o the ZDD.
     * @return os itself.
     */
    friend std::ostream& operator<<(std::ostream& os, ZddStructure const& o) {
        o.dumpDot(os);
        return os;
    }
};
