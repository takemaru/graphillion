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
#include <cmath>
#include <ostream>
#include <stdexcept>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "Node.hpp"
#include "NodeTable.hpp"
#include "../util/MyHashTable.hpp"
#include "../util/MyList.hpp"
#include "../util/MyVector.hpp"

namespace tdzdd {

template<int ARITY, bool BDD, bool ZDD>
class DdReducer {
    NodeTableEntity<ARITY>& input;
    NodeTableHandler<ARITY> oldDiagram;
    NodeTableHandler<ARITY> newDiagram;
    NodeTableEntity<ARITY>& output;
    MyVector<MyVector<NodeId> > newIdTable;
    MyVector<MyVector<NodeId*> > rootPtr;

    struct ReducNodeInfo {
        Node<ARITY> children;
        size_t column;

        size_t hash() const {
            return children.hash();
        }

        bool operator==(ReducNodeInfo const& o) const {
            return children == o.children;
        }

        friend std::ostream& operator<<(std::ostream& os,
                                        ReducNodeInfo const& o) {
            return os << "(" << o.children << " -> " << o.column << ")";
        }
    };

#ifdef _OPENMP
    static int const TASKS_PER_THREAD = 10;

    int const threads;
    int const tasks;
    MyVector<MyVector<MyList<ReducNodeInfo> > > taskMatrix;
    MyVector<size_t> baseColumn;

#ifdef DEBUG
    ElapsedTimeCounter etcP1, etcP2, etcP3, etcS0, etcS1, etcS2, etcS3, etcS4;
#endif
#endif

    bool readyForSequentialReduction;

public:
    DdReducer(NodeTableHandler<ARITY>& diagram, bool useMP = false) :
            input(diagram.privateEntity()),
            oldDiagram(diagram),
            newDiagram(input.numRows()),
            output(newDiagram.privateEntity()),
            newIdTable(input.numRows()),
            rootPtr(input.numRows()),
#ifdef _OPENMP
            threads(omp_get_max_threads()),
            tasks(MyHashConstant::primeSize(TASKS_PER_THREAD * threads)),
            taskMatrix(threads),
            baseColumn(tasks + 1),
#endif
            readyForSequentialReduction(false) {
#ifdef _OPENMP
#ifdef DEBUG
        if (useMP) {
            MessageHandler mh;
            mh << "#thread = " << threads << ", #task = " << tasks;
        }
        etcS0.start();
#endif
#endif
        diagram = newDiagram;

        input.initTerminals();
        input.makeIndex(useMP);

        newIdTable[0].resize(2);
        newIdTable[0][0] = 0;
        newIdTable[0][1] = 1;

#ifdef _OPENMP
        for (int y = 0; y < threads; ++y) {
            taskMatrix[y].resize(tasks);
        }
#ifdef DEBUG
        etcS0.stop();
#endif
#endif
    }

#ifdef _OPENMP
#ifdef DEBUG
    ~DdReducer() {
        if (etcP1 != 0) {
            MessageHandler mh;
            mh << "P1: " << etcP1 << "\n";
            mh << "P2: " << etcP2 << "\n";
            mh << "P3: " << etcP3 << "\n";
            mh << "S0: " << etcS0 << "\n";
            mh << "S1: " << etcS1 << "\n";
            mh << "S2: " << etcS2 << "\n";
            mh << "S3: " << etcS3 << "\n";
            mh << "S4: " << etcS4 << "\n";
        }
    }
#endif
#endif

private:
    /**
     * Applies the node deletion rules.
     * It is required before serial reduction (Algorithm-R)
     * in order to make lower-level index safe.
     */
    void makeReadyForSequentialReduction() {
        if (readyForSequentialReduction) return;
#ifdef DEBUG
        size_t dead = 0;
#endif
        for (int i = 2; i < input.numRows(); ++i) {
            size_t const m = input[i].size();
            Node<ARITY>* const tt = input[i].data();

            for (size_t j = 0; j < m; ++j) {
                for (int b = 0; b < ARITY; ++b) {
                    NodeId& f = tt[j].branch[b];
                    if (f.row() == 0) continue;

                    NodeId f0 = input.child(f, 0);
                    NodeId deletable = BDD ? f0 : 0;
                    bool del = true;

                    for (int bb = (BDD || ZDD) ? 1 : 0; bb < ARITY; ++bb) {
                        if (input.child(f, bb) != deletable) {
                            del = false;
                        }
                    }

                    if (del) {
                        f = f0;
#ifdef DEBUG
                        if (f == 0) ++dead;
#endif
                    }
                }
            }
        }
#ifdef DEBUG
        MessageHandler mh;
        mh << "[#dead = " << dead << "]";
#endif
        input.makeIndex();
        readyForSequentialReduction = true;
    }

public:
    /**
     * Sets a root node.
     * @param root reference to a root node ID storage.
     */
    void setRoot(NodeId& root) {
        rootPtr[root.row()].push_back(&root);
    }

    /**
     * Reduces one level.
     * @param i level.
     * @param useMP use an algorithm for multiple processors.
     */
    void reduce(int i, bool useMP = false) {
        if (useMP) {
            reduceMP_(i);
        }
        else if (ARITY == 2) {
            algorithmR(i);
        }
        else {
            reduce_(i);
        }
    }

private:
    /**
     * Reduces one level using Algorithm-R.
     * @param i level.
     */
    void algorithmR(int i) {
        assert(ARITY == 2);
        makeReadyForSequentialReduction();
        size_t const m = input[i].size();
        Node<ARITY>* const tt = input[i].data();
        NodeId const mark(i, m);

        MyVector<NodeId>& newId = newIdTable[i];
        newId.resize(m);

        for (size_t j = m - 1; j + 1 > 0; --j) {
            NodeId& f0 = tt[j].branch[0];
            NodeId& f1 = tt[j].branch[1];

            if (f0.row() != 0) f0 = newIdTable[f0.row()][f0.col()];
            if (f1.row() != 0) f1 = newIdTable[f1.row()][f1.col()];

            if ((BDD && f1 == f0) || (ZDD && f1 == 0)) {
                newId[j] = f0;
            }
            else {
                NodeId& f00 = input.child(f0, 0);
                NodeId& f01 = input.child(f0, 1);

                if (f01 != mark) {        // the first touch from this level
                    f01 = mark;        // mark f0 as touched
                    newId[j] = NodeId(i + 1, m); // tail of f0-equivalent list
                }
                else {
                    newId[j] = f00;         // next of f0-equivalent list
                }
                f00 = NodeId(i + 1, j);  // new head of f0-equivalent list
            }
        }

        {
            MyVector<int> const& levels = input.lowerLevels(i);
            for (int const* t = levels.begin(); t != levels.end(); ++t) {
                newIdTable[*t].clear();
            }
        }
        size_t mm = 0;

        for (size_t j = 0; j < m; ++j) {
            NodeId const f(i, j);
            assert(newId[j].row() <= i + 1);
            if (newId[j].row() <= i) continue;

            for (size_t k = j; k < m;) { // for each g in f0-equivalent list
                assert(j <= k);
                NodeId const g(i, k);
                NodeId& g0 = tt[k].branch[0];
                NodeId& g1 = tt[k].branch[1];
                NodeId& g10 = input.child(g1, 0);
                NodeId& g11 = input.child(g1, 1);
                assert(g1 != mark);
                assert(newId[k].row() == i + 1);
                size_t next = newId[k].col();

                if (g11 != f) { // the first touch to g1 in f0-equivalent list
                    g11 = f; // mark g1 as touched
                    g10 = g; // record g as a canonical node for <f0,g1>
                    newId[k] = NodeId(i, mm++, g0.hasEmpty());
                }
                else {
                    g0 = g10;       // make a forward link
                    g1 = mark;      // mark g as forwarded
                    newId[k] = 0;
                }

                k = next;
            }
        }

        if (!BDD) {
            MyVector<int> const& levels = input.lowerLevels(i);
            for (int const* t = levels.begin(); t != levels.end(); ++t) {
                input[*t].clear();
            }
        }

        output.initRow(i, mm);
        Node<ARITY>* nt = output[i].data();

        for (size_t j = 0; j < m; ++j) {
            NodeId const& f0 = tt[j].branch[0];
            NodeId const& f1 = tt[j].branch[1];

            if (f1 == mark) { // forwarded
                assert(f0.row() == i);
                assert(newId[j] == 0);
                newId[j] = newId[f0.col()];
            }
            else if ((BDD && f1 == f0) || (ZDD && f1 == 0)) { // forwarded
                assert(newId[j].row() < i);
            }
            else {
                assert(newId[j].row() == i);
                size_t k = newId[j].col();
                nt[k] = tt[j];
            }
        }

        for (size_t k = 0; k < rootPtr[i].size(); ++k) {
            NodeId& root = *rootPtr[i][k];
            root = newId[root.col()];
        }
    }

    /**
     * Reduces one level.
     * @param i level.
     */
    void reduce_(int i) {
        size_t const m = input[i].size();
        newIdTable[i].resize(m);
        size_t jj = 0;

        {
            //MyList<ReducNodeInfo> rni;
            //MyHashTable<ReducNodeInfo const*> uniq(m * 2);
            MyHashTable<Node<ARITY> const*> uniq(m * 2);

            for (size_t j = 0; j < m; ++j) {
                Node<ARITY>* const p0 = input[i].data();
                Node<ARITY>& f = input[i][j];

                // make f canonical
                NodeId& f0 = f.branch[0];
                f0 = newIdTable[f0.row()][f0.col()];
                NodeId deletable = BDD ? f0 : 0;
                bool del = BDD || ZDD || (f0 == 0);
                for (int b = 1; b < ARITY; ++b) {
                    NodeId& ff = f.branch[b];
                    ff = newIdTable[ff.row()][ff.col()];
                    if (ff != deletable) del = false;
                }

                if (del) { // f is redundant
                    newIdTable[i][j] = f0;
                }
                else {
                    Node<ARITY> const* pp = uniq.add(&f);

                    if (pp == &f) {
                        newIdTable[i][j] = NodeId(i, jj++, f0.hasEmpty());
                    }
                    else {
                        newIdTable[i][j] = newIdTable[i][pp - p0];
                    }
                }
            }
        }

        MyVector<int> const& levels = input.lowerLevels(i);
        for (int const* t = levels.begin(); t != levels.end(); ++t) {
            newIdTable[*t].clear();
        }

        output.initRow(i, jj);

        for (size_t j = 0; j < m; ++j) {
            NodeId const& ff = newIdTable[i][j];
            if (ff.row() == i) {
                output[i][ff.col()] = input[i][j];
            }
        }

        input[i].clear();

        for (size_t k = 0; k < rootPtr[i].size(); ++k) {
            NodeId& root = *rootPtr[i][k];
            root = newIdTable[i][root.col()];
        }
    }

    /**
     * Reduces one level using OpenMP.
     * @param i level.
     */
    void reduceMP_(int i) {
#ifndef _OPENMP
        reduce_(i);
#else
#ifdef DEBUG
        etcS1.start();
#endif
        size_t const m = input[i].size();
        newIdTable[i].resize(m);
#ifdef DEBUG
        etcS1.stop();
        etcP1.start();
#endif

#pragma omp parallel
        {
            int y = omp_get_thread_num();
            MyHashTable<ReducNodeInfo const*> uniq;

#pragma omp for schedule(static)
            for (size_t j = 0; j < m; ++j) {
                Node<ARITY>& f = input[i][j];

                // make f canonical
                NodeId& f0 = f.branch[0];
                f0 = newIdTable[f0.row()][f0.col()];
                NodeId deletable = BDD ? f0 : 0;
                bool del = BDD || ZDD || (f0 == 0);
                for (int b = 1; b < ARITY; ++b) {
                    NodeId& ff = f.branch[b];
                    ff = newIdTable[ff.row()][ff.col()];
                    if (ff != deletable) del = false;
                }

                if (del) { // f is redundant
                    newIdTable[i][j] = f0;
                    continue;
                }

                // schedule a task
                int x = f.hash() % tasks;
                ReducNodeInfo* p = taskMatrix[y][x].alloc_front();
                p->children = f;
                p->column = j;
            }

#pragma omp single
            {
#ifdef DEBUG
                etcP1.stop();
                etcS2.start();
#endif
                MyVector<int> const& levels = input.lowerLevels(i);
                for (int const* t = levels.begin(); t != levels.end(); ++t) {
                    newIdTable[*t].clear();
                }
#ifdef DEBUG
                etcS2.stop();
                etcP2.start();
#endif
            }

#pragma omp for schedule(dynamic)
            for (int x = 0; x < tasks; ++x) {
                size_t mm = 0;
                for (int yy = 0; yy < threads; ++yy) {
                    mm += taskMatrix[yy][x].size();
                }
                if (mm == 0) {
                    baseColumn[x + 1] = 0;
                    continue;
                }

                uniq.initialize(mm * 2);
                size_t j = 0;

                for (int yy = 0; yy < threads; ++yy) {
                    MyList<ReducNodeInfo>& taskq = taskMatrix[yy][x];

                    for (typename MyList<ReducNodeInfo>::iterator t =
                            taskq.begin(); t != taskq.end(); ++t) {
                        ReducNodeInfo const* p = *t;
                        ReducNodeInfo const* pp = uniq.add(p);

                        if (pp == p) {
                            newIdTable[i][p->column] =
                                    NodeId(i + x,
                                           j++,
                                           p->children.branch[0].hasEmpty()); // row += task ID
                        }
                        else {
                            newIdTable[i][p->column] =
                                    newIdTable[i][pp->column];
                        }
                    }
                }

                baseColumn[x + 1] = j;
            }

            for (int x = 0; x < tasks; ++x) {
                taskMatrix[y][x].clear();
            }

#pragma omp single
            {
#ifdef DEBUG
                etcP2.stop();
                etcS3.start();
#endif
                for (int x = 1; x < tasks; ++x) {
                    baseColumn[x + 1] += baseColumn[x];
                }

//                for (int k = 1; k < tasks; k <<= 1) {
//#pragma omp for schedule(static)
//                    for (int x = k; x < tasks; ++x) {
//                        if (x & k) {
//                            int w = (x & ~k) | (k - 1);
//                            baseColumn[x + 1] += baseColumn[w + 1];
//                        }
//                    }
//                }

                output.initRow(i, baseColumn[tasks]);
#ifdef DEBUG
                etcS3.stop();
                etcP3.start();
#endif
            }

#pragma omp for schedule(static)
            for (size_t j = 0; j < m; ++j) {
                NodeId& ff = newIdTable[i][j];
                if (ff.row() >= i) {
                    ff = NodeId(i,
                                ff.col() + baseColumn[ff.row() - i],
                                ff.getAttr());
                    output[i][ff.col()] = input[i][j];
                }
            }
        }
#ifdef DEBUG
        etcP3.stop();
        etcS4.start();
#endif
        input[i].clear();

        for (size_t k = 0; k < rootPtr[i].size(); ++k) {
            NodeId& root = *rootPtr[i][k];
            root = newIdTable[i][root.col()];
        }
#ifdef DEBUG
        etcS4.stop();
#endif
#endif // _OPENMP
    }

public:
    void garbageCollect() {
        // Initialize marks
        for (int i = input.numRows() - 1; i > 0; --i) {
            size_t m = input[i].size();

            size_t r = rootPtr[i].size();
            MyVector<size_t> roots;
            roots.reserve(r + 1);
            for (size_t k = 0; k < r; ++k) {
                roots.push_back(rootPtr[i][k]->col());
            }
            if (r >= 2) {
                std::sort(roots.begin(), roots.end());
                roots.resize(std::unique(roots.begin(), roots.end())
                        - roots.begin());
            }
            roots.push_back(m);

            for (size_t j = 0, k = 0; j < m; ++j) {
                if (j < roots[k]) {
                    input.child(i, j, 0).setAttr(true);
                }
                else {
                    assert(j == roots[k]);
                    input.child(i, j, 0).setAttr(false);
                    ++k;
                }
            }
        }

        // Delete unnecessary nodes
        for (int i = input.numRows() - 1; i > 0; --i) {
            size_t m = input[i].size();
            for (size_t j = 0; j < m; ++j) {
                NodeId& f0 = input.child(i, j, 0);
                if (f0.getAttr()) {
                    f0 = 0;
                    for (int b = 1; b < ARITY; ++b) {
                        NodeId& ff = input.child(i, j, b);
                        ff = 0;
                    }
                }
                else {
                    input.child(f0, 0).setAttr(false);
                    for (int b = 1; b < ARITY; ++b) {
                        NodeId& ff = input.child(i, j, b);
                        input.child(ff, 0).setAttr(false);
                    }
                }
            }
        }
    }
};

} // namespace tdzdd
