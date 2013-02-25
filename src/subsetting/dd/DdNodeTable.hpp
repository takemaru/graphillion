/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: DdNodeTable.hpp 412 2013-02-15 01:13:19Z iwashita $
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <ostream>
#include <stdexcept>

#include "DdNodeId.hpp"
#include "DataTable.hpp"
#include "../util/MessageHandler.hpp"
#include "../util/MyVector.hpp"

class DdNodeTable: public DataTable<DdNode> {
    typedef DataTable<DdNode> base;

    mutable MyVector<MyVector<int> > levelJumpTable;

public:
    /**
     * Constructor.
     * @param n the number of rows.
     */
    DdNodeTable(int n = 1)
            : base(n) {
        assert(n >= 1);
    }

    /**
     * Clear and initialize the table.
     * @param n the number of rows.
     */
    void init(int n) {
        assert(n >= 1);
        base::init(n);
        deleteIndex();
    }

    /**
     * Deletes current index information.
     */
    void deleteIndex() {
        levelJumpTable.clear();
    }

    /**
     * Makes index information.
     */
    void makeIndex() const {
        int const n = numRows() - 1;
        levelJumpTable.clear();
        levelJumpTable.resize(n + 1);
        MyVector<bool> levelMark(n + 1);

        for (int i = n; i >= 1; --i) {
            size_t const m = rowSize(i);
            DdNode const* const nt = (*this)[i];
            MyVector<int>& ljt = levelJumpTable[i];

            for (size_t j = 0; j < m; ++j) {
                for (int c = 0; c <= 1; ++c) {
                    int const ii = nt[j].branch[c].row;
                    if (ii == 0) continue;

                    if (!levelMark[ii]) {
                        ljt.push_back(ii);
                        levelMark[ii] = true;
                    }
                }
            }

            std::sort(ljt.begin(), ljt.end());
        }
    }

    /**
     * Returns a collection of the lower levels that are referred
     * from the given level but are not referred
     * directly from the higher levels.
     * @param level the level.
     */
    MyVector<int> const& lowerLevels(int level) const {
        if (levelJumpTable.empty()) makeIndex();
        return levelJumpTable[level];
    }
};

class DdNodeTableHandler {
    struct Object {
        unsigned refCount;
        DdNodeTable entity;

        Object(int n)
                : refCount(1), entity(n) {
        }

        void ref() {
            ++refCount;
            if (refCount == 0) throw std::runtime_error("Too many references");
        }

        void deref() {
            --refCount;
            if (refCount == 0) delete this;
        }
    };

    Object* pointer;

public:
    DdNodeTableHandler(int n = 1)
            : pointer(new Object(n)) {
    }

    DdNodeTableHandler(DdNodeTableHandler const& o)
            : pointer(o.pointer) {
        pointer->ref();
    }

    DdNodeTableHandler& operator=(DdNodeTableHandler const& o) {
        pointer->deref();
        pointer = o.pointer;
        pointer->ref();
        return *this;
    }

    ~DdNodeTableHandler() {
        pointer->deref();
    }

    DdNodeTable* operator->() {
        return &pointer->entity;
    }

    DdNodeTable const* operator->() const {
        return &pointer->entity;
    }

    DdNodeTable& operator*() {
        return pointer->entity;
    }

    DdNodeTable const& operator*() const {
        return pointer->entity;
    }

    /**
     * Clear a row if it is not shared.
     * @param i row index.
     */
    void derefLevel(int i) {
        if (pointer->refCount == 1) pointer->entity.clear(i);
    }
};

/**
 * Table of node property values.
 * @param T type of the property value.
 */
template<typename T>
class DdNodeProperty {
    DdNodeTable const& nodeTable;
    DataTable<T> dataTable;

public:
    DdNodeProperty(DdNodeTable const& nodeTable)
            : nodeTable(nodeTable), dataTable(nodeTable.numRows()) {
        dataTable.initRow(0, 2);
    }

    /**
     * Deletes the the given level.
     * @param level the level.
     */
    void clear(int level) {
        dataTable.clear(level);
    }

    /**
     * Access to a row.
     * @param i row index.
     * @return array of elements on the row.
     */
    T* operator[](int i) {
        assert(0 <= i && i < nodeTable.numRows());

        if (dataTable[i] == 0) {
            dataTable.initRow(i, nodeTable.rowSize(i));
        }

        return dataTable[i];
    }
};
