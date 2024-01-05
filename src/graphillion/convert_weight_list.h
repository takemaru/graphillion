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

#ifndef GRAPHILLION_CONVERT_WEIGHT_LIST_H_
#define GRAPHILLION_CONVERT_WEIGHT_LIST_H_

#include <cstring>
#include <string>
#include <vector>

#include "subsetting/util/Graph.hpp"

namespace graphillion {

template <typename T>
std::vector<T> convert_weight_list(
    const tdzdd::Graph &g,
    const std::map<std::string, T> &_weight_list) {
  std::vector<T> weight_list(g.vertexSize(), 1);
  for (const auto &p : _weight_list) {
    weight_list[g.getVertex(p.first) - 1] = p.second;
  }
  return weight_list;
}

}  // namespace graphillion

#endif  // GRAPHILLION_CONVERT_WEIGHT_LIST_H_
