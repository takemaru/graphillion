/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: DegreeConstraint.hpp 421 2013-02-25 05:33:17Z iwashita $
 */

#pragma once

#include <cassert>
#include <map>
#include <vector>

#include "../DdSpec.hpp"

namespace tdzdd {

template<typename T>
class LinearConstraints: public PodArrayDdSpec<LinearConstraints<T>,T,2> {
    struct CheckItem {
        int index;
        T weight;
        T addMin;
        T addMax;
        T lowerBound;
        T upperBound;
        bool finalChoice;

        CheckItem(int i,
                  T const& w,
                  T const& min,
                  T const& max,
                  T const& lb,
                  T const& ub,
                  bool fc) :
                index(i),
                weight(w),
                addMin(min),
                addMax(max),
                lowerBound(lb),
                upperBound(ub),
                finalChoice(fc) {
        }
    };

    typedef std::vector<CheckItem> Checklist;

    int const n;
    std::vector<Checklist> checklists;
    int arraySize;
    int constraintId;
    bool isFalse;

public:
    LinearConstraints(int n) :
            n(n),
            checklists(n + 1),
            arraySize(0),
            constraintId(0),
            isFalse(false) {
        assert(n >= 1);
    }

    void addConstraint(std::map<int,T> const& expr, T const& lb, T const& ub) {
        if (isFalse) return;
        T min = 0;
        T max = 0;
        for (typename std::map<int,T>::const_iterator t = expr.begin();
                t != expr.end(); ++t) {
            T const& w = t->second;
            if (w > 0) max += w;
            else if (w < 0) min += w;
        }
        if (lb <= min && max <= ub) return;
        if (ub < lb || max < lb || ub < min) {
            isFalse = true;
            return;
        }
        if (expr.empty()) return;

        min = 0;
        max = 0;
        bool fc = true;
        for (typename std::map<int,T>::const_iterator t = expr.begin();
                t != expr.end(); ++t) {
            Checklist& list = checklists[t->first];
            T const& w = t->second;
            list.push_back(CheckItem(constraintId, w, min, max, lb, ub, fc));
            if (w > 0) max += w;
            else if (w < 0) min += w;
            fc = false;
        }
        ++constraintId;
    }

    void update() {
        std::vector<int> indexMap(constraintId);
        for (int id = 0; id < constraintId; ++id) {
            indexMap[id] = -1;
        }
        std::vector<int> freeIndex;

        for (int i = n; i >= 1; --i) {
            Checklist& list = checklists[i];

            for (typename Checklist::iterator t = list.begin(); t != list.end();
                    ++t) {
                int id = t->index;

                if (indexMap[id] < 0) {
                    if (freeIndex.empty()) {
                        indexMap[id] = arraySize++;
                    }
                    else {
                        indexMap[id] = freeIndex.back();
                        freeIndex.pop_back();
                    }
                }

                t->index = indexMap[id];
            }

            for (typename Checklist::iterator t = list.begin(); t != list.end();
                    ++t) {
                if (t->finalChoice) {
                    freeIndex.push_back(t->index);
                }
            }
        }

        this->setArraySize(arraySize);
    }

    int getRoot(T* value) const {
        if (isFalse) return 0;

        for (int k = 0; k < arraySize; ++k) {
            value[k] = 0;
        }

        return n;
    }

    int getChild(T* value, int level, bool take) const {
        Checklist const& list = checklists[level];

        for (typename Checklist::const_iterator t = list.begin();
                t != list.end(); ++t) {
            T& v = value[t->index];
            if (take) v += t->weight;
            if (v + t->addMax < t->lowerBound || t->upperBound < v + t->addMin)
                return 0;
            if (t->lowerBound <= v + t->addMin
                && v + t->addMax <= t->upperBound) // state compression
            v = t->lowerBound - t->addMin;
            if (t->finalChoice) v = 0;
        }

        return (--level >= 1) ? level : -1;
    }
};

} // namespace tdzdd
