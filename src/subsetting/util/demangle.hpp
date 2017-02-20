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
#include <string>
#include <typeinfo>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace tdzdd {

inline std::string demangle(char const* name) {
#ifdef __GNUC__
    char* dName = abi::__cxa_demangle(name, 0, 0, 0);
    if (dName == 0) return name;
    char const* p = dName;
#else
    char const* p = name;
#endif

    std::string s;

    for (char c = *p++; c; c = *p++) {
        s += c;
        if (!isalnum(c)) {
            while (std::isspace(*p)) {
                ++p;
            }
        }
    }

#ifdef __GNUC__
    free(dName);
#endif
    return s;
}

inline std::string demangleTypename(char const* name) {
    std::string s = demangle(name);
    size_t i = 0;
    size_t j = 0;

    while (j + 1 < s.size()) {
        if (std::isalnum(s[j])) {
            ++j;
        }
        else if (s[j] == ':' && s[j + 1] == ':') {
            s = s.replace(i, j + 2 - i, "");
            j = i;
        }
        else if (s[j] == '(') { // (anonymous namespace)
            size_t k = j + 1;
            while (k < s.size() && s[k++] != ')');
            s = s.replace(j, k - j, "");
        }
        else {
            i = ++j;
        }
    }

    return s;
}

template<typename T>
std::string typenameof() {
    return demangleTypename(typeid(T).name());
}

template<typename T>
std::string typenameof(T const& obj) {
    return demangleTypename(typeid(obj).name());
}

} // namespace tdzdd
