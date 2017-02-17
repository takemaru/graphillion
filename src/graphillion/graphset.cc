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

#include "graphillion/graphset.h"

#include <climits>

#include <set>
#include <vector>

#include "subsetting/DdStructure.hpp"
#include "subsetting/DdSpecOp.hpp"
#include "subsetting/eval/ToZBDD.hpp"
#include "subsetting/spec/DegreeConstraint.hpp"
#include "subsetting/spec/FrontierBasedSearch.hpp"
#include "subsetting/spec/LinearConstraints.hpp"
#include "subsetting/spec/SapporoZdd.hpp"
#include "subsetting/spec/SizeConstraint.hpp"
#include "subsetting/util/Graph.hpp"

#include "graphillion/setset.h"

namespace graphillion {

using std::map;
using std::pair;
using std::set;
using std::vector;
using namespace tdzdd;

Range::Range(int max) : min_(0), max_(max - 1), step_(1) {
  assert(this->min_ <= this->max_);
}

Range::Range(int min, int max, int step)
    : min_(min), max_(max - 1), step_(step) {
  assert(this->min_ <= this->max_);
  assert(this->step_ > 0);
}

bool Range::contains(int x) const {
  if (x < this->min_ || this->max_ < x) return false;
  return (x - this->min_) % this->step_ == 0;
}

int Range::lowerBound() const {
  return this->min_;
}

int Range::upperBound() const {
  return this->max_;
}

setset SearchGraphs(
    const vector<edge_t>& graph,
    const vector<vector<vertex_t> >* vertex_groups,
    const map<vertex_t, Range>* degree_constraints,
    const Range* num_edges,
    int num_comps,
    bool no_loop,
    const setset* search_space,
    const vector<linear_constraint_t>* linear_constraints) {
  assert(static_cast<size_t>(setset::num_elems()) == graph.size());

  Graph g;
  for (vector<edge_t>::const_iterator e = graph.begin(); e != graph.end(); ++e)
    g.addEdge(e->first, e->second);
  g.update();
  assert(static_cast<size_t>(g.edgeSize()) == graph.size());

#ifdef _OPENMP
  bool use_mp = (omp_get_num_procs() >= 2);
#else
  bool use_mp = false;
#endif

  DdStructure<2> dd;
  if (search_space != NULL) {
    SapporoZdd f(search_space->zdd_, setset::max_elem() - setset::num_elems());
    dd = DdStructure<2>(f, use_mp);
  } else {
    dd = DdStructure<2>(g.edgeSize(), use_mp);
  }

  if (vertex_groups != NULL) {
    int color = 0;
    for (vector<vector<vertex_t> >::const_iterator i = vertex_groups->begin();
         i != vertex_groups->end(); ++i, ++color)
      for (vector<vertex_t>::const_iterator v = i->begin(); v != i->end(); ++v)
        g.setColor(*v, color);
    g.update();
  }

  DegreeConstraint dc(g);
  if (degree_constraints != NULL) {
    for (map<vertex_t, Range>::const_iterator i = degree_constraints->begin();
         i != degree_constraints->end(); ++i)
      dc.setConstraint(i->first, &i->second);
    dd.zddSubset(dc);
  }

  if (num_edges != NULL) {
    SizeConstraint sc(g.edgeSize(), num_edges);
    dd.zddSubset(sc);
  }

  FrontierBasedSearch fbs(g, num_comps, no_loop);

  if (linear_constraints != NULL) {
    LinearConstraints<double> lc(g.edgeSize());
    for (vector<linear_constraint_t>::const_iterator
         i = linear_constraints->begin();
         i != linear_constraints->end(); ++i) {
      map<int,double> expr;
      for (std::vector<weighted_edge_t>::const_iterator
           j = i->first.begin(); j != i->first.end(); ++j) {
        int level = g.edgeSize() - g.getEdge(j->first);
        expr[level] = j->second;
      }
      lc.addConstraint(expr, i->second.first, i->second.second);
    }
    lc.update();
    ZddIntersection<LinearConstraints<double>,FrontierBasedSearch> zi(lc, fbs);

    dd.zddSubset(zi);
  }
  else {
    dd.zddSubset(fbs);
  }

  dd.useMultiProcessors(false);
  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

bool ShowMessages(bool flag) {
  return MessageHandler::showMessages(flag);
}

}  // namespace graphillion
