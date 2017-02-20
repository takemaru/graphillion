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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdint.h>

#include "MemoryPool.hpp"
#include "MyHashTable.hpp"

namespace tdzdd {

template<size_t N>
class MyBitSet {
    static size_t const offset = (N == 0) ? 1 : 0;

    uint64_t array_[(N == 0) ? 1 : N];

    uint64_t& word(size_t i) {
        assert(i < numWords());
        return array_[i + offset];
    }

    uint64_t word(size_t i) const {
        assert(i < numWords());
        return array_[i + offset];
    }

    uint64_t mask(size_t i) const {
        return uint64_t(1) << i;
    }

public:
    MyBitSet(uint64_t n = 0) {
        array_[0] = n;
        for (size_t i = (N == 0) ? 0 : 1; i < numWords(); ++i) {
            word(i) = 0;
        }
    }

    /**
     * Gets the number of words for a bit vector.
     * @return the number of words.
     */
    size_t numWords() const {
        return (N == 0) ? array_[0] : N;
    }

    /**
     * Initializes the set to be empty.
     * The memory is deallocated.
     */
    void clear() {
        for (size_t i = 0; i < numWords(); ++i) {
            word(i) = 0;
        }
    }

    /**
     * Checks if the set is empty.
     * @return true if empty.
     */
    bool empty() const {
        for (size_t i = 0; i < numWords(); ++i) {
            if (word(i) != 0) return false;
        }
        return true;
    }

    /**
     * Adds the i-th element.
     * @param i element number.
     */
    void add(size_t i) {
        uint64_t& w = word(i / 64);
        uint64_t m = mask(i % 64);
        w |= m;
    }

    /**
     * Removes the i-th element.
     * @param i element number.
     */
    void remove(size_t i) {
        uint64_t& w = word(i / 64);
        uint64_t m = mask(i % 64);
        w &= ~m;
    }

    /**
     * Removes the i-th entry and
     * shifts all behind it front by one.
     * @param i element number.
     */
    void pullout(size_t i) {
        size_t k = i / 64;
        uint64_t* p = &word(k);
        uint64_t* q = p + numWords();
        uint64_t m = mask(i % 64 + 1) - 1;
        *p = (*p & ~m) | ((*p << 1) & m);
        while (++p < q) {
            *(p - 1) |= *p >> 63;
            *p <<= 1;
        }
    }

    /**
     * Checks if the i-th element is included.
     * @param i element number.
     * @return true if the i-th element is included.
     */
    bool includes(size_t i) const {
        uint64_t w = word(i / 64);
        uint64_t m = mask(i % 64);
        return w & m;
    }

    /**
     * Checks non-emptiness of the intersection of two bit-vectors.
     * @param o the other bit-vector.
     * @return true if the intersection is not empty.
     */
    bool intersects(MyBitSet const& o) const {
        size_t n = std::min(numWords(), o.numWords());
        for (size_t i = 0; i < n; ++i) {
            if (word(i) & o.word(i)) return true;
        }
        return false;
    }

    class const_iterator {
        MyBitSet const& bitSet;
        size_t wordPos;
        int bitPos;
        uint64_t word;

    public:
        const_iterator(MyBitSet const& bitSet) :
                bitSet(bitSet), wordPos(bitSet.numWords()), bitPos(-1), word(0) {
        }

        const_iterator(MyBitSet const& bitSet, size_t i) :
                bitSet(bitSet), wordPos(i), bitPos(-1), word(bitSet.word(i)) {
            ++(*this);
        }

        size_t operator*() const {
            return wordPos * 64 + bitPos;
        }

        const_iterator& operator++() {
            while (word == 0) {
                bitPos = -1;
                if (++wordPos >= bitSet.numWords()) {
                    wordPos = bitSet.numWords();
                    return *this;
                }
                word = bitSet.word(wordPos);
            }

            if (word == uint64_t(1) << 63) {
                bitPos = 63;
                word = 0;
            }
            else {
                int k = __builtin_ffsll(word);
                bitPos += k;
                word >>= k;
            }

//            int k = __builtin_ffsll(word);
//            bitPos += k;
//            word = (k < 64) ? word >> k : 0;

//            while ((word & 1) == 0) {
//                word >>= 1;
//                ++bitPos;
//            }
//            word >>= 1;
//            ++bitPos;

            return *this;
        }

        bool operator==(const_iterator const& o) const {
            return wordPos == o.wordPos && bitPos == o.bitPos;
        }

        bool operator!=(const_iterator const& o) const {
            return !(*this == o);
        }
    };

    /**
     * Returns an iterator to the beginning.
     * @return iterator to the beginning.
     */
    const_iterator begin() const {
        return (numWords() == 0) ? end() : const_iterator(*this, 0);
    }

    /**
     * Returns an iterator to the end.
     * @return iterator to the end.
     */
    const_iterator end() const {
        return const_iterator(*this);
    }

    /**
     * Gets the hash code of this object.
     * @return the hash code.
     */
    size_t hash() const {
        size_t n = numWords();
        size_t h = 0;
        for (size_t i = 0; i < n; ++i) {
            h = (h + word(i)) * 314159257;
        }
        return h;
    }

    /**
     * Checks equivalence between another object.
     * @param o another object.
     * @return true if equivalent.
     */
    bool operator==(MyBitSet const& o) const {
        size_t n = numWords();
        if (o.numWords() != n) return false;
        for (size_t i = 0; i < n; ++i) {
            if (word(i) != o.word(i)) return false;
        }
        return true;
    }

    /**
     * Sends an object to an output stream.
     * @param os the output stream.
     * @param o the object.
     * @return os.
     */
    friend std::ostream& operator<<(std::ostream& os, MyBitSet const& o) {
        bool comma = false;
        os << '{';
        for (const_iterator t = o.begin(); t != o.end(); ++t) {
            if (comma) os << ',';
            os << *t;
            comma = true;
        }
        return os << '}';
    }
};

class MyBitSetOnPool: public MyBitSet<0> {
    MyBitSetOnPool(size_t n) :
            MyBitSet<0>(n) {
    }

public:
    static MyBitSetOnPool* newInstance(MemoryPool& pool, size_t bits) {
        size_t n = (bits + 63) / 64;
        return new (pool.template allocate<uint64_t>(1 + n)) MyBitSetOnPool(n);
    }
};

template<typename T, size_t N>
class MySmallSet {
    size_t size_;
    T array_[N == 0 ? 1 : N];

public:
    MySmallSet() :
            size_(0) {
    }

    /**
     * Gets the number of elements.
     * @return the number of elements.
     */
    size_t size() const {
        return size_;
    }

    /**
     * Initializes the set to be empty.
     */
    void clear() {
        size_ = 0;
    }

    /**
     * Checks if the set is empty.
     * @return true if empty.
     */
    bool empty() const {
        return size_ == 0;
    }

    /**
     * Adds an element.
     * @param e element.
     */
//    void add(T const& e) {
//        T* p = std::lower_bound(array_, array_ + size_, e);
//        if (p != array_ + size_ && *p == e) return;
//        if (N > 0 && size_ >= N - 1) throw std::out_of_range("MySmallSet::add");
//        for (T* q = array_ + size_; q != p; --q) {
//            *q = *(q - 1);
//        }
//        *p = e;
//        ++size_;
//    }
    void add(T const& e) {
        if (N > 0 && size_ >= N - 1) throw std::out_of_range("MySmallSet::add");
        T* pz = array_ + size_;
        T* p = pz - 1;
        while (array_ <= p && e <= *p) {
            if (e == *p) {
                while (++p < pz) {
                    *p = *(p + 1);
                }
                return;
            }
            *(p + 1) = *p;
            --p;
        }
        *(p + 1) = e;
        ++size_;
    }

    /**
     * Removes an element.
     * @param e element.
     */
    void remove(T const& e) {
        T* p = std::lower_bound(array_, array_ + size_, e);
        if (p == array_ + size_ || *p != e) return;
        --size_;
        for (T* q = p; q != array_ + size_; ++q) {
            *q = *(q + 1);
        }
    }

    /**
     * Checks if the given element is included.
     * @param e element.
     * @return true if the element is included.
     */
    bool includes(T const& e) const {
        return std::binary_search(array_, array_ + size_, e);
    }

    /**
     * Checks equality between another container.
     * The container must be sorted.
     * @param c sorted container.
     * @return true if they have the same set of elements.
     */
    template<typename C>
    bool equals(C const& c) const {
        const_iterator p = begin();
        typename C::const_iterator q = c.begin();
        const_iterator pz = end();
        typename C::const_iterator qz = c.end();
        while (p != pz && q != qz) {
            if (*p != *q) return false;
            ++p, ++q;
        }
        return p == pz && q == qz;
    }

    /**
     * Checks non-emptiness of the intersection between another container.
     * The container must be sorted.
     * @param c sorted container.
     * @return true if the intersection is not empty.
     */
    template<typename C>
    bool intersects(C const& c) const {
        const_iterator p = begin();
        typename C::const_iterator q = c.begin();
        const_iterator pz = end();
        typename C::const_iterator qz = c.end();
        while (p != pz && q != qz) {
            if (*p == *q) return true;
            if (*p < *q) ++p;
            else ++q;
        }
        return false;
    }

    /**
     * Checks if the set contains all elements in another container.
     * The container must be sorted.
     * @param c sorted container.
     * @return true if all elements are contained in this set.
     */
    template<typename C>
    bool containsAll(C const& c) const {
        const_iterator p = begin();
        typename C::const_iterator q = c.begin();
        const_iterator pz = end();
        typename C::const_iterator qz = c.end();
        while (p != pz && q != qz) {
            if (*p > *q) return false;
            if (*p < *q) ++p;
            else ++p, ++q;
        }
        return q == qz;
    }

    typedef T const* const_iterator;

    /**
     * Returns an iterator to the beginning.
     * @return iterator to the beginning.
     */
    const_iterator begin() const {
        return array_;
    }

    /**
     * Returns an iterator to the end.
     * @return iterator to the end.
     */
    const_iterator end() const {
        return array_ + size_;
    }

    class const_reverse_iterator {
        T const* ptr;

    public:
        const_reverse_iterator(T const* ptr) :
                ptr(ptr) {
        }

        T const& operator*() const {
            return *ptr;
        }

        T const* operator->() const {
            return ptr;
        }

        const_reverse_iterator& operator++() {
            --ptr;
            return *this;
        }

        bool operator==(const_reverse_iterator const& o) const {
            return ptr == o.ptr;
        }

        bool operator!=(const_reverse_iterator const& o) const {
            return ptr != o.ptr;
        }
    };

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
    const_reverse_iterator rend() const {
        return const_reverse_iterator(array_ - 1);
    }

    /**
     * Gets the i-th element.
     * @param i element number.
     * @return the i-th element.
     */
    T const& get(size_t i) const {
        assert(N == 0 || i < N);
        return array_[i];
    }

    /**
     * Gets the hash code of this object.
     * @return the hash code.
     */
    size_t hash() const {
        size_t h = 0;
        for (size_t k = 0; k < size_; ++k) {
            h = h * 271828171 + MyHashDefault<T>()(array_[k]);
        }
        return h;
    }

    /**
     * Checks equivalence between another object.
     * @param o another object.
     * @return true if equivalent.
     */
    bool operator==(MySmallSet const& o) const {
        if (size_ != o.size_) return false;
        for (size_t k = 0; k < size_; ++k) {
            if (array_[k] != o.array_[k]) return false;
        }
        return true;
    }

    /**
     * Sends an object to an output stream.
     * @param os the output stream.
     * @param o the object.
     * @return os.
     */
    friend std::ostream& operator<<(std::ostream& os, MySmallSet const& o) {
        bool comma = false;
        os << '{';
        for (size_t k = 0; k < o.size_; ++k) {
            if (comma) os << ',';
            os << o.array_[k];
            comma = true;
        }
        return os << '}';
    }
};

template<typename T>
class MySmallSetOnPool: public MySmallSet<T,0> {
public:
    static MySmallSetOnPool* newInstance(MemoryPool& pool, size_t n) {
        size_t m = (sizeof(MySmallSet<T,0> ) - sizeof(T)
                    + sizeof(T) * n
                    + sizeof(size_t)
                    - 1)
                   / sizeof(size_t);
        return new (pool.template allocate<size_t>(m)) MySmallSetOnPool();
    }

    static MySmallSetOnPool* newInstance(MemoryPool& pool, int n) {
        return newInstance(pool, size_t(n));
    }

    template<typename C>
    static MySmallSetOnPool* newInstance(MemoryPool& pool, C const& copy) {
        MySmallSetOnPool* obj = newInstance(pool, copy.size());
        for (typename C::const_iterator t = copy.begin(); t != copy.end();
                ++t) {
            obj->add(*t);
        }
        return obj;
    }
};

} // namespace tdzdd
