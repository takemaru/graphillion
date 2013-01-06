#ifndef ILLION_SETSET_H_
#define ILLION_SETSET_H_

#include <initializer_list>
#include <map>
#include <string>

#include "illion/zdd.h"

namespace illion {

class setset_test;

class setset : public zdd {
 public:
  class iterator
      : public std::iterator<std::forward_iterator_tag, std::set<elem_t> > {
   public:
    iterator() {}
    explicit iterator(const setset& ss);
    iterator(const std::set<elem_t>& s) : zdd_(bot()), s_(s) {}
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

    zdd_t zdd_ = null();
    std::vector<int> weights_;
    std::set<elem_t> s_ = std::set<elem_t>();

    friend class setset_test;
  };

  typedef iterator const_iterator;

  setset() {}
  setset(const setset& ss) : zdd_(ss.zdd_), weights_(ss.weights_) {}
  explicit setset(const std::set<elem_t>& s);
  explicit setset(const std::vector<std::set<elem_t> >& v);
  explicit setset(const std::map<std::string, std::set<elem_t> >& m);
  explicit setset(const std::vector<std::map<std::string, std::set<elem_t> > >& v);
  explicit setset(const std::initializer_list<std::set<elem_t> >& v);

  /* Disable this constructor to avoid ambiguity, because compilers
   * automatically convert {{1}, {2}} to {1, 2} if it defined. */
  //explicit setset(const std::initializer_list<elem_t>& s);
  
  virtual ~setset() {}

  void operator=(const setset& ss) {
    this->zdd_ = ss.zdd_;
    this->weights_ = ss.weights_;
  }
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

  bool is_disjoint(const setset& ss) const;
  bool is_subset(const setset& ss) const;
  bool is_superset(const setset& ss) const;

  bool empty() const { return this->zdd_ == bot(); }
  std::string size() const;
  iterator begin() const { return iterator(*this); }
  static iterator end() { return iterator(); }
  iterator find(const std::set<elem_t>& s) const;
  setset find(elem_t e) const;
  size_t count(const std::set<elem_t>& s) const;
  std::pair<iterator, bool> insert(const std::set<elem_t>& s);
  iterator insert(const_iterator hint, const std::set<elem_t>& s);
  void insert(const std::initializer_list<std::set<elem_t> >& v);
  iterator erase(const_iterator position);
  size_t erase(const std::set<elem_t>& s);
  size_t erase(elem_t e);
  void clear() { this->zdd_ = bot(); }
  void swap(setset& ss) {
    zdd_t z = this->zdd_; this->zdd_ = ss.zdd_; ss.zdd_ = z;
  }

  void set_weights(const std::vector<int>& w) { this->weights_ = w; }
  void clear_weights() { this->weights_ = std::vector<int>(); }
  const std::vector<int>& weights() const { return this->weights_; }

  setset minimal() const;
  setset maximal() const;
  setset hitting() const;
  setset smaller(size_t max_set_size) const;

  setset subsets(const setset& ss) const;
  setset supersets(const setset& ss) const;
  setset nonsubsets(const setset& ss) const;
  setset nonsupersets(const setset& ss) const;

 private:
  explicit setset(const zdd_t& z) : zdd_(z) {}

  void dump() const;

  zdd_t zdd_ = bot();
  std::vector<int> weights_;

  friend class setset_test;
};

}  // namespace illion

#endif  // ILLION_SETSET_H_
