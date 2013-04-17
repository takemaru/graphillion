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
    struct { // row-col order for experimentally fast reduce()
        uint64_t row :16;
        uint64_t col :48;
    };

    DdNodeId()
            : code(0) {
    }

    DdNodeId(size_t terminalValue)
            : row(0), col(terminalValue) {
    }

    DdNodeId(int row, size_t col)
            : row(row), col(col) {
    }

    size_t hash() const {
        return code * 314159257;
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

//    class RowRef {
//        DdNodeId* id;
//
//    public:
//        RowRef(DdNodeId& id)
//                : id(&id) {
//        }
//
//        void operator=(int row) {
//            id->row = row;
//        }
//
//        operator int() const {
//            return id->row;
//        }
//    };
//
//    class ColRef {
//        DdNodeId* id;
//
//    public:
//        ColRef(DdNodeId& id)
//                : id(&id) {
//        }
//
//        void operator=(size_t col) {
//            id->col = col;
//        }
//
//        operator size_t() const {
//            return id->col;
//        }
//    };
//
//    RowRef rowReference() {
//        return RowRef(*this);
//    }
//
//    ColRef colReference() {
//        return ColRef(*this);
//    }
};
