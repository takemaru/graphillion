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

#ifndef GRAPHILLION_SETSET_H_
#define GRAPHILLION_SETSET_H_

#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "graphillion/type.h"

#include "SAPPOROBDD/BDDCT.h"
#include "subsetting/DdStructure.hpp"
#include "subsetting/util/Graph.hpp"
#include "graphillion/variable_converter/variable_converter.h"

namespace graphillion {

class Range;

class setset {
 public:
  class iterator
      : public std::iterator<std::forward_iterator_tag, std::set<elem_t> > {
   public:
    iterator();
    iterator(const iterator& i);
    explicit iterator(const setset& ss);
    explicit iterator(const setset& ss, const std::set<elem_t>& s);

    virtual ~iterator() {}

    bool operator==(const iterator& i) const { return this->zdd_ == i.zdd_; }
    bool operator!=(const iterator& i) const { return this->zdd_ != i.zdd_; }
    std::set<elem_t>& operator*() { return this->s_; }
    iterator& operator++();

   protected:
    virtual void next();

    zdd_t zdd_;
    std::set<elem_t> s_;
  };

  typedef iterator const_iterator;

  class random_iterator : public iterator {
   public:
    random_iterator();
    random_iterator(const random_iterator& i);
    explicit random_iterator(const setset& ss);

    virtual ~random_iterator() {}

   protected:
    virtual void next();

    double size_;
  };

  class weighted_iterator : public iterator {
   public:
    weighted_iterator();
    weighted_iterator(const weighted_iterator& i);
    explicit weighted_iterator(const setset& ss, std::vector<double> weights);

    virtual ~weighted_iterator() {}

   protected:
    virtual void next();

    std::vector<double> weights_;
  };

  setset();
  setset(const setset& ss) : zdd_(ss.zdd_) {}
  explicit setset(const std::vector<std::set<elem_t> >& v);
  explicit setset(const std::map<std::string, std::vector<elem_t> >& m);
  explicit setset(std::istream& in);

  virtual ~setset() {}

  void operator=(const setset& ss) { this->zdd_ = ss.zdd_; }
  bool operator==(const setset& ss) { return this->zdd_ == ss.zdd_; }
  bool operator!=(const setset& ss) { return this->zdd_ != ss.zdd_; }
  setset operator~() const;
  setset operator&(const setset& ss) const;
  setset operator|(const setset& ss) const;
  setset operator-(const setset& ss) const;
  setset operator^(const setset& ss) const;
  setset operator/(const setset& ss) const;
  setset operator%(const setset& ss) const;
  void operator|=(const setset& ss);
  void operator&=(const setset& ss);
  void operator-=(const setset& ss);
  void operator^=(const setset& ss);
  void operator/=(const setset& ss);
  void operator%=(const setset& ss);
  bool operator<=(const setset& ss) const;
  bool operator<(const setset& ss) const;
  bool operator>=(const setset& ss) const;
  bool operator>(const setset& ss) const;

  word_t id() const;

  bool is_disjoint(const setset& ss) const;
  bool is_subset(const setset& ss) const;
  bool is_superset(const setset& ss) const;

  bool empty() const;
  std::string size() const;
  iterator begin() const;
  random_iterator begin_randomly() const;
  weighted_iterator begin_from_min(const std::vector<double>& weights) const;
  weighted_iterator begin_from_max(const std::vector<double>& weights) const;
  iterator find(const std::set<elem_t>& s) const;
  size_t count(const std::set<elem_t>& s) const;
  std::pair<iterator, bool> insert(const std::set<elem_t>& s);
  iterator insert(const_iterator hint, const std::set<elem_t>& s);
  void insert(elem_t e);
  iterator erase(const_iterator position);
  size_t erase(const std::set<elem_t>& s);
  void erase(elem_t e);
  void clear();
  void swap(setset& ss);
  void flip(elem_t e);
  void flip();

  setset minimal() const;
  setset maximal() const;
  setset hitting() const;
  setset smaller(size_t set_size) const;
  setset larger(size_t set_size) const;
  setset set_size(size_t set_size) const;

  setset join(const setset& ss) const;
  setset meet(const setset& ss) const;
  setset subsets(const setset& ss) const;
  setset supersets(const setset& ss) const;
  setset supersets(elem_t e) const;
  setset non_subsets(const setset& ss) const;
  setset non_supersets(const setset& ss) const;
  setset non_supersets(elem_t e) const;
  setset cost_le(const std::vector<bddcost>& costs, const bddcost cost_bound) const;
  setset remove_some_element() const;
  setset add_some_element(int n, int lower) const;
  setset remove_add_some_elements(int n, int lower) const;

  setset to_vertexsetset_setset(const std::vector<std::vector<std::string>> &edges_from_top) const;

  double probability(const std::vector<double>& probabilities) const;

  void dump(std::ostream& out) const;
  void dump(FILE* fp = stdout) const;
  static setset load(std::istream& in);
  static setset load(FILE* fp = stdin);
  void _enum(std::ostream& out,
             const std::pair<const char*, const char*> outer_braces
               = std::make_pair("{", "}"),
             const std::pair<const char*, const char*> inner_braces
               = std::make_pair("{", "}")) const;
  void _enum(FILE* fp = stdout,
             const std::pair<const char*, const char*> outer_braces
               = std::make_pair("{", "}"),
             const std::pair<const char*, const char*> inner_braces
               = std::make_pair("{", "}")) const;

  static iterator end() { return iterator(); }
  static elem_t elem_limit();
  static elem_t max_elem();
  static elem_t num_elems();
  static void num_elems(elem_t num_elems);

  friend std::ostream& operator<<(std::ostream& out, const setset& ss);
  friend std::istream& operator>>(std::istream& in, setset& ss);

 private:
  explicit setset(const std::set<elem_t>& s);
  explicit setset(const zdd_t& z) : zdd_(z) {}

  zdd_t zdd_;

  friend class TestSetset;
  friend setset SearchGraphs(
      const std::vector<edge_t>& graph,
      const std::vector<std::vector<vertex_t> >* vertex_groups,
      const std::map<vertex_t, Range>* degree_constraints,
      const Range* num_edges,
      int num_comps,
      bool no_loop,
      const setset* search_space,
      const std::vector<linear_constraint_t>* linear_constraints);
  friend setset SearchRegularGraphs(
      const std::vector<edge_t> &edges,
      const int degree_lower,
      const int degree_upper,
      const bool is_connected,
      const setset* search_space);
  friend setset SearchPartitions(const std::vector<graphillion::edge_t>& edges,
                                 int16_t comp_lb, int16_t comp_ub);
  friend setset SearchRatioPartitions(
      const std::vector<edge_t>& edges,
      const std::map<std::string, uint32_t>& weight_list, double ratio,
      uint32_t lower, uint32_t upper, int k);
  friend setset SearchWeightPartitions(
      const std::vector<edge_t>& edges,
      const std::map<std::string, uint32_t>& weight_list, uint32_t lower,
      uint32_t upper, int k);
  friend setset SearchBalancedPartitions(
      const std::vector<edge_t>& edges,
      const std::map<std::string, uint32_t>& weight_list, double ratio,
      uint32_t lower, uint32_t upper, int k);
  friend setset SearchInducedGraphs(const std::vector<edge_t>& edges);
  friend setset SearchWeightedInducedGraphs(
      const std::vector<edge_t> &edges,
      const std::map<std::string, uint32_t> &weight_list, uint32_t lower,
      uint32_t upper);
  friend setset SearchChordals(const std::vector<edge_t> &edges, const uint32_t k);
  friend setset SearchForbiddenInducedSubgraphs(const std::vector<edge_t> &edges, setset* ss);
  friend setset SearchOddEdgeSubgraphs(const std::vector<edge_t>& edges);
  friend setset SearchDegreeDistributionGraphs(const std::vector<edge_t> &edges,
                                const std::vector<int>& degRanges,
                                const bool is_connected,
                                const setset* search_space);
};

}  // namespace graphillion

#endif  // GRAPHILLION_SETSET_H_
