/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: PsnZdd.hpp 414 2013-02-15 04:04:57Z iwashita $
 */

#pragma once

#include <cassert>
#include <stdint.h>
#include <cstring>
#include <iostream>

#include "../dd/DdSpec.hpp"

struct PsnZddState {
    uint8_t waiting :1;
    uint8_t value :7;

    friend std::ostream& operator<<(std::ostream& os, PsnZddState const& o) {
        return os << static_cast<int>(o.value) << (o.waiting ? "#" : "");
    }
};

class PsnZdd: public PodArrayDdSpec<PsnZdd,PsnZddState> {
    int const n;
    int const topLevel;
    bool gridStyle;
    bool extraMerge;

public:
    PsnZdd(int n, bool gridStyle = false, bool extraMerge = false)
            : n(n), topLevel(n * (n - 1) * (n - 1) / 2), gridStyle(gridStyle),
              extraMerge(extraMerge) {
        this->setArraySize(n);
    }

    int getRoot(PsnZddState* perm) const {
        for (int i = 0; i < n; ++i) {
            perm[i].waiting = false;
            perm[i].value = n - i;
        }
        return topLevel;
    }

    /*
     * 逆順なのに線を引かなかったら waiting 状態に入る。
     * 一行引かなかったら解なし。
     * 左も右も動く見込みがないのに waiting 状態に入ったら解なし。
     */
    int getChild(PsnZddState* perm, int level, bool take) const {
        assert(1 <= level && level <= topLevel);
        int k = (topLevel - level) % (n - 1);
//        auto d = std::div(topLevel - level, n - 2);
//        bool goingRight = !(d.quot & 1);
//        int k = goingRight ? d.rem : n - 2 - d.rem;

        if (take) {
            if (!takable(perm, k)) return 0;

            if (k >= 1) perm[k - 1].waiting = false;
            perm[k + 1].waiting = false;

            assert(!perm[k].waiting && !perm[k + 1].waiting);
            std::swap(perm[k], perm[k + 1]);
            if (sorted(perm)) return -1;

            if (extraMerge) doExtraMerge(perm);

            if (gridStyle) {
                if (k < n - 2) { // cannot take the successor
                    ++k;
                    --level;
                }
            }
        }

        do {
            if (!take && perm[k].value > perm[k + 1].value) {
                if ((!takable(perm, 0, k) || maxAtRightEnd(perm, 0, k))
                        && (!takable(perm, k + 1, n - 1)
                                || minAtLeftEnd(perm, k + 1, n - 1))) return 0;

                perm[k].waiting = true; // cannot take this row until a neighbor is taken
            }

            if (++k == n - 1) k = 0;
//            if (goingRight) {
//                if (++k == n - 2) goingRight = false;
//            }
//            else {
//                if (--k == 0) goingRight = true;
//            }

            --level;
            take = false;
        } while (!takable(perm, k));

        return level;
    }

private:
    bool sorted(PsnZddState const* perm) const {
        for (int k = n - 1; k >= 1; --k) {
            if (perm[k - 1].value > perm[k].value) return false;
        }
        return true;
    }

    bool takable(PsnZddState const* perm, int k) const {
        return !perm[k].waiting && perm[k].value > perm[k + 1].value;
    }

    bool takable(PsnZddState const* perm, int k1, int k2) const {
        for (int k = k1; k < k2; ++k) {
            if (takable(perm, k)) return true;
        }
        return false;
    }

    bool minAtLeftEnd(PsnZddState const* perm, int k1, int k2) const {
        unsigned v = perm[k1].value;
        for (int k = k1 + 1; k <= k2; ++k) {
            if (v > perm[k].value) return false;
        }
        return true;
    }

    bool maxAtRightEnd(PsnZddState const* perm, int k1, int k2) const {
        unsigned v = perm[k2].value;
        for (int k = k1; k < k2; ++k) {
            if (v < perm[k].value) return false;
        }
        return true;
    }

    void doExtraMerge(PsnZddState* perm) const {
        while (perm[n - 1].value == n) {
            assert(!perm[n - 1].waiting);

            for (int k = n - 1; k >= 1; --k) {
                perm[k].waiting = perm[k - 1].waiting;
                perm[k].value = perm[k - 1].value + 1;
            }

            perm[0].waiting = false;
            perm[0].value = 1;
        }
    }
//    void doExtraMerge(PsnZddState* perm) const {
//        while (perm[1].value == 1) {
//            assert(!perm[1].waiting);
//
//            for (int k = 1; k < n; ++k) {
//                perm[k].waiting = perm[k + 1].waiting;
//                perm[k].value = perm[k + 1].value - 1;
//            }
//
//            perm[n].waiting = false;
//            perm[n].value = n;
//        }
//    }
};
