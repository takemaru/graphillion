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
#include <cerrno>
#include <climits>
#include <ostream>
#include <stdint.h>
#include <string>
#include <vector>

#include "../DdSpec.hpp"

namespace tdzdd {

class GraphillionZdd: public tdzdd::DdSpec<GraphillionZdd,uint64_t,2> {
    struct Node {
        int index;
        uint64_t child[2];
    };

    std::vector<Node> table;
    uint64_t root;
    int minIndex;
    int maxIndex;

public:
    GraphillionZdd()
            : root(0), minIndex(INT_MAX), maxIndex(INT_MIN) {
    }

    void read(std::string const& filename = "") {
        tdzdd::MessageHandler mh;
        mh.begin("reading");

        if (filename.empty()) {
            mh << " STDIN ...";
            read(std::cin);
        }
        else {
            mh << " \"" << filename << "\" ...";
            std::ifstream fin(filename.c_str(), std::ios::in);
            if (!fin) throw std::runtime_error(strerror(errno));
            read(fin);
        }

        mh.end();
    }

    void addNode(uint64_t id, Node const& node) {
        if (minIndex > node.index) minIndex = node.index, root = id;
        if (maxIndex < node.index) maxIndex = node.index;

        uint64_t n = table.size();
        if (n <= id) n = id + 1;
        if (n <= node.child[0]) n = node.child[0] + 1;
        if (n <= node.child[1]) n = node.child[1] + 1;
        if (n > table.size()) table.resize(n * 2);

        table.at(id) = node;
    }

private:
    void read(std::istream& is) {
        while (is) {
            if (isdigit(skipSpace(is))) {
                uint64_t id;
                Node node;

                id = readID(is);
                is >> node.index;
                node.child[0] = readID(is);
                node.child[1] = readID(is);

                addNode(id, node);
            }
            skipLine(is);
        }
    }

    static uint64_t readID(std::istream& is) {
        int c;
        uint64_t id;
        while (isspace(c = is.get()))
            ;
        if (isdigit(c)) {
            is.unget();
            is >> id;
            id += 2;
        }
        else {
            id = (c == 'T' || c == 't') ? 1 : 0;
        }
        return id;
    }

    static int skipSpace(std::istream& is) {
        int c;
        while (isspace(c = is.get()))
            ;
        is.unget();
        return c;
    }

    static void skipLine(std::istream& is) {
        while (is && is.get() != '\n')
            ;
    }

public:
    int getRoot(uint64_t& f) const {
        f = root;
        if (f == 0) return 0;
        if (f == 1) return -1;
        return maxIndex - table.at(f).index + 1;
    }

    int getChild(uint64_t& f, int level, int take) const {
        f = table.at(f).child[take];
        if (f == 0) return 0;
        if (f == 1) return -1;
        return maxIndex - table.at(f).index + 1;
    }
};

} // namespace tdzdd
