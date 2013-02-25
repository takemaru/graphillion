/*
 * Decode "mangled" name
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: demangle.hpp 410 2013-02-14 06:33:04Z iwashita $
 */

#pragma once

#include <cstdlib>
#include <cxxabi.h>
#include <string>
#include <typeinfo>

inline std::string demangle(char const* name) {
    char* dName = abi::__cxa_demangle(name, 0, 0, 0);
    if (dName == 0) return name;

    std::string s;
    char* p = dName;
    char c;
    while ((c = *p++) != 0) {
        s += c;
        if (!isalnum(c)) {
            while (std::isspace(*p)) {
                ++p;
            }
        }
    }

    free(dName);
    return s;
}

template<typename T>
std::string typenameof() {
    return demangle(typeid(T).name());
}

template<typename T>
std::string typenameof(T const& obj) {
    return demangle(typeid(obj).name());
}
