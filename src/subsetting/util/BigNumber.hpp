/*
 * Top-Down BDD/ZDD Package
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: BigNumber.hpp 410 2013-02-14 06:33:04Z iwashita $
 */

#pragma once

#include <stdint.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

class BigNumber {
protected:
    static uint64_t const MSB = uint64_t(1) << 63;

    uint64_t* array;

public:
    BigNumber()
            : array(0) {
    }

    explicit BigNumber(uint64_t* array)
            : array(array) {
    }

    void setArray(uint64_t* array) {
        this->array = array;
    }

    int size() const {
        uint64_t const* p = array;
        if (p == 0) return 1;
        while (*p++ & MSB)
            ;
        return p - array;
    }

    BigNumber& store(BigNumber const& o) {
        if (o.array == 0) return store(0);

        uint64_t* p = array;
        uint64_t const* q = o.array;
        do {
            *p++ = *q;
        } while (*q++ & MSB);
        return *this;
    }

    BigNumber& store(uint64_t n) {
        if (array == 0) {
            if (n != 0) throw std::runtime_error(
                    "Non-zero assignment to null BigNumberWriter");
        }
        else {
            array[0] = n;
            if (n & MSB) array[1] = 1;
        }
        return *this;
    }

    bool equals(BigNumber const& o) const {
        if (array == 0) return o.equals(0);
        if (o.array == 0) return equals(0);

        uint64_t* p = array;
        uint64_t const* q = o.array;
        do {
            if (*p++ != *q) return false;
        } while (*q++ & MSB);
        return true;
    }

    bool equals(uint64_t n) const {
        return (array == 0) ? n == 0 :
                array[0] == n && ((n & MSB) == 0 || array[1] == 1);
    }

    size_t add(BigNumber const& o) {
        uint64_t* p = array;
        uint64_t const* q = o.array;
        uint64_t x = 0;

        while (true) {
            x += *p & ~MSB;
            x += *q & ~MSB;

            if ((*p & MSB) == 0) {
                while ((*q & MSB) != 0) {
                    *p++ = x | MSB;
                    ++q;
                    x >>= 63;
                    x += *q & ~MSB;
                }

                break;
            }

            if ((*q & MSB) == 0) {
                while ((*p & MSB) != 0) {
                    *p++ = x | MSB;
                    x >>= 63;
                    x += *p & ~MSB;
                }

                break;
            }

            *p++ = x | MSB;
            ++q;
            x >>= 63;
        }

        *p++ = x;
        if (x & MSB) *p++ = 1;

        return p - array;
    }

    uint32_t divide(uint32_t n) {
        uint64_t* p = array;
        if (p == 0) return 0;

        while (*p++ & MSB)
            ;
        uint64_t r = 0;
        bool cont = false;

        do {
            --p;
            uint64_t q = cont ? MSB : 0;
            r = (r << 31) | ((*p & ~MSB) >> 32);
            lldiv_t d = lldiv(r, 10LL);
            q += d.quot << 32;
            r = (d.rem << 32) | (*p & ((uint64_t(1) << 32) - 1));
            d = lldiv(r, 10LL);
            q += d.quot;
            r = d.rem;
            *p = q;
            if (q != 0) cont = true;
        } while (p != array);

        return r;
    }

    template<typename T>
    T translate() const {
        uint64_t const* p = array;
        while (*p & MSB) {
            ++p;
        }
        T v = *p;
        while (p != array) {
            v <<= 63;
            v += *--p & ~MSB;
        }
        return v;
    }

private:
    void printHelper(std::ostream& os) {
        uint32_t r = divide(10);
        if (!equals(0)) printHelper(os);
        os << r;
    }

public:
    friend std::ostream& operator<<(std::ostream& os, BigNumber const& o) {
        uint64_t storage[o.size()];
        BigNumber n(storage);
        n.store(o);
        n.printHelper(os);
        return os;
    }

    operator std::string() const {
        std::ostringstream ss;
        ss << *this;
        return ss.str();
    }
};

template<int size>
class FixedBigNumber {
    uint32_t val[size];

public:
    FixedBigNumber() {
        for (int i = 0; i < size; ++i) {
            val[i] = 0;
        }
    }

    FixedBigNumber(uint32_t i) {
        val[0] = i;
        for (int i = 1; i < size; ++i) {
            val[i] = 0;
        }
    }

    FixedBigNumber& operator=(uint32_t n) {
        val[0] = n;
        for (int i = 1; i < size; ++i) {
            val[i] = 0;
        }
        return *this;
    }

    bool operator==(FixedBigNumber const& o) const {
        for (int i = 0; i < size; ++i) {
            if (val[i] != o.val[i]) return false;
        }
        return true;
    }

    bool operator!=(FixedBigNumber const& o) const {
        return !operator==(o);
    }

    bool operator==(uint32_t n) const {
        if (val[0] != n) return false;
        for (int i = 1; i < size; ++i) {
            if (val[i] != 0) return false;
        }
        return true;
    }

    bool operator!=(uint32_t n) const {
        return !operator==(n);
    }

    void operator+=(FixedBigNumber const& o) {
        uint64_t x = 0;
        for (int i = 0; i < size; ++i) {
            x += val[i];
            x += o.val[i];
            val[i] = x;
            x >>= 32;
        }
        if (x != 0) throw std::runtime_error("FixedBigNumber overflow!");
    }

    FixedBigNumber operator+(FixedBigNumber const& o) const {
        FixedBigNumber n = *this;
        n += o;
        return n;
    }

    uint32_t divide(uint32_t n) {
        uint64_t r = 0;
        for (int i = size - 1; i >= 0; --i) {
            r = (r << 32) + val[i];
            lldiv_t d = lldiv(r, n);
            val[i] = d.quot;
            r = d.rem;
        }
        return r;
    }

    template<typename T>
    T translate() const {
        T v = 0;
        for (int i = size - 1; i >= 0; --i) {
            v <<= 32;
            v += val[i];
        }
        return v;
    }

private:
    void printHelper(std::ostream& os) {
        uint32_t r = divide(10);
        if (*this != 0) printHelper(os);
        os << r;
    }

public:
    friend std::ostream& operator<<(std::ostream& os, FixedBigNumber const& o) {
        FixedBigNumber n = o;
        n.printHelper(os);
        return os;
    }

    operator std::string() const {
        std::ostringstream ss;
        ss << *this;
        return ss.str();
    }
};
