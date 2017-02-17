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
#include <iostream>
#include <stdexcept>

#include "MyVector.hpp"

namespace tdzdd {

/**
 * Memory pool.
 * Allocated memory blocks are kept until the pool is destructed.
 */
class MemoryPool {
    struct Unit {
        Unit* next;
    };

    static size_t const UNIT_SIZE = sizeof(Unit);
    static size_t const BLOCK_UNITS = 400000 / UNIT_SIZE;
    static size_t const MAX_ELEMENT_UNIS = BLOCK_UNITS / 10;

    Unit* blockList;
    size_t nextUnit;

public:
    MemoryPool()
            : blockList(0), nextUnit(BLOCK_UNITS) {
    }

    MemoryPool(MemoryPool const& o)
            : blockList(0), nextUnit(BLOCK_UNITS) {
//        if (o.blockList != 0) throw std::runtime_error(
//                "MemoryPool can't be copied unless it is empty!"); //FIXME
    }

//    MemoryPool& operator=(MemoryPool const& o) {
//        if (o.blockList != 0) throw std::runtime_error(
//                "MemoryPool can't be copied unless it is empty!"); //FIXME
//        clear();
//        return *this;
//    }

//    MemoryPool(MemoryPool&& o): blockList(o.blockList), nextUnit(o.nextUnit) {
//        o.blockList = 0;
//    }
//
//    MemoryPool& operator=(MemoryPool&& o) {
//        blockList = o.blockList;
//        nextUnit = o.nextUnit;
//        o.blockList = 0;
//        return *this;
//    }

    void moveFrom(MemoryPool& o) {
        blockList = o.blockList;
        nextUnit = o.nextUnit;
        o.blockList = 0;
    }

    virtual ~MemoryPool() {
        clear();
    }

    bool empty() const {
        return blockList == 0;
    }

    void clear() {
        while (blockList != 0) {
            Unit* block = blockList;
            blockList = blockList->next;
            delete[] block;
        }
        nextUnit = BLOCK_UNITS;
    }

    void reuse() {
        if (blockList == 0) return;
        while (blockList->next != 0) {
            Unit* block = blockList;
            blockList = blockList->next;
            delete[] block;
        }
        nextUnit = 1;
    }

    void splice(MemoryPool& o) {
        if (blockList != 0) {
            Unit** rear = &o.blockList;
            while (*rear != 0) {
                rear = &(*rear)->next;
            }
            *rear = blockList;
        }

        blockList = o.blockList;
        nextUnit = o.nextUnit;

        o.blockList = 0;
        o.nextUnit = BLOCK_UNITS;
    }

    void* alloc(size_t n) {
        size_t const elementUnits = (n + UNIT_SIZE - 1) / UNIT_SIZE;

        if (elementUnits > MAX_ELEMENT_UNIS) {
            size_t m = elementUnits + 1;
            Unit* block = new Unit[m];
            if (blockList == 0) {
                block->next = 0;
                blockList = block;
            }
            else {
                block->next = blockList->next;
                blockList->next = block;
            }
            return block + 1;
        }

        if (nextUnit + elementUnits > BLOCK_UNITS) {
            Unit* block = new Unit[BLOCK_UNITS];
            block->next = blockList;
            blockList = block;
            nextUnit = 1;
            assert(nextUnit + elementUnits <= BLOCK_UNITS);
        }

        Unit* p = blockList + nextUnit;
        nextUnit += elementUnits;
        return p;
    }

    template<typename T>
    T* allocate(size_t n = 1) {
        return static_cast<T*>(alloc(sizeof(T) * n));
    }

    template<typename T>
    class Allocator: public std::allocator<T> {
    public:
        MemoryPool* pool;

        template<typename U>
        struct rebind {
            typedef Allocator<U> other;
        };

        Allocator() throw ()
                : pool(0) {
        }

        Allocator(MemoryPool& pool) throw ()
                : pool(&pool) {
        }

        Allocator(Allocator const& o) throw ()
                : pool(o.pool) {
        }

        Allocator& operator=(Allocator const& o) {
            pool = o.pool;
            return *this;
        }

        template<typename U>
        Allocator(Allocator<U> const& o) throw ()
                : pool(o.pool) {
        }

        ~Allocator() throw () {
        }

        T* allocate(size_t n, const void* = 0) {
            return pool->allocate<T>(n);
        }

        void deallocate(T*, size_t) {
        }
    };

    template<typename T>
    Allocator<T> allocator() {
        return Allocator<T>(*this);
    }

    /**
     * Send an object to an output stream.
     * @param os the output stream.
     * @param o the object.
     * @return os.
     */
    friend std::ostream& operator<<(std::ostream& os, MemoryPool const& o) {
        int n = 0;
        for (Unit* p = o.blockList; p != 0; p = p->next) {
            ++n;
        }
        return os << "MemoryPool(" << n << ")";
    }
};

/**
 * Collection of memory pools.
 */
typedef MyVector<MemoryPool> MemoryPools;

template<>
inline void MyVector<MemoryPool>::moveElement(MemoryPool& from,
        MemoryPool& to) {
    to.moveFrom(from);
}

} // namespace tdzdd
