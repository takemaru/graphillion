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

namespace graphillion {

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

    intx_t size_;
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
  random_iterator randomize() const;
  weighted_iterator minimize(const std::vector<double>& weights) const;
  weighted_iterator maximize(const std::vector<double>& weights) const;
  iterator find(const std::set<elem_t>& s) const;
  setset include(elem_t e) const;
  setset exclude(elem_t e) const;
  size_t count(const std::set<elem_t>& s) const;
  std::pair<iterator, bool> insert(const std::set<elem_t>& s);
  iterator insert(const_iterator hint, const std::set<elem_t>& s);
  void insert(elem_t e);
  iterator erase(const_iterator position);
  size_t erase(const std::set<elem_t>& s);
  void erase(elem_t e);
  void clear();
  void swap(setset& ss);

  setset minimal() const;
  setset maximal() const;
  setset hitting() const;
  setset smaller(size_t set_size) const;
  setset larger(size_t set_size) const;
  setset same_size(size_t set_size) const;

  setset flip(elem_t e) const;
  setset flip() const;
  setset join(const setset& ss) const;
  setset meet(const setset& ss) const;
  setset subsets(const setset& ss) const;
  setset supersets(const setset& ss) const;
  setset non_subsets(const setset& ss) const;
  setset non_supersets(const setset& ss) const;

  void dump(std::ostream& out) const;
  void dump(FILE* fp = stdout) const;
  void load(std::istream& in);
  void load(FILE* fp = stdin);
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
  static elem_t num_elems();
  static void num_elems(elem_t num_elems);

  friend std::ostream& operator<<(std::ostream& out, const setset& ss);
  friend std::istream& operator>>(std::istream& in, setset& ss);

 private:
  explicit setset(const std::set<elem_t>& s);
  explicit setset(const zdd_t& z) : zdd_(z) {}

  zdd_t zdd_;

  friend class TestSetset;
};

}  // namespace graphillion

#endif  // GRAPHILLION_SETSET_H_
