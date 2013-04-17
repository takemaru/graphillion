/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: DataTable.hpp 411 2013-02-14 09:16:45Z iwashita $
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <ostream>

template<typename T>
class DataTable {
    int numRows_;
    size_t* rowSize_;
    T** data;

    /**
     * Imports a table and clear it.
     * @param o the table.
     */
    template<typename U>
    void copyToThis(DataTable<U> const& o) {
        int const n = o.numRows();
        init(n);

        for (int i = 0; i < n; ++i) {
            size_t const m = o.rowSize(i);
            U const* p = o[i];
            U const* pe = p + m;
            T* q = initRow(i, m);

            while (p != pe) {
                *q++ = *p++;
            }
        }
    }

//    /**
//     * Imports a table and clear it.
//     * @param o the table.
//     */
//    template<typename U>
//    void importToThis(DataTable<U>& o) {
//        int const n = o.numRows();
//        init(n);
//
//        for (int i = 0; i < n; ++i) {
//            size_t const m = o.rowSize(i);
//            U const* p = o[i];
//            U const* pe = p + m;
//            T* q = initRow(i, m);
//
//            while (p != pe) {
//                *q++ = *p++;
//            }
//
//            o.clear(i);
//        }
//
//        o.init(0);
//    }

//    DataTable(DataTable const& o);
//    DataTable& operator=(DataTable const& o);

public:
    /**
     * Constructor.
     * @param n the number of rows.
     */
    DataTable(int n = 0)
            : numRows_(0), rowSize_(0), data(0) {
        init(n);
    }

//    DataTable(DataTable const& o)
//            : numRows_(0), rowSize_(0), data(0) {
//        copyToThis(o);
//    }

//    DataTable& operator=(DataTable const& o) {
//        copyToThis(o);
//        return *this;
//    }

    template<typename U>
    DataTable(DataTable<U> const& o)
            : numRows_(0), rowSize_(0), data(0) {
        copyToThis(o);
    }

    template<typename U>
    DataTable& operator=(DataTable<U> const& o) {
        copyToThis(o);
        return *this;
    }

//    DataTable(DataTable&& o): numRows_(0), rowSize_(0), data(0) {
//        *this = std::move(o);
//    }

//    DataTable& operator=(DataTable&& o) {
//        this->~DataTable();
//        numRows_ = o.numRows_;
//        rowSize_ = o.rowSize_;
//        data = o.data;
//
//        o.numRows_ = 0;
//        o.rowSize_ = 0;
//        o.data = 0;
//        return *this;
//    }

    /*
     * Imports the table that can be broken.
     * @param o the table.
     */
    void moveAssign(DataTable& o) {
        this->~DataTable();
        numRows_ = o.numRows_;
        rowSize_ = o.rowSize_;
        data = o.data;

        o.numRows_ = 0;
        o.rowSize_ = 0;
        o.data = 0;
    }

//    template<typename CopyType>
//    DataTable(DataTable<CopyType>&& o): numRows_(0), rowSize_(0), data(0) {
//        importToThis(o);
//    }

//    template<typename CopyType>
//    DataTable& operator=(DataTable<CopyType>&& o) {
//        importToThis(o);
//        return *this;
//    }

    /**
     * Destructor.
     */
    virtual ~DataTable() {
        for (int i = 0; i < numRows_; ++i) {
            delete[] data[i];
        }
        delete[] data;
        delete[] rowSize_;
    }

    /**
     * Clear and initialize the table.
     * @param n the number of rows.
     */
    void init(int n) {
        this->~DataTable();
        numRows_ = n;
        rowSize_ = new size_t[n]();
        data = new T*[n]();
    }

    /**
     * Clear a row.
     * @param i row index.
     */
    void clear(int i) {
        assert(0 <= i && i < numRows_);
        rowSize_[i] = 0;
        delete[] data[i];
        data[i] = 0;
    }

    /**
     * Clear and initialize a row.
     * @param i row index.
     * @param size new size of the row.
     * @return array of elements on the new row.
     */
    T* initRow(int i, size_t size) {
        assert(0 <= i && i < numRows_);
        rowSize_[i] = size;
        delete[] data[i];
        return data[i] = (size == 0) ? 0 : new T[size]();
    }

    /**
     * Clear and initialize a row.
     * @param i row index.
     * @param size new size of the row.
     * @param initVal initial value of elements.
     * @return array of elements on the new row.
     */
    T* initRow(int i, size_t size, T const& initVal) {
        assert(0 <= i && i < numRows_);
        rowSize_[i] = size;
        delete[] data[i];
        if (size == 0) return data[i] = 0;

        T* row = data[i] = new T[size];
        for (size_t j = 0; j < size; ++j) {
            row[j] = initVal;
        }
        return row;
    }

    /**
     * Resize a row.
     * Elements are copied to the new array.
     * @param i row index.
     * @param size new size of the row.
     * @return array of elements on the new row.
     */
    T* resizeRow(int i, size_t size) {
        assert(0 <= i && i < numRows_);
        T* src = data[i];
        T* dst = new T[size]();
        size_t const m = std::min(rowSize_[i], size);
        for (size_t j = 0; j < m; ++j) {
            dst[j] = src[j];
        }
        delete[] src;
        rowSize_[i] = size;
        return data[i] = dst;
    }

    /**
     * Get the number of rows.
     * @return the number of rows.
     */
    int numRows() const {
        return numRows_;
    }

    /**
     * Get the number of elements in a row.
     * @param i row index.
     * @return the number of elements in the row.
     */
    size_t rowSize(int i) const {
        assert(0 <= i && i < numRows_);
        return rowSize_[i];
    }

    /**
     * Get the total number of elements in the table.
     * @return the total number of elements in the table.
     */
    size_t totalSize() const {
        size_t k = 0;
        for (int i = 0; i < numRows_; ++i) {
            k += rowSize_[i];
        }
        return k;
    }

    /**
     * Access to a row.
     * @param i row index.
     * @return array of elements on the row.
     */
    T* operator[](int i) {
        assert(0 <= i && i < numRows_);
        return data[i];
    }

    /**
     * Access to a row.
     * @param i row index.
     * @return array of elements on the row.
     */
    T const* operator[](int i) const {
        assert(0 <= i && i < numRows_);
        return data[i];
    }

    friend std::ostream& operator<<(std::ostream& os, DataTable const& o) {
        for (int i = 0; i < o.numRows(); ++i) {
            os << i << ": ";
            for (size_t j = 0; j < o.rowSize(i); ++j) {
                if (j != 0) os << ", ";
                os << o.data[i][j];
            }
            os << "\n";
        }
        return os;
    }
};
