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

#ifndef GRAPHILLION_GRAPHSET_H_
#define GRAPHILLION_GRAPHSET_H_

#include "subsetting/util/IntSubset.hpp"

#include "graphillion/setset.h"

namespace graphillion {

class Range : public tdzdd::IntSubset {
 public:
  Range(int max = 1);
  Range(int min, int max, int step = 1);

  bool contains(int x) const;
  int lowerBound() const;
  int upperBound() const;

 private:
  int min_;
  int max_;
  int step_;
};

setset SearchGraphs(
    const std::vector<edge_t>& graph,
    const std::vector<std::vector<vertex_t> >* vertex_groups = NULL,
    const std::map<vertex_t, Range>* degree_constraints = NULL,
    const Range* num_edges = NULL,
    int num_comps = -1,  // not including vertex_groups
    bool no_loop = false,
    const setset* search_space = NULL,
    const std::vector<linear_constraint_t>* linear_constraints = NULL);

bool ShowMessages(bool flag = true);

}  // namespace graphillion

#endif  // GRAPHILLION_GRAPHSET_H_
