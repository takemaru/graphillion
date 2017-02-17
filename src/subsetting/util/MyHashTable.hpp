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
#include <stdint.h>

namespace tdzdd {

class MyHashConstant {
protected:
    static int const MAX_FILL = 75;

public:
    static size_t primeSize(size_t n) {
        static unsigned long long primes[] = { //
                (1ULL << 3) + 3, (1ULL << 4) + 3, (1ULL << 5) + 5, //
                (1ULL << 6) + 3, (1ULL << 7) + 3, (1ULL << 8) + 7, //
                (1ULL << 9) + 9, (1ULL << 10) + 7, (1ULL << 11) + 5, //
                (1ULL << 12) + 3, (1ULL << 13) + 17, (1ULL << 14) + 27, //
                (1ULL << 15) + 3, (1ULL << 16) + 3, (1ULL << 17) + 29, //
                (1ULL << 18) + 3, (1ULL << 19) + 21, (1ULL << 20) + 7, //
                (1ULL << 21) + 17, (1ULL << 22) + 15, (1ULL << 23) + 9, //
                (1ULL << 24) + 43, (1ULL << 25) + 35, (1ULL << 26) + 15, //
                (1ULL << 27) + 29, (1ULL << 28) + 3, (1ULL << 29) + 11, //
                (1ULL << 30) + 3, (1ULL << 31) + 11, (1ULL << 32) + 15, //
                (1ULL << 33) + 17, (1ULL << 34) + 25, (1ULL << 35) + 53, //
                (1ULL << 36) + 31, (1ULL << 37) + 9, (1ULL << 38) + 7, //
                (1ULL << 39) + 23, (1ULL << 40) + 15};

        int lo = 0;
        int hi = sizeof(primes) / sizeof(primes[0]) - 1;

        if (n > primes[hi]) return n + 1;

        int i = (lo + hi) / 2;
        while (lo < hi) {
            if (n <= primes[i]) {
                hi = i;
            }
            else {
                lo = i + 1;
            }
            i = (lo + hi) / 2;
        }

        assert(i == lo && i == hi);
        return primes[i];
    }
};

template<typename T>
struct MyHashDefault {
    size_t operator()(T const& o) const {
        return o.hash();
    }

    bool operator()(T const& o1, T const& o2) const {
        return o1 == o2;
    }
};

template<typename T>
struct MyHashDefault<T*> {
    size_t operator()(T const* p) const {
        return p->hash();
    }

    bool operator()(T const* p1, T const* p2) const {
        return *p1 == *p2;
    }
};

template<typename T>
struct MyHashDefaultForInt {
    size_t operator()(T k) const {
        return k * 314159257ULL;
    }

    bool operator()(T k1, T k2) const {
        return k1 == k2;
    }
};

template<>
struct MyHashDefault<int8_t> : MyHashDefaultForInt<int8_t> {
};

template<>
struct MyHashDefault<int16_t> : MyHashDefaultForInt<int16_t> {
};

template<>
struct MyHashDefault<int32_t> : MyHashDefaultForInt<int32_t> {
};

template<>
struct MyHashDefault<int64_t> : MyHashDefaultForInt<int64_t> {
};

template<>
struct MyHashDefault<uint8_t> : MyHashDefaultForInt<uint8_t> {
};

template<>
struct MyHashDefault<uint16_t> : MyHashDefaultForInt<uint16_t> {
};

template<>
struct MyHashDefault<uint32_t> : MyHashDefaultForInt<uint32_t> {
};

template<>
struct MyHashDefault<uint64_t> : MyHashDefaultForInt<uint64_t> {
};

/**
 * Closed hash table implementation.
 * The default value @p T() cannot be added in the table.
 * @param T type of elements.
 */
template<typename T, typename Hash = MyHashDefault<T>,
        typename Equal = MyHashDefault<T> >
class MyHashTable: MyHashConstant {
protected:
    typedef T Entry;

    Hash const hashFunc;   ///< Functor for getting hash codes.
    Equal const eqFunc;    ///< Functor for checking equivalence.

    size_t tableCapacity_; ///< Size of the hash table storage.
    size_t tableSize_;     ///< Size of the hash table.
    size_t maxSize_;       ///< The maximum number of elements.
    size_t size_;          ///< The number of elements.
    Entry* table;          ///< Pointer to the storage.
    size_t collisions_;

public:
    /**
     * Default constructor.
     */
    MyHashTable(Hash const& hash = Hash(), Equal const& equal = Equal())
            : hashFunc(hash), eqFunc(equal), tableCapacity_(0), tableSize_(0),
              maxSize_(0), size_(0), table(0), collisions_(0) {
    }

    /**
     * Constructor.
     * @param n initial table size.
     * @param hash hash function.
     * @param equal equality function
     */
    MyHashTable(size_t n, Hash const& hash = Hash(), Equal const& equal =
            Equal())
            : hashFunc(hash), eqFunc(equal), tableCapacity_(0), tableSize_(0),
              maxSize_(0), size_(0), table(0), collisions_(0) {
        initialize(n);
    }

    /**
     * Copy constructor.
     * @param o object to be copied.
     * @param n lower bound of initial table size.
     */
    MyHashTable(MyHashTable const& o, size_t n = 1)
            : hashFunc(o.hashFunc), eqFunc(o.eqFunc), tableCapacity_(0),
              tableSize_(0), maxSize_(0), size_(0), table(0), collisions_(0) {
        initialize(std::max(o.tableSize_, n));
        for (const_iterator t = o.begin(); t != o.end(); ++t) {
            add(*t);
        }
    }

    MyHashTable& operator=(MyHashTable const& o) {
        initialize(o.tableSize_);
        for (const_iterator t = o.begin(); t != o.end(); ++t) {
            add(*t);
        }
        return *this;
    }

//    /**
//     * Move constructor.
//     * @param o object to be moved.
//     * @param n lower bound of initial table size.
//     */
//    MyHashTable(MyHashTable&& o, size_t n):
//    hashFunc(o.hashFunc), eqFunc(o.eqFunc), tableCapacity_(0),
//    tableSize_(0), maxSize_(0), size_(0), table(0), collisions_(0) {
//        initialize(std::max(o.tableSize_, n));
//        for (Entry const& e : o) {
//            add(std::move(e));
//        }
//        o.table = 0;
//        o.clear();
//    }
//
//    /**
//     * Move constructor.
//     * @param o object to be moved.
//     */
//    MyHashTable(MyHashTable&& o):
//    hashFunc(o.hashFunc), eqFunc(o.eqFunc),
//    tableCapacity_(o.tableCapacity_),
//    tableSize_(o.tableSize_), maxSize_(o.maxSize_),
//    size_(o.size_), table(o.table), collisions_(o.collisions_) {
//        o.table = 0;
//        o.clear();
//    }
//
//    MyHashTable& operator=(MyHashTable&& o) {
//        delete [] table;
//        tableCapacity_ = o.tableCapacity_;
//        tableSize_ = o.tableSize_;
//        maxSize_ = o.maxSize_;
//        size_ = o.size_;
//        table = o.table;
//        collisions_ = o.collisions_;
//        o.table = 0;
//        o.clear();
//        return *this;
//    }

    void moveAssign(MyHashTable& o) {
        delete[] table;
        tableCapacity_ = o.tableCapacity_;
        tableSize_ = o.tableSize_;
        maxSize_ = o.maxSize_;
        size_ = o.size_;
        table = o.table;
        collisions_ = o.collisions_;
        o.table = 0;
        o.clear();
    }

    virtual ~MyHashTable() {
        delete[] table;
    }

    size_t tableCapacity() const {
        return tableCapacity_ * sizeof(Entry);
    }

    size_t tableSize() const {
        return tableSize_;
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    size_t collisions() const {
        return collisions_;
    }

    /**
     * Initialize the table to be empty.
     * The memory is deallocated.
     */
    void clear() {
        delete[] table;
        tableCapacity_ = 0;
        tableSize_ = 0;
        maxSize_ = 0;
        size_ = 0;
        table = 0;
        collisions_ = 0;
    }

    /**
     * Initialize the table to be empty.
     * @param n initial table size.
     */
    void initialize(size_t n) {
        tableSize_ = primeSize(n * 100 / MAX_FILL + 1);
        maxSize_ = tableSize_ * MAX_FILL / 100;
        size_ = 0;
        collisions_ = 0;

        if (tableSize_ <= tableCapacity_) {
            for (size_t i = 0; i < tableSize_; ++i) {
                table[i] = Entry();
            }
        }
        else {
            tableCapacity_ = tableSize_;
            delete[] table;
            table = new Entry[tableCapacity_]();
        }
    }

    /**
     * Resize the storage appropriately.
     * @param n hint for the new table size.
     */
    void rehash(size_t n = 1) {
        MyHashTable tmp(std::max(tableSize_, n), hashFunc, eqFunc);
        for (iterator t = begin(); t != end(); ++t) {
            tmp.add(*t);
        }
        moveAssign(tmp);
    }

    /**
     * Insert an element if no other equivalent element is registered.
     * @param elem the element to be inserted.
     * @return reference to the element in the table.
     */
    Entry& add(Entry const& elem) {
        assert(!(elem == Entry()));
        if (tableSize_ == 0) rehash();
        size_t i;

        while (1) {
            i = hashFunc(elem) % tableSize_;

            while (!(table[i] == Entry())) {
                if (eqFunc(table[i], elem)) return table[i];
                ++collisions_;
                ++i;
                if (i >= tableSize_) i = 0;
            }

            if (size_ < maxSize_) break;

            /* Rehash only when new element is inserted. */
            rehash(size_ * 2);
        }

        ++size_;
        table[i] = elem;
        return table[i];
    }

    /**
     * Get the element that is already registered.
     * @param elem the element to be searched.
     * @return pointer to the element in the table or null.
     */
    Entry* get(Entry const& elem) const {
        assert(!(elem == Entry()));

        if (tableSize_ > 0) {
            size_t i = hashFunc(elem) % tableSize_;
            while (!(table[i] == Entry())) {
                if (eqFunc(table[i], elem)) return &table[i];
                ++i;
                if (i >= tableSize_) i = 0;
            }
        }

        return static_cast<Entry*>(0);
    }

    class iterator {
        Entry* ptr;
        Entry const* end;

    public:
        explicit iterator(Entry* from, Entry const* to)
                : ptr(from), end(to) {
            while (ptr < end && *ptr == Entry()) {
                ++ptr;
            }
        }

        Entry& operator*() {
            return *ptr;
        }

        Entry* operator->() {
            return ptr;
        }

        iterator& operator++() {
            while (++ptr < end) {
                if (!(*ptr == Entry())) break;
            }
            return *this;
        }

        bool operator==(iterator const& o) const {
            return ptr == o.ptr;
        }

        bool operator!=(iterator const& o) const {
            return ptr != o.ptr;
        }
    };

    class const_iterator {
        Entry const* ptr;
        Entry const* end;

    public:
        explicit const_iterator(Entry const* from, Entry const* to)
                : ptr(from), end(to) {
            while (ptr < end && *ptr == Entry()) {
                ++ptr;
            }
        }

        Entry const& operator*() const {
            return *ptr;
        }

        Entry const* operator->() const {
            return ptr;
        }

        const_iterator& operator++() {
            while (++ptr < end) {
                if (!(*ptr == Entry())) break;
            }
            return *this;
        }

        bool operator==(const_iterator const& o) const {
            return ptr == o.ptr;
        }

        bool operator!=(const_iterator const& o) const {
            return ptr != o.ptr;
        }
    };

    iterator begin() {
        return iterator(table, table + tableSize_);
    }

    const_iterator begin() const {
        return const_iterator(table, table + tableSize_);
    }

    iterator end() {
        return iterator(table + tableSize_, table + tableSize_);
    }

    const_iterator end() const {
        return const_iterator(table + tableSize_, table + tableSize_);
    }
};

/**
 * Entry for a hash map.
 * @param K key of the map.
 * @param V value of the map.
 */
template<typename K, typename V>
struct MyHashMapEntry {
    typedef K Key;
    typedef V Value;

    K key; ///< The key.
    V value; ///< The value.

    MyHashMapEntry()
            : key(), value() {
    }

    MyHashMapEntry(K const& key)
            : key(key), value() {
    }

    MyHashMapEntry(K const& key, V const& value)
            : key(key), value(value) {
    }

    /**
     * Check key's equivalence between another object.
     * @param o another object.
     * @return true if equivalent.
     */
    bool operator==(MyHashMapEntry const& o) const {
        return key == o.key;
    }

    /**
     * Check the order of keys with another object.
     * @param o another object.
     * @return true if this is less than the other.
     */
    bool operator<(MyHashMapEntry const& o) const {
        return key < o.key;
    }

    /**
     * Print the object.
     * @param os the output stream.
     * @param o the object.
     * @return os.
     */
    friend std::ostream& operator<<(std::ostream& os, MyHashMapEntry const& o) {
        return os << "(" << o.key << " => " << o.value << ")";
    }
}
;

template<typename K, typename V, typename Hash, typename Equal>
class MyHashMapHashWrapper {
    Hash const hashFunc;
    Equal const eqFunc;

public:
    MyHashMapHashWrapper(Hash const& hash, Equal const& equal)
            : hashFunc(hash), eqFunc(equal) {
    }

    size_t operator()(MyHashMapEntry<K,V> const& o) const {
        return hashFunc(o.key);
    }

    bool operator()(MyHashMapEntry<K,V> const& o1,
            MyHashMapEntry<K,V> const& o2) const {
        return eqFunc(o1.key, o2.key);
    }
};

/**
 * Closed hash map implementation.
 * An entry for the default key @p K() cannot be added in the map.
 * @param K type of keys.
 * @param V type of values.
 */
template<typename K, typename V, typename Hash = MyHashDefault<K>,
        typename Equal = MyHashDefault<K> >
struct MyHashMap: public MyHashTable<MyHashMapEntry<K,V>,
        MyHashMapHashWrapper<K,V,Hash,Equal>,
        MyHashMapHashWrapper<K,V,Hash,Equal> > {
    typedef MyHashMapEntry<K,V> Entry;
    typedef MyHashTable<Entry,MyHashMapHashWrapper<K,V,Hash,Equal>,
            MyHashMapHashWrapper<K,V,Hash,Equal> > Table;

    /**
     * Default constructor.
     */
    MyHashMap(Hash const& hash = Hash(), Equal const& equal = Equal())
            : Table(MyHashMapHashWrapper<K,V,Hash,Equal>(hash, equal),
                    MyHashMapHashWrapper<K,V,Hash,Equal>(hash, equal)) {
    }

    /**
     * Constructor.
     * @param n initial table size.
     * @param hash hash function.
     * @param equal equality function.
     */
    MyHashMap(size_t n, Hash const& hash = Hash(), Equal const& equal = Equal())
            : Table(n, MyHashMapHashWrapper<K,V,Hash,Equal>(hash, equal),
                    MyHashMapHashWrapper<K,V,Hash,Equal>(hash, equal)) {
    }

    /**
     * Copy constructor.
     * @param o the object to be copied.
     * @param n lower bound of initial table size.
     */
    MyHashMap(MyHashMap const& o, size_t n = 1)
            : Table(o, n) {
    }

//    /**
//     * Move constructor.
//     * @param o the object to be moved.
//     * @param n lower bound of initial table size.
//     */
//    MyHashMap(MyHashMap&& o, size_t n = 1): Table(std::move(o), n) {
//    }

    /**
     * Insert an element if no other equivalent key is registered.
     * @param key key of the element to be inserted.
     * @return reference to the value in the table.
     */
    V& operator[](K const& key) {
        return Table::add(Entry(key)).value;
    }

    /**
     * Get the value that is already registered.
     * @param key the key to be searched.
     * @return pointer to the value in the map or null.
     */
    V* getValue(K const& key) const {
        Entry* p = Table::get(Entry(key));
        return (p != 0) ? &p->value : 0;
    }
};

} // namespace tdzdd
