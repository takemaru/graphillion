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

#include "subsetting/dd/ZddStructure.hpp"
#include "subsetting/eval/Cardinality.hpp"
#include "subsetting/eval/ToZBDD.hpp"
#include "subsetting/spec/DegreeConstraint.hpp"
#include "subsetting/spec/FrontierBasedSearch.hpp"
#include "subsetting/spec/SapporoZdd.hpp"
#include "subsetting/spec/SizeConstraint.hpp"
#include "subsetting/util/Graph.hpp"

#include "graphillion/setset.h"

namespace graphillion {

using std::map;
using std::set;
using std::vector;

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

struct ToZBDD2 : public DdEval<ToZBDD, ZBDD> {
  ToZBDD2(int num_elems) : num_elems_(num_elems) {}

  void evalTerminal(ZBDD& f, bool one) {
    f = ZBDD(one ? 1 : 0);
  }

  void evalNode(ZBDD& f, int level, ZBDD& f0, int i0, ZBDD& f1, int i1) {
    while (BDD_VarUsed() < level)
      BDD_NewVar();
    f = f0 + f1.Change(this->num_elems_ - level + 1);
  }

  int num_elems_;
};

class SapporoZdd2 : public ScalarDdSpec<SapporoZdd, ZBDD> {
 public:
  SapporoZdd2(ZBDD const& f, int num_elems)
      : root(f), num_elems_(num_elems) {
  }

  int getRoot(ZBDD& f) const {
    f = root;
    return f == 1 ?  -1 : BDD_LevOfVar(f.Top());
  }

  int getChild(ZBDD& f, int level, bool take) const {
//    f = take ? f.OnSet0(BDD_VarOfLev(level)) : f.OffSet(BDD_VarOfLev(level));
    f = take ? f.OnSet0(this->num_elems_ - level + 1) : f.OffSet(this->num_elems_ - level + 1);
//    assert(BDD_LevOfVar(f.Top()) < level);
    assert(level > this->num_elems_ - f.Top() + 1);
//    return f == 1 ?  -1 : BDD_LevOfVar(f.Top());
    return f == 1 ?  -1 : this->num_elems_ - f.Top() + 1;
  }

  size_t hashCode(ZBDD const& f) const {
    return const_cast<ZBDD*>(&f)->GetID();
  }

 private:
  ZBDD root;
  int num_elems_;
};

setset FrontierSearch(
    const vector<edge_t>& graph,
    const vector<vector<vertex_t> >* vertex_groups,
    const map<vertex_t, Range>* degree_constraints,
    const Range* num_edges,
    int num_comps,
    bool no_loop,
    const setset* search_space) {
  assert(static_cast<size_t>(setset::num_elems()) == graph.size());

  Graph g;
  for (vector<edge_t>::const_iterator e = graph.begin(); e != graph.end(); ++e)
    g.addEdge(e->first, e->second);
  g.update();
  assert(static_cast<size_t>(g.edgeSize()) == graph.size());

  ZddStructure* dd = NULL;
  if (search_space != NULL) {
    SapporoZdd2 f(search_space->zdd_, setset::num_elems());
    dd = new ZddStructure(f);
  } else {
    dd = new ZddStructure(g.edgeSize());
  }

  if (vertex_groups != NULL) {
    int color = 0;
    for (vector<vector<vertex_t> >::const_iterator i = vertex_groups->begin();
         i != vertex_groups->end(); ++i, ++color)
      for (vector<vertex_t>::const_iterator v = i->begin(); v != i->end(); ++v)
        g.setColor(*v, color);
    g.update();
  }

  if (degree_constraints != NULL) {
    DegreeConstraint dc(g);
    for (map<vertex_t, Range>::const_iterator i = degree_constraints->begin();
         i != degree_constraints->end(); ++i)
      dc.setConstraint(i->first, &i->second);
    dd->subset(dc);
  }

  if (num_edges != NULL) {
    SizeConstraint sc(g.edgeSize(), num_edges);
    dd->subset(sc);
  }

  FrontierBasedSearch fbs(g, num_comps, no_loop);
  dd->subset(fbs);

  zdd_t f = dd->evaluate(ToZBDD2(setset::num_elems()));
  delete dd;
  return setset(f);
}

}  // namespace graphillion
