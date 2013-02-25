/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: TableToZdd.hpp 414 2013-02-15 04:04:57Z iwashita $
 */

#pragma once

#include <cassert>
//#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "../dd/DdSpec.hpp"

class TableToZdd: public PodArrayDdSpec<TableToZdd,bool> {
    std::vector<std::vector<int> > table;
    int m;
    int n;

    template<typename C>
    void add(C const& list) {
        for (typename C::const_iterator t = list.begin(); t != list.end();
                ++t) {
            if (*t == 0) throw std::runtime_error("Illegal argument");
            n = std::max(n, std::abs(*t));
        }

        table.resize(table.size() + 1);
        std::vector<int>& val = table.back();
        val.resize(n + 1);

        for (typename C::const_iterator t = list.begin(); t != list.end();
                ++t) {
            val[std::abs(*t)] = (*t > 0) ? 1 : -1;
        }

        m = table.size();
        setArraySize(m);
    }

public:
//    TableToZdd(std::initializer_list<std::initializer_list<int> > list) {
//        init(list.begin(), list.end());
//    }

//    TableToZdd(std::initializer_list<int> list)
//            : m(0), n(0) {
//        add(list);
//    }

//    TableToZdd(std::initializer_list<std::initializer_list<int> > array)
//            : m(0), n(0) {
//        for (auto const& list : array) {
//            add(list);
//        }
//        for (auto& val : table) {
//            val.resize(n + 1);
//        }
//    }

    int getRoot(bool* a) const {
        for (int k = 0; k < m; ++k) {
            a[k] = true;
        }
        return m == 0 ? 0 : n;
    }

    int getChild(bool* a, int level, bool b) const {
        bool alive = false;

        for (int k = 0; k < m; ++k) {
            if (!a[k]) continue;

            if (table[k][level] == b || table[k][level] < 0) {
                alive = true;
            }
            else {
                a[k] = false;
            }
        }

        if (!alive) return 0;
        if (--level == 0) return -1;

        int maxLevel = 0;
        for (int k = 0; k < m; ++k) {
            if (!a[k]) continue;

            for (int i = level; i > maxLevel; --i) {
                if (table[k][i] != 0) {
                    maxLevel = i;
                    break;
                }
            }
        }

        return maxLevel == 0 ? -1 : maxLevel;
    }
};
