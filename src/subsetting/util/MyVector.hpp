/*
 * Top-Down BDD/ZDD Package
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: MyVector.hpp 410 2013-02-14 06:33:04Z iwashita $
 */

#pragma once

#include <cassert>
#include <cstring>

template<typename T>
class MyVector {
    size_t capacity_;  ///< Size of the array.
    size_t size_;      ///< Current number of elements.
    T* array_;         ///< Start address of the array.

    static T* allocate(size_t n) {
        return std::allocator<T>().allocate(n);
    }

    static void deallocate(T* p, size_t n) {
        std::allocator<T>().deallocate(p, n);
    }

    void ensureCapacity(size_t capacity) {
        if (capacity_ < capacity) {
            reserve(std::max(size_t(16), capacity * 4));
        }
    }

public:
    MyVector()
            : capacity_(0), size_(0), array_(0) {
    }

    MyVector(size_t n)
            : capacity_(0), size_(0), array_(0) {
        resize(n);
    }

//    template<typename ... Args>
//    MyVector(size_t n, Args const&... args)
//            : capacity_(0), size_(0), array_(0) {
//        resize(n, args...);
//    }

//    MyVector(size_t size, T&& val)
//    : capacity_(0), size_(0), array_(0) {
//        resize(size, std::move(val));
//    }

    MyVector(MyVector const& o)
            : capacity_(o.size_), size_(o.size_), array_(allocate(capacity_)) {
        for (size_t i = 0; i < size_; ++i) {
            new (array_ + i) T(o[i]);
        }
    }

    MyVector& operator=(MyVector const& o) {
        resize(0);
        reserve(o.size_);
        size_ = o.size_;
        for (size_t i = 0; i < size_; ++i) {
            new (array_ + i) T(o[i]);
        }
        return *this;
    }

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

    ~MyVector() {
        clear();
    }

    /**
     * Get the number of elements.
     * @return the number of elements.
     */
    size_t size() const {
        return size_;
    }

    /**
     * Check emptiness.
     * @return true if empty.
     */
    bool empty() const {
        return size_ == 0;
    }

    /**
     * Reserve the memory.
     * Data is moved if the actual capacity is extended.
     * @param capacity lower bound of the capacity.
     */
    void reserve(size_t capacity) {
        if (capacity_ < capacity) {
            T* tmp = allocate(capacity);
            for (size_t i = 0; i < size_; ++i) {
//                new (tmp + i) T(std::move(array_[i]));
                new (tmp + i) T(array_[i]);
                array_[i].~T();
            }
            deallocate(array_, capacity_);
            array_ = tmp;
            capacity_ = capacity;
        }
    }

    /**
     * Resize the array.
     * Data is moved if the new size exceeds the capacity.
     * @param n new size.
     */
    void resize(size_t n) {
        if (size_ < n) {
            reserve(n);
            for (T* p = array_ + size_; p < array_ + n; ++p) {
                new (p) T();
            }
            size_ = n;
        }
        else {
            while (n < size_) {
                array_[--size_].~T();
            }
        }
    }

//    /**
//     * Resize the array.
//     * Data is moved if the new size exceeds the capacity.
//     * @param n new size.
//     * @param args initializer for new elements.
//     */
//    template<typename ... Args>
//    void resize(size_t n, Args const&... args) {
//        if (size_ < n) {
//            reserve(n);
//            for (T* p = array_ + size_; p < array_ + n; ++p) {
//                new (p) T(args...);
//            }
//            size_ = n;
//        }
//        else {
//            while (n < size_) {
//                array_[--size_].~T();
//            }
//        }
//    }

    /**
     * Shrink the array.
     * @param n new size.
     */
    void shrink(size_t n) {
        while (n < size_) {
            array_[--size_].~T();
        }

        T* tmp = allocate(size_);
        std::memcpy(tmp, array_, size_ * sizeof(T));
        deallocate(array_, capacity_);
        array_ = tmp;
        capacity_ = size_;
    }

    /**
     * Initialize the array to be empty.
     * The memory is deallocated.
     */
    void clear() {
        shrink(0);
        deallocate(array_, capacity_);
        array_ = 0;
        capacity_ = 0;
    }

    /**
     * Add an element to the end of the array.
     * The array is automatically extended,
     * which may cause data move.
     * @param val value of the new element.
     */
    void push_back(T const& val) {
        ensureCapacity(size_ + 1);
        new (array_ + size_) T(val);
        ++size_;
    }

//    /**
//     * Add an element constructed in place to the end of the array.
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
     * Access to the last element.
     * @return reference to the last element.
     */
    T& back() {
        assert(size_ >= 1);
        return array_[size_ - 1];
    }

    /**
     * Access to the i-th element.
     * @param i index of the element.
     * @return reference to the element.
     */
    T& operator[](size_t i) {
        assert(i < size_);
        return array_[i];
    }

    /**
     * Access to the i-th element.
     * @param i index of the element.
     * @return reference to the element.
     */
    T const& operator[](size_t i) const {
        assert(i < size_);
        return array_[i];
    }

    /**
     * Get a pointer to the first element.
     * @return pointer to the first element.
     */
    T* begin() const {
        return array_;
    }

    /**
     * Get a pointer to the end of elements.
     * @return pointer to the end of elements.
     */
    T* end() const {
        return array_ + size_;
    }

    /**
     * Get the hash code of this object.
     * @return the hash code.
     */
    size_t hash() const {
        size_t h = size_;
        for (size_t i = 0; i < size_; ++i) {
            h = h * 31 + array_[i].hash();
        }
        return h;
    }

    /**
     * Check equivalence between another object.
     * @param o another object.
     * @return true if equivalent.
     */
    bool operator==(MyVector const& o) const {
        if (size_ != o.size_) return false;
        for (size_t i = 0; i < size_; ++i) {
            if (!(array_[i] == o.array_[i])) return false;
        }
        return true;
    }

    /**
     * Send an object to an output stream.
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
