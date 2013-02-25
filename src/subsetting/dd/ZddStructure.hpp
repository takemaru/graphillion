/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: ZddStructure.hpp 414 2013-02-15 04:04:57Z iwashita $
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
    ZddStructure() {
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
        DdNodeId f(1);

        for (int i = 1; i <= n; ++i) {
            nodeTable->initRow(i, 1);
            (*nodeTable)[i][0].branch[0] = f;
            (*nodeTable)[i][0].branch[1] = f;
            f = DdNodeId(i, 0);
        }

        root = f;
    }

    /**
     * DD construction.
     * @param spec DD spec.
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
        DdBuilder<SPEC> zc(spec, *nodeTable);
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
        ZddSubsetter<SPEC> zs(*nodeTable, spec, *tmpTable);
        zs.initialize(root);

        if (root.row > 0) {
            mh << "\n";
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
        MessageHandler mh;
        mh.begin("reduction") << " ";
        int dots = 0;

        int const n = nodeTable->numRows() - 1;
        DdNodeTableHandler tmpTable(n + 1);

        /* for remaking levelJumpTable */
        for (int i = 2; i <= n; ++i) {
            size_t const m = nodeTable->rowSize(i);
            DdNode* const tt = (*nodeTable)[i];

            for (size_t j = 0; j < m; ++j) {
                for (int c = 0; c <= 1; ++c) {
                    DdNodeId& fc = tt[j].branch[c];
                    if (fc.row == 0) continue;

                    DdNodeId fc1 = (*nodeTable)[fc.row][fc.col].branch[1];
                    if (fc1 == 0) {
                        fc = (*nodeTable)[fc.row][fc.col].branch[0];
                    }
                }
            }
        }
        nodeTable->makeIndex();

        DdNodeProperty<DdNodeId> newIdTable(*nodeTable);
        newIdTable[0][0] = 0;
        newIdTable[0][1] = 1;

        nodeTable->initRow(0, 2);

        for (int i = 1; i <= n; ++i) {
            for (; n * dots < 10 * i; ++dots) {
                mh << ".";
            }

            size_t const m = nodeTable->rowSize(i);
            DdNode* const tt = (*nodeTable)[i];

            DdNodeId* newId = newIdTable[i];

            for (size_t j = m - 1; j + 1 > 0; --j) {
                DdNodeId const f(i, j);
                DdNodeId& f0 = tt[j].branch[0];
                DdNodeId& f1 = tt[j].branch[1];

                if (f0.row != 0) f0 = newIdTable[f0.row][f0.col];
                if (f1.row != 0) f1 = newIdTable[f1.row][f1.col];

                if (f1 == 0) {
                    newId[j] = f0;
                    assert(newId[j] < f);
                    continue;
                }

                DdNodeId& f00 = (*nodeTable)[f0.row][f0.col].branch[0];
                if (f00.row != i) { /* the first touch from this level */
                    newId[j] = DdNodeId(i, m);
                }
                else {
                    newId[j] = f00;
                }
                f00 = f;
                assert(newId[j].row == i);
                assert(newId[j] > f);
            }

            {
                MyVector<int> const& levels = nodeTable->lowerLevels(i);
                for (int const* t = levels.begin(); t != levels.end(); ++t) {
                    newIdTable.clear(*t);
                }
            }
            size_t mm = 0;

            for (size_t j = 0; j < m; ++j) {
                DdNodeId const f(i, j);
                if (newId[j] <= f) continue;
                assert(newId[j].row == i);

                for (size_t k = j; k < m;) {
                    assert(j <= k);
                    DdNodeId const g(i, k);
                    DdNodeId& g0 = tt[k].branch[0];
                    DdNodeId& g1 = tt[k].branch[1];
                    DdNodeId& g10 = (*nodeTable)[g1.row][g1.col].branch[0];
                    DdNodeId& g11 = (*nodeTable)[g1.row][g1.col].branch[1];
                    if (g11 != f) {
                        g11 = f;
                        g10 = g;
                        ++mm;
                    }
                    else {
                        g0 = g10;
                        g1 = 0;
                    }
                    assert(newId[k].row == i);
                    size_t next = newId[k].col;
                    newId[k] = 0;
                    k = next;
                }
            }

            {
                MyVector<int> const& levels = nodeTable->lowerLevels(i);
                for (int const* t = levels.begin(); t != levels.end(); ++t) {
                    nodeTable.derefLevel(*t);
                }
            }

            DdNode* nt = tmpTable->initRow(i, mm);
            size_t k = 0;

            for (size_t j = 0; j < m; ++j) {
                if (tt[j].branch[1] != 0) {
                    nt[k].branch[0] = tt[j].branch[0];
                    nt[k].branch[1] = tt[j].branch[1];
                    newId[j] = DdNodeId(i, k++);
                }
                else {
                    DdNodeId f0 = tt[j].branch[0];
                    if (f0.row == i) newId[j] = newId[f0.col];
                }
            }

            assert(k == mm);
        }

        nodeTable = tmpTable;
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
            }
            else if (root == 0) {
                os << "  \"" << DdNodeId(0)
                        << "\" [shape=square,label=\"0\"];\n";
            }
            else {
                os << "  \"" << DdNodeId(1)
                        << "\" [shape=square,label=\"1\"];\n";
            }

            os << "  {rank=same; " << i;
            for (size_t j = 0; j < m; ++j) {
                os << "; \"" << DdNodeId(i, j) << "\"";
            }
            os << "}\n";
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
