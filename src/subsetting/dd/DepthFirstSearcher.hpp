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

#include <cstdlib>
#include <utility>
#include <vector>

#include "../util/MyList.hpp"

namespace tdzdd {

template<typename S>
class DepthFirstSearcher {
    typedef S Spec;
    static int const AR = Spec::ARITY;

    Spec spec;
    int const datasize;

    MyList<char> statePool;
    std::vector<std::pair<int,int> > valueList;

public:
    DepthFirstSearcher(Spec const& spec) :
            spec(spec), datasize(spec.datasize()) {
    }

    /**
     * Returns a random instance using simple depth-first search.
     * It does not guarantee that the selection is uniform.
     * merge_states(void*, void*) is not supported.
     * @return a collection of (item, value) pairs.
     * @exception std::runtime_error no instance exists.
     */
    std::vector<std::pair<int,int> > findOneInstance() {
        valueList.clear();
        void* p = statePool.alloc_front(datasize);
        int n = spec.get_root(p);
        bool ok;

        if (n <= 0) {
            ok = (n != 0);
        }
        else {
            valueList.reserve(n);

            ok = findOneInstanceStep(p, n);

            for (int i = n; i >= 1; --i) {
                spec.destructLevel(i);
            }
        }

        spec.destruct(p);
        statePool.pop_front();
        if (!ok) throw std::runtime_error("No instance");
        return valueList;
    }

private:
    bool findOneInstanceStep(void* p, int i) {
        int b0 = std::rand() % AR;
        void* pp = statePool.alloc_front(datasize);
        bool ok = false;

        for (int bb = 0; bb < AR; ++bb) {
            int b = (b0 + bb) % AR;

            spec.get_copy(pp, p);
            int ii = spec.get_child(pp, i, b);
            if (ii <= 0) {
                ok = (ii != 0);
            }
            else {
                assert(ii < i);
                ok = findOneInstanceStep(pp, ii);
            }
            spec.destruct(pp);

            if (ok) {
                valueList.push_back(std::make_pair(i, b));
                break;
            }
        }

        statePool.pop_front();
        return ok;
    }
};

} // namespace tdzdd
