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
#include <ostream>

#include "../util/MyVector.hpp"

namespace tdzdd {

template<typename T>
class DataTable {
    MyVector<MyVector<T> > table;

//    DataTable(DataTable const& o);
//    DataTable& operator=(DataTable const& o);

public:
    /**
     * Constructor.
     * @param n the number of rows.
     */
    DataTable(int n = 0)
            : table(n) {
    }

    DataTable(DataTable const& o)
            : table(o.table) {
    }

    DataTable& operator=(DataTable const& o) {
        table = o.table;
        return *this;
    }

//    template<typename U>
//    DataTable(DataTable<U> const& o)
//            : table(o.table) {
//    }
//
//    template<typename U>
//    DataTable& operator=(DataTable<U> const& o) {
//        table = o.table;
//        return *this;
//    }

//    /*
//     * Imports the table that can be broken.
//     * @param o the table.
//     */
//    void moveFrom(DataTable& o) {
//        this->~DataTable();
//        numRows_ = o.numRows_;
//        rowSize_ = o.rowSize_;
//        rowCapacity_ = o.rowCapacity_;
//        table = o.table;
//
//        o.numRows_ = 0;
//        o.rowSize_ = 0;
//        o.rowCapacity_ = 0;
//        o.table = 0;
//    }

    /**
     * Clears and initializes the table.
     * @param n the number of rows.
     */
    void init(int n = 0) {
        table.clear();
        table.resize(n);
    }

    /**
     * Resizes the table rows.
     * @param n the number of rows.
     */
    void setNumRows(int n) {
        table.resize(n);
    }

    /**
     * Clears and initializes a row.
     * @param i row index.
     * @param size new size of the row.
     */
    void initRow(int i, size_t size) {
        table[i].clear();
        table[i].resize(size);
    }

//    /**
//     * Clears and initializes a row.
//     * @param i row index.
//     * @param size new size of the row.
//     * @param initVal initial value of elements.
//     * @return the new array for the row.
//     */
//    T* initRow(int i, size_t size, T const& initVal) {
//        assert(0 <= i && i < numRows_);
//        rowSize_[i] = rowCapacity_[i] = size;
//        delete[] table[i];
//        if (size == 0) return table[i] = 0;
//
//        T* row = table[i] = new T[size];
//        for (size_t j = 0; j < size; ++j) {
//            row[j] = initVal;
//        }
//        return row;
//    }

    /**
     * Adds one column to a row.
     * @param i row index.
     * @return the new column index.
     */
    size_t addColumn(int i) {
        table[i].push_back(T());
        return table[i].size() - 1;
    }

    /**
     * Gets the number of rows.
     * @return the number of rows.
     */
    int numRows() const {
        return table.size();
    }

    /**
     * Gets the total number of elements in the table.
     * @return the total number of elements in the table.
     */
    size_t totalSize() const {
        size_t k = 0;
        for (size_t i = 0; i < table.size(); ++i) {
            k += table[i].size();
        }
        return k;
    }

    /**
     * Accesses to a row.
     * @param i row index.
     * @return vector of elements on the row.
     */
    MyVector<T>& operator[](int i) {
        return table[i];
    }

    /**
     * Accesses to a row.
     * @param i row index.
     * @return vector of elements on the row.
     */
    MyVector<T> const& operator[](int i) const {
        return table[i];
    }

    friend std::ostream& operator<<(std::ostream& os, DataTable const& o) {
        for (int i = 0; i < o.numRows(); ++i) {
            os << i << ": ";
            for (size_t j = 0; j < o[i].size(); ++j) {
                if (j != 0) os << ", ";
                os << o.table[i][j];
            }
            os << "\n";
        }
        return os;
    }
};

} // namespace tdzdd
