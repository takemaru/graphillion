/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: Cardinality.hpp 412 2013-02-15 01:13:19Z iwashita $
 */

#pragma once

#include <string>

#include "../dd/DdEval.hpp"
#include "../util/BigNumber.hpp"
#include "../util/MemoryPool.hpp"
#include "../util/MyVector.hpp"

template<typename T = std::string>
struct Cardinality: public DdEval<Cardinality<T>,T> {
    void evalTerminal(T& n, bool one) const {
        n = one ? 1 : 0;
    }

    void evalNode(T& n, int, T const& n0, int, T const& n1, int) const {
        n = n0 + n1;
    }
};

template<>
class Cardinality<std::string> : public DdEval<Cardinality<std::string>,
        BigNumber,std::string> {
    MyVector<MemoryPool> pools;
    int numberSize;

public:
    Cardinality()
            : numberSize(2) {
    }

    void initialize(int level) {
        pools.resize(level + 1);
    }

    void evalTerminal(BigNumber& n, bool one) {
        n.setArray(pools[0].allocate<uint64_t>(1));
        n.store(one ? 1 : 0);
    }

    void evalNode(BigNumber& n, int i, BigNumber const& n0, int i0,
            BigNumber const& n1, int i1) {
        assert(numberSize >= 1);
        assert(size_t(i) <= pools.size());
        n.setArray(pools[i].allocate<uint64_t>(numberSize));
        n.store(n0);
        int w = n.add(n1);
        if (numberSize <= w) numberSize = w + 1;
    }

    std::string getValue(BigNumber const& n) const {
        return std::string(n);
    }

    void destructLevel(int i) {
        pools[i].clear();
    }
};
