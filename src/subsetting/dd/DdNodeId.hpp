/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: DdNodeId.hpp 410 2013-02-14 06:33:04Z iwashita $
 */

#pragma once

#include <cassert>
#include <stdint.h>
#include <ostream>

union DdNodeId {
    uint64_t code;
    struct {
        uint64_t row :16;
        uint64_t col :48;
    };

    DdNodeId()
            : code(0) {
    }

    DdNodeId(size_t val)
            : row(0), col(val) {
    }

    DdNodeId(int row, size_t col)
            : row(row), col(col) {
    }

    size_t hash() const {
        return code;
    }

    bool operator==(DdNodeId const& o) const {
        return code == o.code;
    }

    bool operator!=(DdNodeId const& o) const {
        return code != o.code;
    }

    bool operator<(DdNodeId const& o) const {
        return row < o.row || (row == o.row && code < o.code);
    }

    bool operator>=(DdNodeId const& o) const {
        return !(*this < o);
    }

    bool operator>(DdNodeId const& o) const {
        return o < *this;
    }

    bool operator<=(DdNodeId const& o) const {
        return !(o < *this);
    }

    friend std::ostream& operator<<(std::ostream& os, DdNodeId const& o) {
        return os << o.row << ":" << o.col;
    }
};

//namespace std {
//template<>
//struct hash<DdNodeId> {
//    size_t operator()(DdNodeId const& o) const {
//        return o.hash();
//    }
//};
//}
