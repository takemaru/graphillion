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
#include <vector>

namespace tdzdd {

template<typename T, typename Size = size_t>
class MyVector {
    Size capacity_;  ///< Size of the array.
    Size size_;      ///< Current number of elements.
    T* array_;         ///< Start address of the array.

    static T* allocate(Size n) {
        return std::allocator<T>().allocate(n);
    }

    static void deallocate(T* p, Size n) {
        std::allocator<T>().deallocate(p, n);
    }

    void ensureCapacity(Size capacity) {
        if (capacity_ < capacity) {
//            reserve(std::max(Size(16), capacity * 4));
            reserve(capacity * 2);
        }
    }

    void moveElement(T& from, T& to) {
        //new (&to) T(std::move(from));
        new (&to) T(from);
        from.~T();
    }

public:
    MyVector()
            : capacity_(0), size_(0), array_(0) {
    }

    MyVector(Size n)
            : capacity_(0), size_(0), array_(0) {
        resize(n);
    }

    MyVector(Size n, T const& val)
            : capacity_(0), size_(0), array_(0) {
        reserve(n);
        for (Size i = 0; i < n; ++i) {
            push_back(val);
        }
    }

//    template<typename ... Args>
//    MyVector(Size n, Args const&... args)
//            : capacity_(0), size_(0), array_(0) {
//        resize(n, args...);
//    }

//    MyVector(Size size, T&& val)
//    : capacity_(0), size_(0), array_(0) {
//        resize(size, std::move(val));
//    }

    MyVector(MyVector const& o)
            : capacity_(o.size_), size_(o.size_),
              array_(capacity_ ? allocate(capacity_) : 0) {
        for (Size i = 0; i < size_; ++i) {
            new (array_ + i) T(o[i]);
        }
    }

    MyVector& operator=(MyVector const& o) {
        resize(0);
        reserve(o.size_);
        size_ = o.size_;
        for (Size i = 0; i < size_; ++i) {
            new (array_ + i) T(o[i]);
        }
        return *this;
    }

    template<typename U>
    MyVector(std::vector<U> const& o)
            : capacity_(o.size()), size_(o.size()), array_(allocate(capacity_)) {
        for (Size i = 0; i < size_; ++i) {
            new (array_ + i) T(o[i]);
        }
    }

    template<typename U>
    MyVector& operator=(std::vector<U> const& o) {
        resize(0);
        reserve(o.size());
        size_ = o.size();
        for (Size i = 0; i < size_; ++i) {
            new (array_ + i) T(o[i]);
        }
        return *this;
    }

//    template<typename U>
//    MyVector(MyVector<U> const& o)
//            : capacity_(o.size_), size_(o.size_), array_(allocate(capacity_)) {
//        for (Size i = 0; i < size_; ++i) {
//            new (array_ + i) T(o[i]);
//        }
//    }
//
//    template<typename U>
//    MyVector& operator=(MyVector<U> const& o) {
//        resize(0);
//        reserve(o.size_);
//        size_ = o.size_;
//        for (Size i = 0; i < size_; ++i) {
//            new (array_ + i) T(o[i]);
//        }
//        return *this;
//    }

//    MyVector(MyVector&& o) {
//        *this = std::move(o);
//    }

//    MyVector & operator=(MyVector&& o) {
//        throw std::runtime_error("!!!");
//        capacity_ = o.capacity_;
//        size_ = o.size_;
//        array_ = o.array_;
//        o.capacity_ = 0;
//        o.size_ = 0;
//        o.array_ = 0;
//        return *this;
//    }

    void swap(MyVector& o) {
        std::swap(capacity_, o.capacity_);
        std::swap(size_, o.size_);
        std::swap(array_, o.array_);
    }

    ~MyVector() {
        clear();
    }

    /**
     * Gets the current capacity of the storage.
     * @return the storage capacity.
     */
    Size capacity() const {
        return capacity_;
    }

    /**
     * Gets the number of elements.
     * @return the number of elements.
     */
    Size size() const {
        return size_;
    }

    /**
     * Checks emptiness.
     * @return true if empty.
     */
    bool empty() const {
        return size_ == 0;
    }

    /**
     * Reserves the memory.
     * Data is moved if the actual capacity is extended.
     * @param capacity lower bound of the capacity.
     */
    void reserve(Size capacity) {
        if (capacity_ < capacity) {
            T* tmp = allocate(capacity);
            if (array_ != 0) {
                assert(0 <= size_ && size_ <= capacity_);
                for (Size i = 0; i < size_; ++i) {
                    moveElement(array_[i], tmp[i]);
                }
                deallocate(array_, capacity_);
            }
            array_ = tmp;
            capacity_ = capacity;
        }
    }

    /**
     * Initializes the array.
     * @param n new size.
     */
    void init(Size n) {
        while (0 < size_) {
            array_[--size_].~T();
        }
        resize(n);
    }

    /**
     * Resizes the array.
     * Data may be moved unless new size is the same as capacity.
     * @param n new size.
     */
    void resize(Size n) {
        assert(n >= 0);
        if (n == 0) {
            clear();
        }
        else if (capacity_ * 10 <= n * 11 && n <= capacity_) {
            while (n < size_) {
                array_[--size_].~T();
            }

            while (size_ < n) {
                new (array_ + size_++) T();
            }
        }
        else {
            while (n < size_) {
                array_[--size_].~T();
            }
            assert(size_ <= n);

            T* tmp = allocate(n);
            for (Size i = 0; i < size_; ++i) {
                moveElement(array_[i], tmp[i]);
            }

            while (size_ < n) {
                new (tmp + size_++) T();
            }

            deallocate(array_, capacity_);
            array_ = tmp;
            capacity_ = n;
        }
    }

    /**
     * Erases elements.
     * @param first pointer to the beginning.
     * @param last pointer to the end.
     * @return iterator following the last removed element.
     */
    T* erase(T* first, T* last) {
        assert(array_ <= first && first <= last && last <= array_ + size_);
        Size newSize = size_ - (last - first);

        for (Size i = first - array_; i < newSize; ++i) {
            array_[i] = last[i];
        }

        while (newSize < size_) {
            array_[--size_].~T();
        }

        return first;
    }

    /**
     * Initializes the array to be empty.
     * The memory is deallocated.
     */
    void clear() {
        if (array_ != 0) {
            assert(capacity_ > 0);
            while (size_ > 0) {
                array_[--size_].~T();
            }
            deallocate(array_, capacity_);
            array_ = 0;
        }
        capacity_ = 0;
    }

    /**
     * Adds an element to the end of the array.
     * The array is automatically extended,
     * which may cause data move.
     * @param val value of the new element.
     */
    void push_back(T const& val) {
        ensureCapacity(size_ + 1);
        new (array_ + size_) T(val);
        ++size_;
    }

    /**
     * Removes the last element.
     */
    void pop_back() {
        if (size_ > 0) array_[--size_].~T();
    }

//    /*
//     * Adds an element constructed in place to the end of the array.
//     * The array is automatically extended,
//     * which may cause data move.
//     * @param args arguments to element's constructor.
//     */
//    template<typename ... Args>
//    void emplace_back(Args const&... args) {
//        ensureCapacity(size_ + 1);
//        new (array_ + size_) T(args...);
//        ++size_;
//    }

    /**
     * Accesses to the last element.
     * @return reference to the last element.
     */
    T& back() {
        assert(size_ >= 1);
        return array_[size_ - 1];
    }

    /**
     * Accesses to the last element.
     * @return reference to the last element.
     */
    T const& back() const {
        assert(size_ >= 1);
        return array_[size_ - 1];
    }

    /**
     * Accesses to the i-th element.
     * @param i index of the element.
     * @return reference to the element.
     */
    T& operator[](Size i) {
        assert(i < size_);
        return array_[i];
    }

    /**
     * Accesses to the i-th element.
     * @param i index of the element.
     * @return reference to the element.
     */
    T const& operator[](Size i) const {
        assert(i < size_);
        return array_[i];
    }

    /**
     * Gets a pointer to the first element.
     * @return pointer to the first element.
     */
    T* data() const {
        return array_;
    }

    typedef T* iterator;
    typedef T const* const_iterator;

    /**
     * Gets a pointer to the first element.
     * @return pointer to the first element.
     */
    iterator begin() {
        return array_;
    }

    /**
     * Gets a pointer to the first element.
     * @return pointer to the first element.
     */
    const_iterator begin() const {
        return array_;
    }

    /**
     * Gets a pointer to the end of elements.
     * @return pointer to the end of elements.
     */
    iterator end() {
        return array_ + size_;
    }

    /**
     * Gets a pointer to the end of elements.
     * @return pointer to the end of elements.
     */
    const_iterator end() const {
        return array_ + size_;
    }

    template<typename U>
    class reverse_iterator_ {
        U* ptr;

    public:
        reverse_iterator_(U* ptr)
                : ptr(ptr) {
        }

        U& operator*() const {
            return *ptr;
        }

        U* operator->() const {
            return ptr;
        }

        reverse_iterator_& operator++() {
            --ptr;
            return *this;
        }

        bool operator==(reverse_iterator_ const& o) const {
            return ptr == o.ptr;
        }

        bool operator!=(reverse_iterator_ const& o) const {
            return ptr != o.ptr;
        }
    };

    typedef reverse_iterator_<T> reverse_iterator;
    typedef reverse_iterator_<T const> const_reverse_iterator;

    /**
     * Returns a reverse iterator to the beginning.
     * @return reverse iterator to the beginning.
     */
    reverse_iterator rbegin() {
        return reverse_iterator(array_ + size_ - 1);
    }

    /**
     * Returns a reverse iterator to the beginning.
     * @return reverse iterator to the beginning.
     */
    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(array_ + size_ - 1);
    }

    /**
     * Returns a reverse iterator to the end.
     * @return reverse iterator to the end.
     */
    reverse_iterator rend() {
        return reverse_iterator(array_ - 1);
    }

    /**
     * Returns a reverse iterator to the end.
     * @return reverse iterator to the end.
     */
    const_reverse_iterator rend() const {
        return const_reverse_iterator(array_ - 1);
    }

    /**
     * Removes an element.
     * @param pos iterator to the element to remove.
     */
    iterator erase(iterator pos) {
        assert(begin() <= pos && pos < end());
        std::memmove(pos, pos + 1, end() - pos - 1);
        return pos;
    }

    /**
     * Gets the hash code of this object.
     * @return the hash code.
     */
    size_t hash() const {
        size_t h = size_;
        for (Size i = 0; i < size_; ++i) {
            h = h * 31 + array_[i].hash();
        }
        return h;
    }

    /**
     * Checks equivalence between another object.
     * @param o another object.
     * @return true if equivalent.
     */
    bool operator==(MyVector const& o) const {
        if (size_ != o.size_) return false;
        for (Size i = 0; i < size_; ++i) {
            if (!(array_[i] == o.array_[i])) return false;
        }
        return true;
    }

    /**
     * Sends an object to an output stream.
     * @param os the output stream.
     * @param o the object.
     * @return os.
     */
    friend std::ostream& operator<<(std::ostream& os, MyVector const& o) {
        bool cont = false;
        os << "(";
        for (T const* t = o.begin(); t != o.end(); ++t) {
            if (cont) os << ",";
            os << *t;
            cont = true;
        }
        return os << ")";
    }
};

//template<typename T, typename Size>
//void swap(MyVector<T,Size>& v1, MyVector<T,Size>& v2) {
//    v1.swap(v2);
//}

//template<typename T, int dimension>
//struct MyMultiVector: public MyVector<MyMultiVector<T,dimension - 1> > {
//    MyMultiVector() {
//    }
//
//    MyMultiVector(Size n)
//            : MyVector<MyMultiVector<T,dimension - 1> >(n) {
//    }
//};
//
//template<typename T>
//struct MyMultiVector<T,1> : public MyVector<T> {
//    MyMultiVector() {
//    }
//
//    MyMultiVector(Size n)
//            : MyVector<T>(n) {
//    }
//};

}// namespace tdzdd
