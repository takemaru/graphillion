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
    iterator(const setset& ss, const std::set<elem_t>& s);
    iterator(const iterator& i) : f_(i.f_), weights_(i.weights_), s_(i.s_) {}

    void operator=(const iterator& i) {
      this->f_ = i.f_;
      this->weights_ = i.weights_;
      this->s_ = i.s_;
    }
    std::set<elem_t>& operator*() { return this->s_; }
    iterator& operator++();
    bool operator==(const iterator& i) const {
      return this->f_ == i.f_;
    }
    bool operator!=(const iterator& i) const {
      return this->f_ != i.f_;
    }

   private:
    void next();

    zdd_t f_ = znull();  // TODO: rename to zdd_
    std::vector<int> weights_;
    std::set<elem_t> s_ = std::set<elem_t>();

    friend class setset_test;
  };

  typedef iterator const_iterator;

  setset() {}
  setset(const setset& ss) : f_(ss.f_), weights_(ss.weights_) {}
  explicit setset(const std::set<elem_t>& s);
  explicit setset(const std::vector<std::set<elem_t> >& v);
  explicit setset(const std::map<std::string, std::set<elem_t> >& m);
  explicit setset(const std::vector<std::map<std::string, std::set<elem_t> > >& v);
  explicit setset(const std::initializer_list<std::set<elem_t> >& s);

  // disable this constructor to avoid ambiguity, because compilers
  // automatically convert {{1}, {2}} to {1, 2} if it defined.
  //explicit setset(const std::initializer_list<elem_t>& s);
  
  virtual ~setset() {}

  void operator=(const setset& ss) {
    this->f_ = ss.f_;
    this->weights_ = ss.weights_;
  }
  bool operator==(const setset& ss) { return this->f_ == ss.f_; }
  bool operator!=(const setset& ss) { return this->f_ != ss.f_; }
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

  bool isdisjoint(const setset& ss) const;
  bool issubset(const setset& ss) const;
  bool issuperset(const setset& ss) const;

  bool empty() const { return this->f_ == bot(); }
  std::string size() const;
  iterator begin() const { return iterator(*this); }
  iterator end() const { return iterator(); }
  iterator find(const std::set<elem_t>& s) const;
  size_t count(const std::set<elem_t>& s) const;
  //  std::pair<iterator, bool> insert(const std::set<elem_t>& s);
  void clear() { this->f_ = bot(); }
  void swap(setset& ss) {
    zdd_t f = this->f_; this->f_ = ss.f_; ss.f_ = f;
  }

  void set_weights(const std::vector<int>& w) { this->weights_ = w; }
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
  explicit setset(const zdd_t& f) : f_(f) {}

  void dump() const;

  zdd_t f_ = bot();  // TODO: rename to zdd_
  std::vector<int> weights_;

  friend class setset_test;
};

}  // namespace illion

#endif  // ILLION_SETSET_H_
