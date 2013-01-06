#ifndef ILLION_SETSET_H_
#define ILLION_SETSET_H_

#include <cassert>
#include <cstdint>

#include <initializer_list>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#ifdef HAVE_LIBGMPXX
#include <gmpxx.h>
#endif

#include "hudd/ZBDD.h"

namespace illion {

typedef uint64_t word_t;  // bddword
typedef int32_t elem_t;  // bddvar

#ifdef HAVE_LIBGMPXX
typedef mpz_class intx_t;
#else
typedef double intx_t; // TODO: rename it since double isn't integer?
#endif

class setset_test;

class setset {
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

    ZBDD f_ = znull();  // TODO: rename to zdd_
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
    ZBDD f = this->f_; this->f_ = ss.f_; ss.f_ = f;
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
  explicit setset(const ZBDD& f) : f_(f) {}

  void dump() const;

  // algorithms
  static void do_dump(ZBDD f, std::vector<elem_t>* stack);
  static ZBDD do_minimal(ZBDD f);
  static ZBDD do_maximal(ZBDD f);
  static ZBDD do_hitting(ZBDD f);
  static ZBDD do_nonsubsets(ZBDD f, ZBDD g);
  static ZBDD do_nonsupersets(ZBDD f, ZBDD g);
  static ZBDD choose_randomly(ZBDD f, std::vector<elem_t>* stack, int* idum);
  static ZBDD choose_best(ZBDD f, const std::vector<int>& weights,
                          std::set<elem_t>* s);
  static void algorithm_b(ZBDD f, const std::vector<int>& w,
                          std::vector<bool>* x);
  static intx_t algorithm_c(ZBDD f);
  static ZBDD zuniq(elem_t v, ZBDD l, ZBDD h);
  static double ran3(int* idum);
  static void sort_zdd(ZBDD f, std::vector<std::vector<ZBDD> >* stacks,
                       std::unordered_set<word_t>* visited);
  static std::pair<word_t, word_t> make_key(ZBDD f, ZBDD g) {
    return std::make_pair(node_id(f), node_id(g));
  }

  // ZDD helpers
  static ZBDD node(elem_t e);  // TODO: rename to elem
  static ZBDD znull() { return ZBDD(-1); }
  static ZBDD bot() { return ZBDD(0); }
  static ZBDD top() { return ZBDD(1); }
  static bool is_bot(ZBDD f) { return f == bot(); }
  static bool is_top(ZBDD f) { return f == top(); }
  static bool is_terminal(ZBDD f) { return f.Top() == 0; }
  static ZBDD lo(ZBDD f) { return f.OffSet(f.Top()); }
  static ZBDD hi(ZBDD f) { return f.OnSet0(f.Top()); }
  static word_t node_id(ZBDD f) { return f.GetID(); }
  static elem_t var(ZBDD f) {  // TODO: rename to min_elem or min?
    return is_terminal(f) ? BDD_MaxVar + 1 : f.Top();
  }

  ZBDD f_ = bot();  // TODO: rename to zdd_
  std::vector<int> weights_;

  static bool initialized_;
  static elem_t num_elems_;

  friend class setset_test;
};

}  // namespace illion

#endif  // ILLION_SETSET_H_
