/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: DdNode.hpp 389 2012-12-17 05:24:43Z iwashita $
 */

#pragma once

#include <cassert>
#include <ostream>

#include "DdNodeId.hpp"

struct DdNode {
    DdNodeId branch[2];

    static DdNode* terminal(bool b) {
        return reinterpret_cast<DdNode*>(b);
    }

    /**
     * Check if this is a terminal node.
     * @return <b>true</b> if this is a terminal node.
     */
    bool isTerminal() const {
        return isTerminal(0) || isTerminal(1);
    }

    /**
     * Check if this is a <i>c</i>-terminal node.
     * @param b the terminal value.
     * @return <b>true</b> if this is a <i>b</i>-terminal node.
     */
    bool isTerminal(bool b) const {
        return this == terminal(b);
    }

    size_t hash() const {
        return branch[0].code * 314159257 + branch[1].code * 271828171;
    }

    bool operator==(DdNode const& o) const {
        return branch[0] == o.branch[0] && branch[1] == o.branch[1];
    }

    friend std::ostream& operator<<(std::ostream& os, DdNode const& o) {
        return os << "(" << o.branch[0] << "," << o.branch[1] << ")";
    }
};
