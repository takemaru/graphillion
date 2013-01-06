#include "illion/setset.h"

#include <climits>

#include <sstream>
#include <unordered_map>

#include "illion/util.h"

namespace illion {

using std::initializer_list;
using std::map;
using std::pair;
using std::set;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::unordered_set;
using std::vector;

// class base

ZBDD zdd::node(elem_t e) {
  assert(0 < e && e <= BDD_MaxVar);
  if (!initialized_) {
    BDD_Init(1000000, 8000000000LL);
    initialized_ = true;
  }
  for (; num_elems_ < e; ++num_elems_)
    top().Change(BDD_NewVarOfLev(1));
  return top().Change(e);
}

ZBDD zdd::minimal(ZBDD f) {
  if (is_terminal(f)) return f;
  vector<vector<ZBDD> > stacks(num_elems_ + 1);
  unordered_set<word_t> visited;
  sort_zdd(f, &stacks, &visited);
  unordered_map<word_t, ZBDD> cache
    = { {node_id(bot()), bot()}, {node_id(top()), top()} };
  for (elem_t v = num_elems_; v > 0; --v) {
    while (!stacks[v].empty()) {
      ZBDD n = stacks[v].back();
      stacks[v].pop_back();
      cache[node_id(n)]
        = cache.at(node_id(lo(n)))
        + (cache.at(node_id(hi(n))) - cache.at(node_id(lo(n)))).Change(v);
    }
  }
  return cache.at(node_id(f));
}

ZBDD zdd::maximal(ZBDD f) {
  if (is_terminal(f)) return f;
  vector<vector<ZBDD> > stacks(num_elems_ + 1);
  unordered_set<word_t> visited;
  sort_zdd(f, &stacks, &visited);
  unordered_map<word_t, ZBDD> cache
    = { {node_id(bot()), bot()}, {node_id(top()), top()} };
  for (elem_t v = num_elems_; v > 0; --v) {
    while (!stacks[v].empty()) {
      ZBDD n = stacks[v].back();
      stacks[v].pop_back();
      cache[node_id(n)]
        = cache.at(node_id(lo(n)))
        - cache.at(node_id(lo(n))).Permit(cache.at(node_id(hi(n))))
        + cache.at(node_id(hi(n))).Change(v);
    }
  }
  return cache.at(node_id(f));
}

ZBDD zdd::hitting(ZBDD f) {
  if (is_bot(f)) return top();
  if (is_top(f)) return bot();
  vector<vector<ZBDD> > stacks(num_elems_ + 1);
  unordered_set<word_t> visited;
  sort_zdd(f, &stacks, &visited);
  unordered_map<word_t, ZBDD> cache
    = { {node_id(bot()), bot()}, {node_id(top()), bot()} };
  for (elem_t v = num_elems_; v > 0; --v) {
    while (!stacks[v].empty()) {
      ZBDD n = stacks[v].back();
      stacks[v].pop_back();
      ZBDD l = cache.at(node_id(lo(n)));
      if (lo(n) != bot()) {
        elem_t j = is_top(lo(n)) ? num_elems_ : var(lo(n)) - 1;
        for (; j > v; --j)
          l += l.Change(j);
      }
      ZBDD h = cache.at(node_id(hi(n)));
      if (hi(n) != bot()) {
        elem_t j = is_top(hi(n)) ? num_elems_ : var(hi(n)) - 1;
        for (; j > v; --j)
          h += h.Change(j);
      }
      if (lo(n) == bot()) {
        ZBDD g = top();
        for (elem_t j = num_elems_; j > v; --j)
          g += g.Change(j);
        g = g.Change(var(n));
        cache[node_id(n)] = h + g;
      } else {
        cache[node_id(n)] = (h & l) + l.Change(v);
      }
    }
  }
  ZBDD g = cache.at(node_id(f));
  elem_t j = is_terminal(f) ? num_elems_ : var(f) - 1;
  for (; j > 0; --j)
    g += g.Change(j);
  return g;
}

struct bdd_pair_hash {
  size_t operator()(const pair<word_t, word_t>& o) const {
    return (o.first << 4*sizeof(o.first)) ^ o.second;
  }
};

struct bdd_pair_eq {
  bool operator()(const pair<word_t,word_t>& a, const pair<word_t,word_t>& b) const {
    return a.first == b.first && a.second == b.second;
  }
};

ZBDD zdd::nonsubsets(ZBDD f, ZBDD g) {
  static unordered_map<pair<word_t, word_t>, ZBDD, bdd_pair_hash, bdd_pair_eq> cache;
  if (g == bot())
    return f;
  else if (f == bot() || f == top() || f == g)
    return bot();
  pair<word_t, word_t> k = make_key(f, g);
  auto i = cache.find(k);
  if (i != cache.end())
    return i->second;
  ZBDD rl;
  ZBDD rh;
  if (var(f) < var(g)) {
    rl = nonsubsets(lo(f), g);
    rh = hi(f);
  }
  else {
    rl = nonsubsets(lo(f), hi(g)) & nonsubsets(lo(f), lo(g));
    rh = nonsubsets(hi(f), hi(g));
  }
  return cache[k] = zuniq(var(f), rl, rh);
}

ZBDD zdd::nonsupersets(ZBDD f, ZBDD g) {
  static unordered_map<pair<word_t, word_t>, ZBDD, bdd_pair_hash, bdd_pair_eq> cache;
  if (g == bot())
    return f;
  else if (f == bot() || g == top() || f == g)
    return bot();
  else if (var(f) > var(g))
    return nonsupersets(f, lo(g));
  pair<word_t, word_t> k = make_key(f, g);
  auto i = cache.find(k);
  if (i != cache.end())
    return i->second;
  ZBDD rl;
  ZBDD rh;
  if (var(f) < var(g)) {
    rl = nonsupersets(lo(f), g);
    rh = nonsupersets(hi(f), g);
  }
  else {
    rl = nonsupersets(lo(f), lo(g));
    rh = nonsupersets(hi(f), hi(g)) & nonsupersets(hi(f), lo(g));
  }
  return cache[k] = zuniq(var(f), rl, rh);
}

ZBDD zdd::choose_random(ZBDD f, vector<elem_t>* stack, int* idum) {
  assert(stack != NULL && idum != NULL);
  if (is_terminal(f)) {
    if (is_top(f)) {
      ZBDD g = top();
      for (int i = 0; i <= static_cast<int>(stack->size()) - 1; i++)
        g *= node((*stack)[i]);
      return g;
    }
    assert(false);
  }
#ifdef HAVE_LIBGMPXX
  double ch = algo_c(hi(f)).get_d();
  double cl = algo_c(lo(f)).get_d();
#else
  double ch = algo_c(hi(f))
  double cl = algo_c(lo(f))
#endif
  if (ran3(idum) > cl / (ch + cl)) {
    stack->push_back(var(f));
    return choose_random(hi(f), stack, idum);
  } else {
    return choose_random(lo(f), stack, idum);
  }
}

ZBDD zdd::choose_best(ZBDD f, const vector<int>& weights, set<elem_t>* s) {
  assert(s != NULL);
  if (is_bot(f)) return bot();
  vector<bool> x;
  algo_b(f, weights, &x);
  ZBDD g = top();
  s->clear();
  for (elem_t j = 1; j <= num_elems_; j++) {
    if (x[j]) {
      g *= node(j);
      s->insert(j);
    }
  }
  return g;
}

// Based on Algorithm B from Knuth vol. 4 fascicle 1 sec. 7.1.4.
void zdd::algo_b(ZBDD f, const vector<int>& w, vector<bool>* x) {
  assert(w.size() > static_cast<size_t>(num_elems_));
  assert(x != NULL);
  assert(!is_bot(f));
  x->clear();
  x->resize(num_elems_ + 1, false);
  if (is_top(f)) return;

  unordered_map<word_t, bool> t;
  unordered_map<word_t, int> ms
      = {{node_id(bot()), INT_MIN}, {node_id(top()), 0}};

  vector<vector<ZBDD> > stacks(num_elems_ + 1);
  unordered_set<word_t> visited;
  sort_zdd(f, &stacks, &visited);

  for (elem_t v = num_elems_; v > 0; --v) {
    while (!stacks[v].empty()) {
      ZBDD g = stacks[v].back();
      stacks[v].pop_back();
      word_t k = node_id(g);
      elem_t v = var(g);
      word_t l = node_id(lo(g));
      word_t h = node_id(hi(g));
      if (!is_bot(lo(g)))
        ms[k] = ms.at(l);
      if (!is_bot(hi(g))) {
        int m = ms.at(h) + w[v];
        if (is_bot(lo(g)) || m > ms.at(k)) {
          ms[k] = m;
          t[k] = true;
        }
      }
    }
  }

  while (!is_terminal(f)) {
    word_t k = node_id(f);
    elem_t v = var(f);
    if (t.find(k) == t.end())
      t[k] = false;
    (*x)[v] = t.at(k);
    f = !t.at(k) ? lo(f) : hi(f);
  }
}

// Based on Algorithm C from Knuth vol. 4 fascicle 1 sec. 7.1.4 (p.75).
intx_t zdd::algo_c(ZBDD f) {
    static unordered_map<word_t, intx_t> counts;
    if (is_terminal(f))
        return is_top(f) ? 1 : 0;
    else if (counts.find(node_id(f)) != counts.end())
        return counts.at(node_id(f));
    else
        return counts[node_id(f)] = algo_c(hi(f)) + algo_c(lo(f));
}

// Knuth vol. 4 fascicle 1 sec. 7.1.4.
ZBDD zdd::zuniq(elem_t v, ZBDD l, ZBDD h) {
  return l + node(v) * h;
}

// Based on Seminumerical Algorithms from Knuth vol. 2, sec. 3.2-3.3.
#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC (1.0/MBIG)
double zdd::ran3(int* idum) {
  static int inext, inextp;
  static long ma[56];
  static int iff = 0;
  long mj, mk;
  int i, ii, k;

  if (*idum < 0 || iff == 0) {
    iff = 1;
    mj = labs(MSEED - labs(*idum));
    mj %= MBIG;
    ma[55] = mj;
    mk = 1;
    for (i = 1; i <= 54; ++i) {
      ii = (21*i) % 55;
      ma[ii] = mk;
      mk = mj - mk;
      if (mk < MZ) mk += MBIG;
      mj = ma[ii];
    }
    for (k = 1; k <= 4; ++k)
      for (i = 1; i <= 55; ++i) {
        ma[i] -= ma[1 + (i+30) % 55];
        if (ma[i] < MZ) ma[i] += MBIG;
      }
    inext = 0;
    inextp = 31;
    *idum = 1;
  }
  if (++inext == 56) inext = 1;
  if (++inextp == 56) inextp = 1;
  mj = ma[inext] - ma[inextp];
  if (mj < MZ) mj += MBIG;
  ma[inext] = mj;
  return mj * FAC;
}

void zdd::sort_zdd(ZBDD f, vector<vector<ZBDD> >* stacks,
                      unordered_set<word_t>* visited) {
  assert(stacks != nullptr && visited != nullptr);
  if (!is_terminal(f) && visited->find(node_id(f)) == visited->end()) {
    (*stacks)[var(f)].push_back(f);
    visited->insert(node_id(f));
    sort_zdd(lo(f), stacks, visited);
    sort_zdd(hi(f), stacks, visited);
  }
}

void zdd::dump(ZBDD f) {
  vector<elem_t> stack;
  printf("{");
  dump(f, &stack);
  printf("}\n");
}

void zdd::dump(ZBDD f, vector<elem_t>* stack) {
  assert(stack != nullptr);
  if (is_terminal(f)) {
    if (is_top(f))
      printf("{%s},", join(*stack, ",").c_str());
    return;
  }
  stack->push_back(var(f));
  dump(hi(f), stack);
  stack->pop_back();
  dump(lo(f), stack);
}

bool zdd::initialized_ = false;
elem_t zdd::num_elems_ = 0;

// class setset::iterator

setset::iterator::iterator(const setset& ss)
    : f_(ss.f_), weights_(ss.weights_) {
  this->next();
}

setset::iterator::iterator(const setset& ss, const set<elem_t>& s)
    : f_(ss.f_ - setset(s).f_), weights_(ss.weights_), s_(s) {
}

setset::iterator& setset::iterator::operator++() {
  this->next();
  return *this;
}

void setset::iterator::next() {
  if (this->f_ == znull() || is_bot(this->f_)) {
    this->f_ = znull();
    this->s_ = set<elem_t>();
  } else if (this->weights_.empty()) {  // random sampling
    vector<elem_t> stack;
    static int idum = -1;  // TODO: can be set by users
    this->f_ -= choose_random(this->f_, &stack, &idum);
    this->s_ = set<elem_t>(stack.begin(), stack.end());
  } else {  // optimization
    set<elem_t> s;
    this->f_ -= choose_best(this->f_, this->weights_, &s);
    this->s_ = s;
  }
}

// class setset

setset::setset(const set<elem_t>& s) : f_(top()) {
  for (const auto& e : s)
    this->f_ *= node(e);
}

setset::setset(const vector<set<elem_t> >& v) {
  for (const auto& s : v)
    this->f_ += setset(s).f_;
}

setset::setset(const map<string, set<elem_t> >& m) {
  // TODO: check map keys that are not include/exclude
  // TODO: check if common elements found in include and exclude
  const auto in_it = m.find("include");
  const auto ex_it = m.find("exclude");
  const set<elem_t>& in_s = in_it != m.end() ? in_it->second : set<elem_t>();
  const set<elem_t>& ex_s = ex_it != m.end() ? ex_it->second : set<elem_t>();
  for (const auto& e : in_s) node(e);
  for (const auto& e : ex_s) node(e);
  vector<ZBDD> nodes(num_elems_ + 2);
  nodes[0] = bot(), nodes[1] = top();
  for (elem_t v = num_elems_; v > 0; --v) {
    elem_t i = num_elems_ - v + 2;
    nodes[i] = in_s.find(v) != in_s.end() ? nodes[0]   + node(v) * nodes[i-1]
             : ex_s.find(v) != ex_s.end() ? nodes[i-1] + node(v) * nodes[0]
             :                              nodes[i-1] + node(v) * nodes[i-1];
  }
  this->f_ = nodes[num_elems_ + 1];
}

setset::setset(const vector<map<string, set<elem_t> > >& v ) {
  for (const auto& m : v)
    this->f_ += setset(m).f_;
}

setset::setset(const initializer_list<set<elem_t> >& v) {
  for (auto i = v.begin(); i != v.end(); ++i)
    this->f_ += setset(*i).f_;
}
/*
setset::setset(const initializer_list<int>& s) : f_(top()) {
  for (auto i = s.begin(); i != s.end(); ++i)
    this->f_ *= node(*i);
}
*/
setset setset::operator~() const {
  vector<ZBDD> nodes(num_elems_ + 2);
  nodes[0] = bot(), nodes[1] = top();
  for (elem_t v = num_elems_; v > 0; --v) {
    elem_t i = num_elems_ - v + 2;
    nodes[i] = nodes[i - 1] + node(v) * nodes[i - 1];
  }
  return setset(nodes[num_elems_ + 1] - this->f_);
}

setset setset::operator&(const setset& ss) const {
  return setset(this->f_ & ss.f_);
}

setset setset::operator|(const setset& ss) const {
  return setset(this->f_ + ss.f_);
}

setset setset::operator-(const setset& ss) const {
  return setset(this->f_ - ss.f_);
}

setset setset::operator*(const setset& ss) const {
  return setset(this->f_ * ss.f_);
}

setset setset::operator^(const setset& ss) const {
  return setset((this->f_ - ss.f_) + (ss.f_ - this->f_));
}

setset setset::operator/(const setset& ss) const {
  return setset(this->f_ / ss.f_);
}

setset setset::operator%(const setset& ss) const {
  return setset(this->f_ % ss.f_);
}

void setset::operator&=(const setset& ss) {
  this->f_ &= ss.f_;
}

void setset::operator|=(const setset& ss) {
  this->f_ += ss.f_;
}

void setset::operator-=(const setset& ss) {
  this->f_ -= ss.f_;
}

void setset::operator*=(const setset& ss) {
  this->f_ *= ss.f_;
}

void setset::operator^=(const setset& ss) {
  this->f_ = (this->f_ - ss.f_) + (ss.f_ - this->f_);
}

void setset::operator/=(const setset& ss) {
  this->f_ /= ss.f_;
}

void setset::operator%=(const setset& ss) {
  this->f_ %= ss.f_;
}

bool setset::operator<=(const setset& ss) const {
  return (this->f_ - ss.f_) == bot();
}

bool setset::operator<(const setset& ss) const {
  return (this->f_ - ss.f_) == bot() && this->f_ != ss.f_;
}

bool setset::operator>=(const setset& ss) const {
  return (ss.f_ - this->f_) == bot();
}

bool setset::operator>(const setset& ss) const {
  return (ss.f_ - this->f_) == bot() && this->f_ != ss.f_;
}

bool setset::isdisjoint(const setset& ss) const {
  return (this->f_ & ss.f_) == bot();
}

bool setset::issubset(const setset& ss) const {
  return *this <= ss;
}

bool setset::issuperset(const setset& ss) const {
  return *this >= ss;
}

string setset::size() const {
  stringstream ss;
  ss << algo_c(this->f_);
  return ss.str();
}

setset::iterator setset::find(const std::set<elem_t>& s) const {
  if (this->f_ / setset(s).f_ != bot())
    return setset::iterator(*this, s);
  else
    return setset::iterator();
}

size_t setset::count(const std::set<elem_t>& s) const {
  return this->f_ / setset(s).f_ != bot() ? 1 : 0;
}

setset setset::minimal() const {
  return setset(zdd::minimal(this->f_));
}

setset setset::maximal() const {
  return setset(zdd::maximal(this->f_));
}

setset setset::hitting() const {  // a.k.a cross elements
  return setset(zdd::hitting(this->f_));
}

setset setset::smaller(size_t max_set_size) const {
  return setset(this->f_.PermitSym(max_set_size));
}

setset setset::subsets(const setset& ss) const {
  return setset(this->f_.Permit(ss.f_));
}

setset setset::supersets(const setset& ss) const {
  return setset(this->f_.Restrict(ss.f_));
}

setset setset::nonsubsets(const setset& ss) const {
  return setset(zdd::nonsubsets(this->f_, ss.f_));
}

setset setset::nonsupersets(const setset& ss) const {
  return setset(zdd::nonsupersets(this->f_, ss.f_));
}

void setset::dump() const {
  zdd::dump(this->f_);
}

}  // namespace illion
