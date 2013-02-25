/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: InstanceFinder.hpp 412 2013-02-15 01:13:19Z iwashita $
 */

#pragma once

#include <cassert>
#include <ostream>
#include <vector>

//#include <Judy.h>

#include "../util/demangle.hpp"
#include "../util/MessageHandler.hpp"
#include "../util/MemoryPool.hpp"
#include "../util/MyHashTable.hpp"
#include "DdSpec.hpp"

template<typename DD>
class InstanceFinder {
    DD& dd;
    int stateSize;
    //size_t const tableSize;
    //std::vector<bool> table;
    //Pvoid_t table;
    MessageHandler mh;
    int instanceNumber;

public:
    InstanceFinder(DD& dd, size_t searchSpace = 0)
            : dd(dd), stateSize(dd.datasize()),
            //tableSize(searchSpace),
            //table(tableSize),//
            //table(0), //
              instanceNumber(0) {
    }

//    ~InstanceFinder() {
//        int rc;
//        J1FA(rc, table);
//    }

    bool find() {
        mh.begin("finding") << " an instance of " << typenameof(dd) << " ...";

        char s[stateSize];
        int const n = dd.get_root(s);

        if (find(s, n)) {
            std::cout << "\n";
            mh.end("succeeded");
            return true;
        }

        mh.end("failed");
        return false;
    }

private:
    bool find(char* s, int i) {
        if (i == 0) return false;
        if (i < 0) {
            mh << "\n";
            std::cout << "#" << ++instanceNumber;
            return true;
        }

//        if (tableSize > 0) {
//            size_t code = (dd.hash_code(s) + i * 271828171) % tableSize;
//            //if (table[code]) return false;
//            //table[code] = true;
//            int rc;
//            J1S(rc, table, code);
//            if (!rc) return false;
//        }

        char t[stateSize];
        dd.get_copy(t, s);

        return find(s, i, false) || find(t, i, true);
    }

    bool find(char* s, int i, bool b) {
        int ii = dd.get_child(s, i, b);
        if (find(s, ii)) {
            if (b) std::cout << "," << i;
            return true;
        }
        return false;
    }
};

//template<typename S>
//bool findInstance(DdSpec<S> const& dd, size_t searchSpace = 0) {
//    return InstanceFinder<S>(dd.entity(), searchSpace).find();
//}
