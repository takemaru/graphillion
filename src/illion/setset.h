#ifndef ILLION_SETSET_H_
#define ILLION_SETSET_H_

#include <initializer_list>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "illion/type.h"

namespace illion {

class setset_test;

class setset {
 public:
  class iterator
      : public std::iterator<std::forward_iterator_tag, std::set<elem_t> > {
   public:
    iterator();
    iterator(const setset& ss, std::vector<int> weights = std::vector<int>());
    explicit iterator(const std::set<elem_t>& s);
    iterator(const iterator& i)
        : zdd_(i.zdd_), weights_(i.weights_), s_(i.s_) {}

    void operator=(const iterator& i) {
      this->zdd_ = i.zdd_;
      this->weights_ = i.weights_;
      this->s_ = i.s_;
    }
    std::set<elem_t>& operator*() { return this->s_; }
    iterator& operator++();
    bool operator==(const iterator& i) const {
      return this->zdd_ == i.zdd_;
    }
    bool operator!=(const iterator& i) const {
      return this->zdd_ != i.zdd_;
    }

   private:
    void next();

    zdd_t zdd_;
    std::vector<int> weights_ = std::vector<int>();
    std::set<elem_t> s_ = std::set<elem_t>();

    friend class setset_test;
  };

  typedef iterator const_iterator;

  setset();
  setset(const setset& ss) : zdd_(ss.zdd_) {}
  explicit setset(const std::set<elem_t>& s);
  explicit setset(const std::vector<std::set<elem_t> >& v);
  explicit setset(const std::map<std::string, std::set<elem_t> >& m);
  explicit setset(const std::vector<std::map<std::string, std::set<elem_t> > >& v);
  explicit setset(const std::initializer_list<std::set<elem_t> >& v);
  explicit setset(std::istream& in);

  // Disable this constructor to avoid ambiguity, because compilers
  // automatically convert {{1}, {2}} to {1, 2} if it is defined.
//  explicit setset(const std::initializer_list<elem_t>& s);
  
  virtual ~setset() {}

  void operator=(const setset& ss) { this->zdd_ = ss.zdd_; }
  bool operator==(const setset& ss) { return this->zdd_ == ss.zdd_; }
  bool operator!=(const setset& ss) { return this->zdd_ != ss.zdd_; }
  setset operator~() const;
  setset operator&(const setset& ss) const;
  setset operator|(const setset& ss) const;
  setset operator-(const setset& ss) const;
  setset operator*(const setset& ss) const;
  setset operator^(const setset& ss) const;
  setset operator/(const setset& ss) const;
  setset operator%(const setset& ss) const;
  void operator&=(const setset& ss);
  void operator|=(const setset& ss);
  void operator-=(const setset& ss);
  void operator*=(const setset& ss);
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
  iterator begin(std::vector<int> weights = std::vector<int>()) const {
    return iterator(*this, weights);
  }
  static iterator end() { return iterator(); }
  iterator find(const std::set<elem_t>& s) const;
  setset find(elem_t e) const;
  setset not_find(elem_t e) const;
  size_t count(const std::set<elem_t>& s) const;
  std::pair<iterator, bool> insert(const std::set<elem_t>& s);
  iterator insert(const_iterator hint, const std::set<elem_t>& s);
  void insert(const std::initializer_list<std::set<elem_t> >& v);
  iterator erase(const_iterator position);
  size_t erase(const std::set<elem_t>& s);
  size_t erase(elem_t e);
  void clear();
  void swap(setset& ss);

  setset minimal() const;
  setset maximal() const;
  setset hitting() const;
  setset smaller(size_t max_set_size) const;

  setset subsets(const setset& ss) const;
  setset supersets(const setset& ss) const;
  setset nonsubsets(const setset& ss) const;
  setset nonsupersets(const setset& ss) const;

  void dump(std::ostream& out) const;
  void dump(FILE* fp = stdout) const;
  void load(std::istream& in);
  void load(FILE* fp = stdin);
  void _enum(std::ostream& out) const;
  void _enum(FILE* fp = stdout) const;

  friend std::ostream& operator<<(std::ostream& out, const setset& ss);
  friend std::istream& operator>>(std::istream& in, setset& ss);

 private:
  explicit setset(const zdd_t& z) : zdd_(z) {}

  zdd_t zdd_;

  friend class setset_test;
};

}  // namespace illion

#endif  // ILLION_SETSET_H_
