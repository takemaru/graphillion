/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: MyArray.hpp 351 2012-11-15 11:20:21Z iwashita $
 */

#pragma once

#include <cassert>
#include <ostream>

template<typename T, int N>
class MyArray {
    static size_t const numData = N;
    static size_t const numWords = (N * sizeof(T) - 1) / sizeof(uint64_t) + 1;

    union {
        T data[numData];
        uint64_t word[numWords];
    };

    T* init(T* p) {
        return p;
    }

    template<typename ... Args>
    T* init(T* p, T const& val, Args ... rest) {
        *p++ = val;
        return init(p, rest...);
    }

public:
    MyArray() {
        for (size_t i = 0; i < numWords; ++i) {
            word[i] = 0;
        }
    }

    template<typename ... Args>
    MyArray(T const& val, Args ... rest) {
        T* p = init(data, val, rest...);
        T const& v = *(p - 1);
        while (p < data + numData) {
            *p++ = v;
        }
    }

    T& operator[](size_t i) {
        assert(0 <= i && i < numData);
        return data[i];
    }

    T const& operator[](size_t i) const {
        assert(0 <= i && i < numData);
        return data[i];
    }

    size_t hash() const {
        size_t h = word[0];
        for (size_t i = 1; i < numWords; ++i) {
            h = h * 31 + word[i];
        }
        return h;
    }

    bool operator==(MyArray const& o) const {
        for (size_t i = 0; i < numWords; ++i) {
            if (word[i] != o.word[i]) return false;
        }
        return true;
    }

    friend std::ostream& operator<<(std::ostream& os, MyArray const& o) {
        bool b = false;
        os << "[";
        for (size_t i = 0; i < numData; ++i) {
            if (b) os << ",";
            os << o.data[i];
            b = true;
        }
        return os << "]";
    }
};
