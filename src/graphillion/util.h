/*********************************************************************
Copyright 2013  JST ERATO Minato project and other contributors
http://www-erato.ist.hokudai.ac.jp/?language=en

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**********************************************************************/

#ifndef GRAPHILLION_UTIL_H_
#define GRAPHILLION_UTIL_H_

#include <cstring>

#include <sstream>
#include <string>
#include <vector>

#include "subsetting/util/IntSubset.hpp"

namespace graphillion {

#ifdef __PRETTY_FUNCTION__
#define __FUNC__ (__PRETTY_FUNCTION__)
#else
#define __FUNC__ (__func__)
#endif

#ifndef assert
#define assert(e) {                                                     \
    if (!(e)) {                                                          \
      fprintf(stderr, "Error: %s:%u: %s: assertion `%s' failed.\n",     \
              __FILE__, __LINE__, __FUNC__, (#e));                      \
      exit(1);                                                          \
    }                                                                   \
  }
#endif

#define error_if(e, m) {                                                \
    if ((e)) {                                                          \
      std::stringstream ss;                                             \
      (ss << m);                                                        \
      fprintf(stderr, "Error: %s:%u: %s: %s, assertion `%s' failed.\n", \
              __FILE__, __LINE__, __FUNC__, ss.str().c_str(), (#e));    \
      exit(1);                                                          \
    }                                                                   \
  }

template <class T>
std::string join(const std::vector<T>& v, std::string sep = " ") {
  std::stringstream ss;
  for (int i = 0; i < static_cast<int>(v.size()); ++i) {
    ss << v[i];
    if (i < static_cast<int>(v.size()) - 1) ss << sep;
  }
  return ss.str();
}

std::vector<std::string> split(const std::string& str,
                               const std::string sep = " ");

inline bool is_space(std::string s) {
  return s.find_first_not_of(" \t\r\n") == std::string::npos;
}

inline bool is_digit(std::string s) {
  return s.find_first_not_of("0123456789") == std::string::npos;
}

}  // namespace graphillion

#endif  // GRAPHILLION_UTIL_H_
