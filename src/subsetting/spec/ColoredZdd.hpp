/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: ColoredZdd.hpp 414 2013-02-15 04:04:57Z iwashita $
 */

#pragma once

#include <cassert>
#include <iostream>
#include <vector>

#include "../dd/DdSpec.hpp"

class ColoredZdd: public PodArrayDdSpec<ColoredZdd,DdNodeId> {
    std::vector<ZddStructure const*> dds;
    int const colors;

    int getLevel(DdNodeId f, int k) const {
        int i = f.row;
        return (i == 0) ? -f.col : (i - 1) * colors + k + 1;
    }

    int getLevel(DdNodeId const* a) const {
        int level = 0;
        for (int k = 0; k < colors; ++k) {
            int i = getLevel(a[k], k);
            if (i == 0) return 0;
            level = std::max(level, i);
        }
        return level == 0 ? -1 : level;
    }

public:
    ColoredZdd(ZddStructure const& dd, int colors)
            : dds(colors), colors(colors) {
        setArraySize(colors);
        for (int k = 0; k < colors; ++k) {
            dds[k] = &dd;
        }
    }

    template<typename C>
    ColoredZdd(C const& c)
            : dds(), colors(c.size()) {
        setArraySize(colors);
        dds.reserve(colors);
        for (typename C::const_iterator t = c.begin(); t != c.end(); ++t) {
            dds.push_back(&*t);
        }
    }

    int getRoot(DdNodeId* a) const {
        for (int k = 0; k < colors; ++k) {
            a[k] = dds[k]->getRoot();
        }
        return getLevel(a);
    }

    int getChild(DdNodeId* a, int level, bool b) const {
        int i = (level - 1) / colors + 1;
        int k = (level - 1) % colors;

        if (b) {
            if (a[k].row == i) {
                a[k] = dds[k]->getChild(a[k], true);
            }
            else {
                a[k] = 0;
            }

            for (int kk = k - 1; kk >= 0; --kk) {
                if (a[kk].row == i) {
                    a[kk] = dds[kk]->getChild(a[kk], false);
                }
            }
        }
        else {
            bool lastOne = true;
            for (int kk = k - 1; kk >= 0; --kk) {
                if (a[kk].row == i) {
                    lastOne = false;
                    break;
                }
            }
            if (lastOne) return 0;

            if (a[k].row == i) {
                a[k] = dds[k]->getChild(a[k], false);
            }
        }

        return getLevel(a);
    }
};
