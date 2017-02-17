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
#include <cstring>
#include <stdexcept>

namespace tdzdd {

template<typename T, size_t BLOCK_ELEMENTS = 1000>
class MyList {
    static int const headerCells = 1;

    struct Cell {
        Cell* next;
    };

    Cell* front_;
    size_t size_;

    static size_t numCells(size_t n) {
        return (n + sizeof(Cell) - 1) / sizeof(Cell);
    }

    static Cell* setFlag(Cell* p) {
        return reinterpret_cast<Cell*>(reinterpret_cast<size_t>(p) | 1);
    }

    static Cell* clearFlag(Cell* p) {
        static size_t const mask = ~size_t(0) << 1;
        return reinterpret_cast<Cell*>(reinterpret_cast<size_t>(p) & mask);
    }

    static bool flagged(Cell* p) {
        return reinterpret_cast<size_t>(p) & 1;
    }

    static Cell*& blockStart(Cell* p) {
        return p[-1].next;
    }

    static T* dataStart(Cell* p) {
        return reinterpret_cast<T*>(p + 1);
    }

public:
    MyList()
            : front_(0), size_(0) {
    }

    MyList(MyList const& o)
            : front_(0), size_(0) {
        if (o.size_ != 0) throw std::runtime_error(
                "MyList can't be copied unless it is empty!"); //FIXME
    }

    MyList& operator=(MyList const& o) {
        if (o.size_ != 0) throw std::runtime_error(
                "MyList can't be copied unless it is empty!"); //FIXME
        clear();
        return *this;
    }

//    MyList(MyList&& o) {
//        *this = std::move(o);
//    }

//    MyList& operator=(MyList&& o) {
//        front_ = o.front_;
//        o.front_ = 0;
//        return *this;
//    }

    virtual ~MyList() {
        clear();
    }

    /**
     * Returns the number of elements.
     * @return the number of elements.
     */
    size_t size() const {
        return size_;
    }

    /**
     * Checks emptiness.
     * @return true if empty.
     */
    bool empty() const {
        assert((front_ == 0) == (size_ == 0));
        return front_ == 0;
    }

    /**
     * Initializes the array to be empty.
     * The memory is deallocated.
     */
    void clear() {
        while (front_) {
            Cell* p = front_;

            while (!flagged(p)) {
                p = p->next;
            }

            delete[] blockStart(front_);
            front_ = clearFlag(p);
        }
        size_ = 0;
    }

    /**
     * Accesses the first element.
     * @return pointer to the first element.
     */
    T* front() {
        return dataStart(front_);
    }

    /**
     * Accesses the first element.
     * @return pointer to the first element.
     */
    T const* front() const {
        return dataStart(front_);
    }

    /**
     * Allocates a contiguous memory block for one or more new elements
     * at the beginning.
     * The memory block is not initialized.
     * @param numElements the number of elements.
     * @return pointer to the memory block.
     */
    T* alloc_front(size_t numElements = 1) {
        size_t const n = numCells(numElements * sizeof(T)) + 1;

        if (front_ == 0 || front_ < blockStart(front_) + headerCells + n) {
            size_t const m = headerCells + n * BLOCK_ELEMENTS;
            Cell* newBlock = new Cell[m];
            Cell* newFront = newBlock + m - n;
            blockStart(newFront) = newBlock;
            newFront->next = setFlag(front_);
            front_ = newFront;
        }
        else {
            Cell* newFront = front_ - n;
            blockStart(newFront) = blockStart(front_);
            newFront->next = front_;
            front_ = newFront;
        }
        ++size_;

        return dataStart(front_);
    }

    /**
     * Removes a memory block at the beginning.
     */
    void pop_front() {
        //front().~T(); I don't care about destruction!
        Cell* next = front_->next;

        if (flagged(next)) {
            delete[] blockStart(front_);
            front_ = clearFlag(next);
        }
        else {
            blockStart(next) = blockStart(front_);
            front_ = next;
        }
        --size_;
    }

    class iterator {
        Cell* front;

    public:
        iterator(Cell* front)
                : front(front) {
        }

        T* operator*() const {
            return dataStart(front);
        }

        T* operator->() const {
            return dataStart(front);
        }

        iterator& operator++() {
            front = clearFlag(front->next);
            return *this;
        }

        bool operator==(iterator const& o) const {
            return front == o.front;
        }

        bool operator!=(iterator const& o) const {
            return front != o.front;
        }
    };

    class const_iterator {
        Cell const* front;

    public:
        const_iterator(Cell const* front)
                : front(front) {
        }

        T const* operator*() const {
            return *dataStart(front);
        }

        T const* operator->() const {
            return dataStart(front);
        }

        const_iterator& operator++() {
            front = clearFlag(front->next);
            return *this;
        }

        bool operator==(iterator const& o) const {
            return front == o.front;
        }

        bool operator!=(iterator const& o) const {
            return front != o.front;
        }
    };

    /**
     * Returns an iterator to the beginning.
     * @return iterator to the beginning.
     */
    iterator begin() {
        return iterator(front_);
    }

    /**
     * Returns an iterator to the beginning.
     * @return iterator to the beginning.
     */
    const_iterator begin() const {
        return const_iterator(front_);
    }

    /**
     * Returns an iterator to the end.
     * @return iterator to the end.
     */
    iterator end() {
        return iterator(0);
    }

    /**
     * Returns an iterator to the end.
     * @return iterator to the end.
     */
    const_iterator end() const {
        return const_iterator(0);
    }

//    /**
//     * Get the hash code of this object.
//     * @return the hash code.
//     */
//    size_t hash() const {
//        size_t h = size_;
//        for (size_t i = 0; i < size_; ++i) {
//            h = h * 31 + array_[i].hash();
//        }
//        return h;
//    }
//
//    /**
//     * Check equivalence between another object.
//     * @param o another object.
//     * @return true if equivalent.
//     */
//    bool operator==(MyList const& o) const {
//        if (size_ != o.size_) return false;
//        for (size_t i = 0; i < size_; ++i) {
//            if (!(array_[i] == o.array_[i])) return false;
//        }
//        return true;
//    }

    /**
     * Sends an object to an output stream.
     * @param os the output stream.
     * @param o the object.
     * @return os.
     */
    friend std::ostream& operator<<(std::ostream& os, MyList const& o) {
        bool cont = false;
        os << "(";
        for (const_iterator t = o.begin(); t != o.end(); ++t) {
            if (cont) os << ",";
            os << **t;
            cont = true;
        }
        return os << ")";
    }
};

template<typename T>
class MyListOnPool {
    struct Cell {
        Cell* next;
    };

    Cell* front_;
    size_t size_;

    static size_t dataCells(size_t n) {
        return (n + sizeof(Cell) - 1) / sizeof(Cell);
    }

    static size_t workCells(size_t n) {
        return 1 + dataCells(n);
    }

    static T* dataStart(Cell* p) {
        return reinterpret_cast<T*>(p + 1);
    }

    static T const* dataStart(Cell const* p) {
        return reinterpret_cast<T const*>(p + 1);
    }

public:
    MyListOnPool()
            : front_(0), size_(0) {
    }

    MyListOnPool(MyListOnPool const& o) {
        *this = o;
    }

    MyListOnPool& operator=(MyListOnPool const& o) {
        if (!o.empty()) throw std::runtime_error(
                "MyListOnPool: Can't copy a nonempty object.");

        front_ = 0;
        size_ = 0;
        return *this;
    }

//    MyListOnPool(MyListOnPool&& o) {
//        *this = std::move(o);
//    }
//
//    MyListOnPool& operator=(MyListOnPool&& o) {
//        front_ = o.front_;
//        size_ = o.size_;
//        o.front_ = 0;
//        o.size_ = 0;
//        return *this;
//    }

    virtual ~MyListOnPool() {
        clear();
    }

    /**
     * Returns the number of elements.
     * @return the number of elements.
     */
    size_t size() const {
        return size_;
    }

    /**
     * Checks emptiness.
     * @return true if empty.
     */
    bool empty() const {
        assert((front_ == 0) == (size_ == 0));
        return front_ == 0;
    }

    /**
     * Initializes the array to be empty.
     * The memory is left unused on the pool.
     */
    void clear() {
        front_ = 0;
        size_ = 0;
    }

    /**
     * Accesses the first element.
     * @return pointer to the first element.
     */
    T* front() {
        return dataStart(front_);
    }

    /**
     * Accesses the first element.
     * @return pointer to the first element.
     */
    T const* front() const {
        return dataStart(front_);
    }

    /**
     * Allocates a contiguous memory block for one or more new elements
     * at the beginning.
     * The memory block is not initialized.
     * @param pool memory pool for allocating new entries.
     * @param numElements the number of elements.
     * @return pointer to the memory block.
     */
    template<typename Pool>
    T* alloc_front(Pool& pool, size_t numElements = 1) {
        size_t const n = workCells(numElements * sizeof(T));
        Cell* newFront = pool.template allocate<Cell>(n);
        newFront->next = front_;
        front_ = newFront;
        ++size_;

        return dataStart(front_);
    }

    /**
     * Removes a memory block at the beginning.
     */
    void pop_front() {
        front_ = front_->next;
        --size_;
    }

    class iterator {
        Cell* front;

    public:
        iterator(Cell* front)
                : front(front) {
        }

        T* operator*() const {
            return dataStart(front);
        }

        iterator& operator++() {
            front = front->next;
            return *this;
        }

        bool operator==(iterator const& o) const {
            return front == o.front;
        }

        bool operator!=(iterator const& o) const {
            return front != o.front;
        }
    };

    class const_iterator {
        Cell const* front;

    public:
        const_iterator(Cell const* front)
                : front(front) {
        }

        T const* operator*() const {
            return dataStart(front);
        }

        const_iterator& operator++() {
            front = front->next;
            return *this;
        }

        bool operator==(const_iterator const& o) const {
            return front == o.front;
        }

        bool operator!=(const_iterator const& o) const {
            return front != o.front;
        }
    };

    /**
     * Returns an iterator to the beginning.
     * @return iterator to the beginning.
     */
    iterator begin() {
        return iterator(front_);
    }

    /**
     * Returns an iterator to the beginning.
     * @return iterator to the beginning.
     */
    const_iterator begin() const {
        return const_iterator(front_);
    }

    /**
     * Returns an iterator to the end.
     * @return iterator to the end.
     */
    iterator end() {
        return iterator(0);
    }

    /**
     * Returns an iterator to the end.
     * @return iterator to the end.
     */
    const_iterator end() const {
        return const_iterator(0);
    }

//    /**
//     * Get the hash code of this object.
//     * @return the hash code.
//     */
//    size_t hash() const {
//        size_t h = size_;
//        for (size_t i = 0; i < size_; ++i) {
//            h = h * 31 + array_[i].hash();
//        }
//        return h;
//    }
//
//    /**
//     * Check equivalence between another object.
//     * @param o another object.
//     * @return true if equivalent.
//     */
//    bool operator==(MyListOnPool const& o) const {
//        if (size_ != o.size_) return false;
//        for (size_t i = 0; i < size_; ++i) {
//            if (!(array_[i] == o.array_[i])) return false;
//        }
//        return true;
//    }

    /**
     * Sends an object to an output stream.
     * @param os the output stream.
     * @param o the object.
     * @return os.
     */
    friend std::ostream& operator<<(std::ostream& os, MyListOnPool const& o) {
        bool cont = false;
        os << "(";
        for (const_iterator t = o.begin(); t != o.end(); ++t) {
            if (cont) os << ",";
            os << **t;
            cont = true;
        }
        return os << ")";
    }
};

} // namespace tdzdd
