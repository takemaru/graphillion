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

    mutable MyVector<MyVector<int> > higherLevelTable;
    mutable MyVector<MyVector<int> > lowerLevelTable;

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
    void init(int n = 1) {
        assert(n >= 1);
        base::init(n);
        deleteIndex();
    }

    /**
     * Deletes current index information.
     */
    void deleteIndex() {
        higherLevelTable.clear();
        lowerLevelTable.clear();
    }

    /**
     * Makes index information.
     */
    void makeIndex() const {
        int const n = numRows() - 1;
        higherLevelTable.clear();
        higherLevelTable.resize(n + 1);
        lowerLevelTable.clear();
        lowerLevelTable.resize(n + 1);
        MyVector<bool> lowerMark(n + 1);

        for (int i = n; i >= 1; --i) {
            size_t const m = rowSize(i);
            DdNode const* const node = (*this)[i];
            MyVector<int>& lower = lowerLevelTable[i];
            int lowest = i;

            for (size_t j = 0; j < m; ++j) {
                for (int b = 0; b <= 1; ++b) {
                    int const ii = node[j].branch[b].row;
                    if (ii == 0) continue;

                    if (ii < lowest) lowest = ii;

                    if (!lowerMark[ii]) {
                        lower.push_back(ii);
                        lowerMark[ii] = true;
                    }
                }
            }

            std::sort(lower.begin(), lower.end());
            higherLevelTable[lowest].push_back(i);
        }
    }

    /**
     * Returns a collection of the higher levels that directly refers
     * the given level and that does not refer any lower levels.
     * @param level the level.
     */
    MyVector<int> const& higherLevels(int level) const {
        if (higherLevelTable.empty()) makeIndex();
        return higherLevelTable[level];
    }

    /**
     * Returns a collection of the lower levels that are referred
     * by the given level and that are not referred directly by
     * any higher levels.
     * @param level the level.
     */
    MyVector<int> const& lowerLevels(int level) const {
        if (lowerLevelTable.empty()) makeIndex();
        return lowerLevelTable[level];
    }
};

class DdNodeTableHandler {
    struct Object {
        unsigned refCount;
        DdNodeTable entity;

        Object(int n)
                : refCount(1), entity(n) {
        }

        Object(DdNodeTable const& entity)
                : refCount(1), entity(entity) {
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

    DdNodeTable const& operator*() const {
        return pointer->entity;
    }

    DdNodeTable const* operator->() const {
        return &pointer->entity;
    }

    /**
     * Make the table unshared.
     * @return writable reference to the private table.
     */
    DdNodeTable& privateEntity() {
        if (pointer->refCount >= 2) {
            pointer->deref();
            pointer = new Object(pointer->entity);
#ifdef DEBUG
            std::cerr << "DdNodeTableHandler::privateEntity()";
#endif
        }
        return pointer->entity;
    }

    /**
     * Clear and initialize the table.
     * @param n the number of rows.
     * @return writable reference to the private table.
     */
    DdNodeTable& init(int n = 1) {
        if (pointer->refCount == 1) {
            pointer->entity.init(n);
        }
        else {
            pointer->deref();
            pointer = new Object(n);
        }
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
