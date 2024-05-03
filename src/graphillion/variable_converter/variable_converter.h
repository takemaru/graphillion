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

#ifndef GRAPHILLION_VARIABLE_CONVERTER_H_
#define GRAPHILLION_VARIABLE_CONVERTER_H_

#include <vector>

#include "SAPPOROBDD/ZBDD.h"
#include "subsetting/DdStructure.hpp"
#include "subsetting/util/Graph.hpp"

namespace VariableConverter {

class VariableList {
 public:
  enum Kind {VERTEX, EDGE};

 private:
  std::vector<Kind> kind_list_;  // index: ev
  std::vector<int> variable_number_list_;  // index: ev
  std::vector<int> ev_to_newv_;  // index: ev
  std::vector<int> v_to_newv_;  // index: v
  std::vector<int> newv_to_v_;  // index: newv
  int m_;
  int n_;

 public:
  explicit VariableList(const tdzdd::Graph& graph);
  void constructEVArray(const tdzdd::Graph& graph);
  Kind getKind(int evindex) const;
  int getVariableNumber(int evindex) const;
  int evToNewV(int evindex) const;
  int newVToV(int newv) const;
};

tdzdd::DdStructure<2> eToEvZdd(const tdzdd::DdStructure<2>& dd,
                    const tdzdd::Graph& graph,
                    const VariableList& vlist);

ZBDD eToVZdd(const tdzdd::DdStructure<2>& dd,
          const tdzdd::Graph& graph);

ZBDD eToVZdd(const tdzdd::DdStructure<2>& dd,
          const tdzdd::Graph& graph,
          const VariableList& vlist,
          const int offset = 0);

std::pair<tdzdd::Graph, VariableList> construct_graph_and_vlist(
  const std::vector<std::vector<std::string>> &edges_from_top
);

std::vector<std::string> get_vertices_from_top(
  const std::vector<std::vector<std::string>> &edges_from_top
);

}  // namespace VariableConverter

#endif  // GRAPHILLION_VARIABLE_CONVERTER_H_
