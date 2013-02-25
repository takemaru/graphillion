/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: NQueenZdd.hpp 414 2013-02-15 04:04:57Z iwashita $
 */

#pragma once

#include <cassert>
#include <iostream>

#include "../dd/DdSpec.hpp"

template<typename S, bool rookOnly>
class NQueenZddBase: public PodArrayDdSpec<S,uint32_t> {
    typedef uint32_t Bitmap;

    int const n;
    int const topLevel;
    bool const takeTop;

    static Bitmap bit(int j) {
        return Bitmap(1) << j;
    }

public:

    NQueenZddBase(int n)
            : n(n), topLevel(n * n), takeTop(false) {
        assert(n >= 1);
        this->setArraySize(n);
    }

    NQueenZddBase(int n, int firstCol)
            : n(n), topLevel(n * n - firstCol), takeTop(true) {
        assert(n >= 1);
        assert(0 <= firstCol && firstCol < n);
        this->setArraySize(n);
    }

    int getRoot(Bitmap* bitmap) const {
        Bitmap s = ~(Bitmap(-1) << n);
        for (int i = 0; i < n; ++i) {
            bitmap[i] = s;
        }
        return topLevel;
    }

    int getChild(Bitmap* bitmap, int level, bool b) const {
        int i = (level - 1) / n;
        int j = (level - 1) % n;

        if (b) {
            if ((bitmap[i] & bit(j)) == 0) return 0;

            Bitmap total = 0;
            for (int ii = i - 1; ii >= 0; --ii) {
                bitmap[ii] &= ~bit(j);
                if (!rookOnly) {
                    int d = i - ii;
                    if (j - d >= 0) bitmap[ii] &= ~bit(j - d);
                    if (j + d < n) bitmap[ii] &= ~bit(j + d);
                }
                if (bitmap[ii] == 0) return 0;
                total |= bitmap[ii];
            }

            if (i == 0) return -1;
            if (__builtin_popcount(total) < i) return 0;

            level = i * n + 1;
            bitmap[i] = 0;
            //        std::cerr << "\n(" << i << ", " << j << ")\n";
            //        for (int ii = 0; ii < n; ++ii) {
            //            for (int jj = 0; jj < n; ++jj) {
            //                std::cerr << (bitmap[ii] & bit(jj) ? " *" : " -");
            //            }
            //            std::cerr << "\n";
            //        }
        }
        else {
            if (takeTop && level == topLevel) return 0;
            bitmap[i] &= ~bit(j);
            if (bitmap[i] == 0) return 0;
            assert(j >= 1);
        }

        while (true) {
            --level;
            i = (level - 1) / n;
            j = (level - 1) % n;

            if (bitmap[i] & bit(j)) {
                bool takable = true;
                Bitmap total = 0;

                for (int ii = i - 1; ii >= 0; --ii) {
                    Bitmap bm = bitmap[ii];
                    bm &= ~bit(j);
                    if (!rookOnly) {
                        int d = i - ii;
                        if (j - d >= 0) bm &= ~bit(j - d);
                        if (j + d < n) bm &= ~bit(j + d);
                    }
                    if (bm == 0) {
                        takable = false;
                        break;
                    }
                    total |= bm;
                }

                if (takable && __builtin_popcount(total) >= i) break;
            }

            bitmap[i] &= ~bit(j);
            if (bitmap[i] == 0) return 0;
        }

        return level;
    }
};

struct NQueenZdd: public NQueenZddBase<NQueenZdd,false> {
    NQueenZdd(int n)
            : NQueenZddBase<NQueenZdd,false>(n) {
    }

    NQueenZdd(int n, int firstCol)
            : NQueenZddBase<NQueenZdd,false>(n, firstCol) {
    }
};

struct NRookZdd: public NQueenZddBase<NRookZdd,true> {
    NRookZdd(int n)
            : NQueenZddBase<NRookZdd,true>(n) {
    }

    NRookZdd(int n, int firstCol)
            : NQueenZddBase<NRookZdd,true>(n, firstCol) {
    }
};

template<typename S, bool rookOnly>
class ColoredNQueenZddBase: public PodArrayDdSpec<S,uint32_t> {
    typedef uint32_t Bitmap;

    int const n;
    int const topLevel;

private:
    static Bitmap bit(int j) {
        return Bitmap(1) << j;
    }

public:
    ColoredNQueenZddBase(int n)
            : n(n), topLevel(n * n * n) {
        this->setArraySize(n);
    }

    int getRoot(Bitmap* bitmap) const {
        int const m = n * n;
        Bitmap const s = ~(Bitmap(-1) << n);
        for (int i = n - 1; i >= 0; --i) {
            bitmap[m - n + i] = bit(i);
        }
        for (int i = m - n - 1; i >= 0; --i) {
            bitmap[i] = s;
        }
        return topLevel;
    }

    int getChild(Bitmap* bitmap, int level, bool b) const {
        int const n2 = n * n;
        int k = level - 1;
        int i = k / n2;
        k %= n2;
        int j = k / n;
        k %= n;
        int ik = i * n + k;

        if (b) {
            if ((bitmap[ik] & bit(j)) == 0) return 0;

            Bitmap total = 0;
            for (int ii = i - 1; ii >= 0; --ii) {
                int iik = ii * n + k;
                bitmap[iik] &= ~bit(j);
                if (!rookOnly) {
                    int d = i - ii;
                    if (j - d >= 0) bitmap[iik] &= ~bit(j - d);
                    if (j + d < n) bitmap[iik] &= ~bit(j + d);
                }
                if (bitmap[iik] == 0) return 0;
                total |= bitmap[iik];
            }

            if (__builtin_popcount(total) < i) return 0;
            if (i == 0 && j == 0) return -1;

            bitmap[ik] = bit(n - 1);
            for (int ikk = ik - 1; ikk >= i * n; --ikk) {
                bitmap[ikk] &= ~bit(j);
            }

            //        std::cerr << "\n(" << i << ", " << j << ")\n";
            //        for (int ii = 0; ii < n; ++ii) {
            //            for (int jj = 0; jj < n; ++jj) {
            //                std::cerr << " ";
            //                for (int kk = 0; kk < n; ++kk) {
            //                    if (bitmap[ii * n + kk] & bit(jj)) {
            //                        std::cerr << kk;
            //                    }
            //                    else {
            //                        std::cerr << "-";
            //                    }
            //                }
            //            }
            //            std::cerr << "\n";
            //        }
        }
        else {
            bitmap[ik] &= ~bit(j);
            if (bitmap[ik] == 0) return 0;
            assert(j >= 1);
        }

        while (true) {
            --level;
            k = level - 1;
            i = k / n2;
            k %= n2;
            j = k / n;
            k %= n;
            ik = i * n + k;

            if (bitmap[ik] & bit(j)) {
                bool takable = true;
                Bitmap total = 0;

                for (int ii = i - 1; ii >= 0; --ii) {
                    int iik = ii * n + k;
                    Bitmap bm = bitmap[iik];
                    bm &= ~bit(j);
                    if (!rookOnly) {
                        int d = i - ii;
                        if (j - d >= 0) bm &= ~bit(j - d);
                        if (j + d < n) bm &= ~bit(j + d);
                    }
                    if (bm == 0) {
                        takable = false;
                        break;
                    }
                    total |= bm;
                }

                if (takable && __builtin_popcount(total) >= i) break;
            }

            bitmap[ik] &= ~bit(j);
            if (bitmap[ik] == 0) return 0;
        }

        return level;
    }
};

struct ColoredNQueenZdd: public ColoredNQueenZddBase<ColoredNQueenZdd,false> {
    ColoredNQueenZdd(int n)
            : ColoredNQueenZddBase<ColoredNQueenZdd,false>(n) {
    }
};

struct ColoredNRookZdd: public ColoredNQueenZddBase<ColoredNRookZdd,true> {
    ColoredNRookZdd(int n)
            : ColoredNQueenZddBase<ColoredNRookZdd,true>(n) {
    }
};
