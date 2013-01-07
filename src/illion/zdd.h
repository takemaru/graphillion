#ifndef ILLION_ZDD_H_
#define ILLION_ZDD_H_

#include <cstdint>

#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>

#ifdef HAVE_LIBGMPXX
#include <gmpxx.h>
#endif

#include "hudd/ZBDD.h"

namespace illion {

typedef ZBDD zdd_t;
typedef bddword word_t;
typedef int32_t elem_t;  // bddvar

#ifdef HAVE_LIBGMPXX
typedef mpz_class intx_t;
#else
typedef double intx_t;
#endif

zdd_t operator|(const zdd_t& f, const zdd_t& g);

class zdd {
 protected:
  static void init(elem_t num_elems);
  static void new_elems(elem_t max_elem);

  static zdd_t single(elem_t e);
  static zdd_t null() { return zdd_t(-1); }
  static zdd_t bot() { return zdd_t(0); }
  static zdd_t top() { return zdd_t(1); }
  static zdd_t lo(zdd_t f) { return f.OffSet(f.Top()); }
  static zdd_t hi(zdd_t f) { return f.OnSet0(f.Top()); }
  static bool is_bot(zdd_t f) { return f == bot(); }
  static bool is_top(zdd_t f) { return f == top(); }
  static bool is_term(zdd_t f) { return f.Top() == 0; }
  static word_t id(zdd_t f) { return f.GetID(); }
  static elem_t elem(zdd_t f) {
    return is_term(f) ? BDD_MaxVar + 1 : f.Top();
  }

  static zdd_t _not(zdd_t f);
  static zdd_t minimal(zdd_t f);
  static zdd_t maximal(zdd_t f);
  static zdd_t hitting(zdd_t f);
  static zdd_t nonsubsets(zdd_t f, zdd_t g);
  static zdd_t nonsupersets(zdd_t f, zdd_t g);
  static zdd_t choose_random(zdd_t f, std::vector<elem_t>* stack, int* idum);
  static zdd_t choose_best(zdd_t f, const std::vector<int>& weights,
                           std::set<elem_t>* s);
  static void algo_b(zdd_t f, const std::vector<int>& w, std::vector<bool>* x);
  static intx_t algo_c(zdd_t f);
  static zdd_t zuniq(elem_t v, zdd_t l, zdd_t h);
  static double ran3(int* idum);
  static void sort_zdd(zdd_t f, std::vector<std::vector<zdd_t> >* stacks,
                       std::unordered_set<word_t>* visited);
  static std::pair<word_t, word_t> make_key(zdd_t f, zdd_t g) {
    return std::make_pair(id(f), id(g));
  }
  static void save(zdd_t f, std::ostream& out = std::cout);
  static zdd_t load(std::istream& in = std::cin);
  static void dump(zdd_t f, std::ostream& out = std::cout);
  static void dump(zdd_t f, std::vector<elem_t>* stack, std::ostream& out);

  static bool initialized_;
  static elem_t num_elems_;
};

}  // namespace illion

#endif  // ILLION_ZDD_H_
