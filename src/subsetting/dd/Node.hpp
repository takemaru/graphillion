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
#include <stdint.h>
#include <ostream>

namespace tdzdd {

int const NODE_ROW_BITS = 20;
int const NODE_ATTR_BITS = 1;
int const NODE_COL_BITS = 64 - NODE_ROW_BITS - NODE_ATTR_BITS;

int const NODE_ROW_OFFSET = NODE_COL_BITS + NODE_ATTR_BITS;
int const NODE_ATTR_OFFSET = NODE_COL_BITS;

uint64_t const NODE_ROW_MAX = (uint64_t(1) << NODE_ROW_BITS) - 1;
uint64_t const NODE_COL_MAX = (uint64_t(1) << NODE_COL_BITS) - 1;

uint64_t const NODE_ROW_MASK = NODE_ROW_MAX << NODE_ROW_OFFSET;
uint64_t const NODE_ATTR_MASK = uint64_t(1) << NODE_ATTR_OFFSET;

class NodeId {
    uint64_t code_;

public:
    NodeId() { // 'code_' is not initialized in the default constructor for SPEED.
    }

    NodeId(uint64_t code) :
            code_(code) {
    }

    NodeId(uint64_t row, uint64_t col) :
            code_((row << NODE_ROW_OFFSET) | col) {
    }

    NodeId(uint64_t row, uint64_t col, bool attr) :
            code_((row << NODE_ROW_OFFSET) | col) {
        setAttr(attr);
    }

    int row() const {
        return code_ >> NODE_ROW_OFFSET;
    }

    size_t col() const {
        return code_ & NODE_COL_MAX;
    }

    void setAttr(bool val) {
        if (val) {
            code_ |= NODE_ATTR_MASK;
        }
        else {
            code_ &= ~NODE_ATTR_MASK;
        }
    }

    bool getAttr() const {
        return (code_ & NODE_ATTR_MASK) != 0;
    }

    NodeId withoutAttr() const {
        return code_ & ~NODE_ATTR_MASK;
    }

    bool hasEmpty() const {
        return code_ == 1 || getAttr();
    }

    uint64_t code() const {
        return code_ & ~NODE_ATTR_MASK;
    }

    size_t hash() const {
        return code() * 314159257;
    }

    bool operator==(NodeId const& o) const {
        return code() == o.code();
    }

    bool operator!=(NodeId const& o) const {
        return !(*this == o);
    }

    bool operator<(NodeId const& o) const {
        return code() < o.code();
    }

    bool operator>=(NodeId const& o) const {
        return !(*this < o);
    }

    bool operator>(NodeId const& o) const {
        return o < *this;
    }

    bool operator<=(NodeId const& o) const {
        return !(o < *this);
    }

    friend std::ostream& operator<<(std::ostream& os, NodeId const& o) {
        os << o.row() << ":" << o.col();
        if (o.code_ & NODE_ATTR_MASK) os << "+";
        return os;
    }
};

struct NodeBranchId {
    size_t col;
    int row;
    int val;

    NodeBranchId() {
    }

    NodeBranchId(int row, size_t col, int val) :
            col(col), row(row), val(val) {
    }
};

template<int ARITY>
struct Node {
    NodeId branch[ARITY];

    Node() {
    }

    Node(NodeId f0, NodeId f1) {
        branch[0] = f0;
        for (int i = 1; i < ARITY; ++i) {
            branch[i] = f1;
        }
    }

    Node(NodeId const* f) {
        for (int i = 0; i < ARITY; ++i) {
            branch[i] = f[i];
        }
    }

    size_t hash() const {
        size_t h = branch[0].code();
        for (int i = 1; i < ARITY; ++i) {
            h = h * 314159257 + branch[i].code() * 271828171;
        }
        return h;
    }

    bool operator==(Node const& o) const {
        for (int i = 0; i < ARITY; ++i) {
            if (branch[i] != o.branch[i]) return false;
        }
        return true;
    }

    bool operator!=(Node const& o) const {
        return !operator==(o);
    }

    friend std::ostream& operator<<(std::ostream& os, Node const& o) {
        os << "(" << o.branch[0];
        for (int i = 1; i < ARITY; ++i) {
            os << "," << o.branch[i];
        }
        return os << ")";
    }
};

template<int ARITY>
struct InitializedNode: Node<ARITY> {
    InitializedNode() :
            Node<ARITY>(0, 0) {
    }

    InitializedNode(NodeId f0, NodeId f1) :
            Node<ARITY>(f0, f1) {
    }

    InitializedNode(Node<ARITY> const& o) :
            Node<ARITY>(o.branch) {
    }
};

} // namespace tdzdd
