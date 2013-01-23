#include "illion/zdd.h"

//#include <cassert>
#include <cinttypes>
#include <climits>

#include <string>
#include <unordered_map>

namespace illion {

using std::endl;
using std::getline;
using std::istream;
using std::ostream;
using std::pair;
using std::set;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

static const string WORD_FMT = sizeof(word_t) == 8 ? ("%" PRId64) : ("%" PRId32);

static bool initialized_ = false;

// number of elements that can be activated in the ZDD package
static elem_t max_elem_ = 0;

// size of universe, which must not be larger than max_elem_
static elem_t num_elems_ = 0;

ZBDD operator|(const ZBDD& f, const ZBDD& g) {
  return f + g;
}

void init() {
  if (initialized_) return;
  BDD_Init(1000000, 8000000000LL);
  initialized_ = true;
}

void new_elems(elem_t max_elem) {
  assert(max_elem <= BDD_MaxVar);
  if (!initialized_) init();
  if (num_elems_ < max_elem) num_elems_ = max_elem;
  while (max_elem_ < max_elem) {
    top().Change(BDD_NewVarOfLev(1));
    num_elems_ = ++max_elem_;
  }
  assert(num_elems_ <= max_elem_);
}

elem_t num_elems() {
  assert(num_elems_ <= max_elem_);
  return num_elems_;
}

void num_elems(elem_t num_elems) {
  new_elems(num_elems);
  num_elems_ = num_elems;
  assert(num_elems_ <= max_elem_);
}

zdd_t single(elem_t e) {
  assert(e > 0);
  new_elems(e);
  return top().Change(e);
}

zdd_t complement(zdd_t f) {
  vector<zdd_t> n(num_elems_ + 2);
  n[0] = bot(), n[1] = top();
  for (elem_t v = num_elems_; v > 0; --v) {
    elem_t i = num_elems_ - v + 2;
    n[i] = n[i - 1] + single(v) * n[i - 1];
  }
  return n[num_elems_ + 1] - f;
}

zdd_t minimal(zdd_t f) {
  static unordered_map<word_t, zdd_t> cache;
  if (is_term(f)) return f;
  auto i = cache.find(id(f));
  if (i != cache.end())
    return i->second;
  zdd_t rl = minimal(lo(f));
  zdd_t r = minimal(hi(f));
  zdd_t rh = nonsupersets(r, rl);
  r = zuniq(elem(f), rl, rh);
  return cache[id(f)] = r;
}

zdd_t maximal(zdd_t f) {
  static unordered_map<word_t, zdd_t> cache;
  if (is_term(f)) return f;
  auto i = cache.find(id(f));
  if (i != cache.end())
    return i->second;
  zdd_t r = maximal(lo(f));
  zdd_t rh = maximal(hi(f));
  zdd_t rl = nonsubsets(r, rh);
  r = zuniq(elem(f), rl, rh);
  return cache[id(f)] = r;
}

zdd_t hitting(zdd_t f) {
  if (is_bot(f)) return top();
  if (is_top(f)) return bot();
  vector<vector<zdd_t> > stacks(num_elems_ + 1);
  unordered_set<word_t> visited;
  sort_zdd(f, &stacks, &visited);
  unordered_map<word_t, zdd_t> cache
    = { {id(bot()), bot()}, {id(top()), bot()} };
  for (elem_t v = num_elems_; v > 0; --v) {
    while (!stacks[v].empty()) {
      zdd_t n = stacks[v].back();
      stacks[v].pop_back();
      zdd_t l = cache.at(id(lo(n)));
      if (!is_bot(lo(n))) {
        elem_t j = is_top(lo(n)) ? num_elems_ : elem(lo(n)) - 1;
        for (; j > v; --j)
          l = l + l.Change(j);
      }
      zdd_t h = cache.at(id(hi(n)));
      if (!is_bot(hi(n))) {
        elem_t j = is_top(hi(n)) ? num_elems_ : elem(hi(n)) - 1;
        for (; j > v; --j)
          h = h + h.Change(j);
      }
      if (is_bot(lo(n))) {
        zdd_t g = top();
        for (elem_t j = num_elems_; j > v; --j)
          g = g + g.Change(j);
        g = g.Change(elem(n));
        cache[id(n)] = h + g;
      } else {
        cache[id(n)] = (h & l) + l.Change(v);
      }
    }
  }
  zdd_t g = cache.at(id(f));
  elem_t j = is_term(f) ? num_elems_ : elem(f) - 1;
  for (; j > 0; --j)
    g = g + g.Change(j);
  return g;
}

zdd_t join(zdd_t f, zdd_t g) {
  return f * g;
}

zdd_t meet(zdd_t f, zdd_t g) {
  return ZBDD_Meet(f, g);
}

struct bdd_pair_hash {
  size_t operator()(const pair<word_t, word_t>& o) const {
    return (o.first << 4*sizeof(o.first)) ^ o.second;
  }
};

struct bdd_pair_eq {
  bool operator()(const pair<word_t,word_t>& a, const pair<word_t, word_t>& b) const {
    return a.first == b.first && a.second == b.second;
  }
};

zdd_t nonsubsets(zdd_t f, zdd_t g) {
  static unordered_map<pair<word_t, word_t>, zdd_t, bdd_pair_hash, bdd_pair_eq> cache;
  if (is_bot(g))
    return f;
  else if (is_top(g))
    return f - top();
  else if (is_bot(f) || is_top(f) || f == g)
    return bot();
  pair<word_t, word_t> k = make_key(f, g);
  auto i = cache.find(k);
  if (i != cache.end())
    return i->second;
  zdd_t r, r2, rl, rh;
  if (elem(f) < elem(g)) {
    rl = nonsubsets(lo(f), g);
    rh = hi(f);
    r = zuniq(elem(f), rl, rh);
  } else if (elem(f) == elem(g)) {
//    r = nonsubsets(lo(f), lo(g));
//    r2 = nonsubsets(lo(f), hi(g));
//    rl = r & r2;
    r2 = lo(g) | hi(g);
    rl = nonsubsets(lo(f), r2);
    rh = nonsubsets(hi(f), hi(g));
    r = zuniq(elem(f), rl, rh);
  } else {
    r2 = lo(g) | hi(g);
    r = nonsubsets(f, r2);
  }
  return cache[k] = r;
}

zdd_t nonsupersets(zdd_t f, zdd_t g) {
  static unordered_map<pair<word_t, word_t>, zdd_t, bdd_pair_hash, bdd_pair_eq> cache;
  if (is_bot(g))
    return f;
  else if (is_bot(f) || is_top(g) || f == g)
    return bot();
  else if (is_top(f))
    return top();
  else if (elem(f) > elem(g))
    return nonsupersets(f, lo(g));
  pair<word_t, word_t> k = make_key(f, g);
  auto i = cache.find(k);
  if (i != cache.end())
    return i->second;
  elem_t v = elem(f);
  zdd_t r;
  zdd_t rl;
  zdd_t rh;
  if (elem(f) < elem(g)) {
    rl = nonsupersets(lo(f), g);
    rh = nonsupersets(hi(f), g);
  } else {
    rl = nonsupersets(hi(f), hi(g));
    r = nonsupersets(hi(f), lo(g));
    rh = r & rl;
    rl = nonsupersets(lo(f), lo(g));
  }
  r = zuniq(v, rl, rh);
  return cache[k] = r;
}

zdd_t choose_random(zdd_t f, vector<elem_t>* stack, int* idum) {
  assert(stack != nullptr && idum != nullptr);
  if (is_term(f)) {
    if (is_top(f)) {
      zdd_t g = top();
      for (int i = 0; i <= static_cast<int>(stack->size()) - 1; i++)
        g = g * single((*stack)[i]);
      return g;
    }
    assert(false);
  }
#ifdef HAVE_LIBGMPXX
  double ch = algo_c(hi(f)).get_d();
  double cl = algo_c(lo(f)).get_d();
#else
  double ch = algo_c(hi(f));
  double cl = algo_c(lo(f));
#endif
  if (ran3(idum) > cl / (ch + cl)) {
    stack->push_back(elem(f));
    return choose_random(hi(f), stack, idum);
  } else {
    return choose_random(lo(f), stack, idum);
  }
}

zdd_t choose_best(zdd_t f, const vector<int>& weights, set<elem_t>* s) {
  assert(s != nullptr);
  if (is_bot(f)) return bot();
  vector<bool> x;
  algo_b(f, weights, &x);
  zdd_t g = top();
  s->clear();
  for (elem_t j = 1; j < static_cast<elem_t>(x.size()); j++) {
    if (x[j]) {
      g = g * single(j);
      s->insert(j);
    }
  }
  return g;
}

void dump(zdd_t f, ostream& out) {
  if (is_bot(f)) {
    out << "B" << endl;
  } else if (is_top(f)) {
    out << "T" << endl;
  } else {
    vector<vector<zdd_t> > stacks(num_elems_ + 1);
    unordered_set<word_t> visited;
    sort_zdd(f, &stacks, &visited);
    for (elem_t v = num_elems_; v > 0; --v) {
      while (!stacks[v].empty()) {
        zdd_t g = stacks[v].back();
        stacks[v].pop_back();
        zdd_t l = lo(g);
        zdd_t h = hi(g);
        out << id(g) << " " << elem(g) << " ";
        if      (is_bot(l)) out << "B";
        else if (is_top(l)) out << "T";
        else                out << id(l);
        out << " ";
        if      (is_bot(h)) out << "B";
        else if (is_top(h)) out << "T";
        else                out << id(h);
        out << endl;
      }
    }
  }
  out << "." << endl;
}

void dump(zdd_t f, FILE* fp) {
  if (is_bot(f)) {
    fprintf(fp, "B\n");
  } else if (is_top(f)) {
    fprintf(fp, "T\n");
  } else {
    vector<vector<zdd_t> > stacks(num_elems_ + 1);
    unordered_set<word_t> visited;
    sort_zdd(f, &stacks, &visited);
    for (elem_t v = num_elems_; v > 0; --v) {
      while (!stacks[v].empty()) {
        zdd_t g = stacks[v].back();
        stacks[v].pop_back();
        zdd_t l = lo(g);
        zdd_t h = hi(g);
        fprintf(fp, (WORD_FMT +" %d ").c_str(), id(g), elem(g));
        if      (is_bot(l)) fprintf(fp, "B");
        else if (is_top(l)) fprintf(fp, "T");
        else                fprintf(fp, WORD_FMT.c_str(), id(l));
        fprintf(fp, " ");
        if      (is_bot(h)) fprintf(fp, "B");
        else if (is_top(h)) fprintf(fp, "T");
        else                fprintf(fp, WORD_FMT.c_str(), id(h));
        fprintf(fp, "\n");
      }
    }
  }
  fprintf(fp, ".\n");
}

zdd_t load(istream& in) {
  string line;
  getline(in, line);
  if      (line == "B") return bot();
  else if (line == "T") return top();
  unordered_map<word_t, zdd_t> n = {{id(bot()), bot()}, {id(top()), top()}};
  zdd_t root;
  do {
    if (line.empty() || is_space(line)) continue;  // skip preceding empty lines
    if (line[0] == '.') break;
    word_t k;
    elem_t v;
    char sl[256], sh[256];
    int num = sscanf(line.c_str(), (WORD_FMT + " %d %s %s").c_str(),
                     &k, &v, &sl, &sh);
    if (num != 4) goto error;
    word_t l = strcmp(sl, "B") == 0 ? id(bot())
             : strcmp(sl, "T") == 0 ? id(top())
             :                        strtoll(sl, nullptr, 0);
    word_t h = strcmp(sh, "B") == 0 ? id(bot())
             : strcmp(sh, "T") == 0 ? id(top())
             :                        strtoll(sh, nullptr, 0);
    if (l == LLONG_MAX || h == LLONG_MAX) goto error;
    n[k] = root = n.at(l) + single(v) * n.at(h);
  } while (getline(in, line));
  return root;
error:
  in.setstate(in.badbit);
  return null();
}

zdd_t load(FILE* fp) {
  char buf[256];
  if (fgets(buf, sizeof(buf), fp) == NULL) return null();
  string line = buf;
  if      (line == "B") return bot();
  else if (line == "T") return top();
  unordered_map<word_t, zdd_t> n = {{id(bot()), bot()}, {id(top()), top()}};
  zdd_t root;
  do {
    line = string(buf);
    if (line.empty() || is_space(line)) continue;  // skip preceding empty lines
    if (line[0] == '.') break;
    word_t k;
    elem_t v;
    char sl[256], sh[256];
    int num = sscanf(line.c_str(), (WORD_FMT + " %d %s %s").c_str(),
                     &k, &v, &sl, &sh);
    if (num != 4) return null();
    word_t l = strcmp(sl, "B") == 0 ? id(bot())
             : strcmp(sl, "T") == 0 ? id(top())
             :                        strtoll(sl, nullptr, 0);
    word_t h = strcmp(sh, "B") == 0 ? id(bot())
             : strcmp(sh, "T") == 0 ? id(top())
             :                        strtoll(sh, nullptr, 0);
    if (l == LLONG_MAX || h == LLONG_MAX) return null();
    n[k] = root = n.at(l) + single(v) * n.at(h);
  } while (fgets(buf, sizeof(buf), fp) != NULL);
  return root;
}

void _enum(zdd_t f, ostream& out,
           const pair<const char*, const char*>& outer_braces,
           const pair<const char*, const char*>& inner_braces) {
  vector<elem_t> stack;
  out << outer_braces.first;
  bool first = true;
  _enum(f, out, &stack, &first, inner_braces);
  out << outer_braces.second;
  if (out == std::cout || out == std::cerr)
    out << endl;
}

void _enum(zdd_t f, FILE* fp,
           const pair<const char*, const char*>& outer_braces,
           const pair<const char*, const char*>& inner_braces) {
  vector<elem_t> stack;
  fprintf(fp, outer_braces.first);
  bool first = true;
  _enum(f, fp, &stack, &first, inner_braces);
  fprintf(fp, outer_braces.second);
  if (fp == stdout || fp == stderr)
    fprintf(fp, "\n");
}

void _enum(zdd_t f, ostream& out, vector<elem_t>* stack, bool* first,
           const pair<const char*, const char*>& inner_braces) {
  assert(stack != nullptr);
  if (is_term(f)) {
    if (is_top(f)) {
      if (*first)
        *first = false;
      else
        out << ", ";
      out << inner_braces.first << join(*stack, ", ") << inner_braces.second;
    }
    return;
  }
  stack->push_back(elem(f));
  _enum(hi(f), out, stack, first, inner_braces);
  stack->pop_back();
  _enum(lo(f), out, stack, first, inner_braces);
}

void _enum(zdd_t f, FILE* fp, vector<elem_t>* stack, bool* first,
           const pair<const char*, const char*>& inner_braces) {
  assert(stack != nullptr);
  if (is_term(f)) {
    if (is_top(f)) {
      if (*first)
        *first = false;
      else
        fprintf(fp, ", ");
      fprintf(fp, "%s%s%s", inner_braces.first, join(*stack, ", ").c_str(),
              inner_braces.second);
    }
    return;
  }
  stack->push_back(elem(f));
  _enum(hi(f), fp, stack, first, inner_braces);
  stack->pop_back();
  _enum(lo(f), fp, stack, first, inner_braces);
}

// Algorithm B modified for ZDD, from Knuth vol. 4 fascicle 1 sec. 7.1.4.
void algo_b(zdd_t f, const vector<int>& w, vector<bool>* x) {
  assert(x != nullptr);
  assert(!is_bot(f));
  if (is_top(f)) return;
  vector<vector<zdd_t> > stacks(num_elems_ + 1);
  unordered_set<word_t> visited;
  elem_t max_elem = 0;
  sort_zdd(f, &stacks, &visited, &max_elem);
  assert(w.size() > static_cast<size_t>(max_elem));
  x->clear();
  x->resize(max_elem + 1, false);
  unordered_map<word_t, bool> t;
  unordered_map<word_t, int> ms = {{id(bot()), INT_MIN}, {id(top()), 0}};
  for (elem_t v = max_elem; v > 0; --v) {
    while (!stacks[v].empty()) {
      zdd_t g = stacks[v].back();
      stacks[v].pop_back();
      word_t k = id(g);
      elem_t v = elem(g);
      word_t l = id(lo(g));
      word_t h = id(hi(g));
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
  while (!is_term(f)) {
    word_t k = id(f);
    elem_t v = elem(f);
    if (t.find(k) == t.end())
      t[k] = false;
    (*x)[v] = t.at(k);
    f = !t.at(k) ? lo(f) : hi(f);
  }
}

// Algorithm C modified for ZDD, from Knuth vol. 4 fascicle 1 sec. 7.1.4 (p.75).
intx_t algo_c(zdd_t f) {
    static unordered_map<word_t, intx_t> counts;
    if (is_term(f))
        return is_top(f) ? 1 : 0;
    else if (counts.find(id(f)) != counts.end())
        return counts.at(id(f));
    else
        return counts[id(f)] = algo_c(hi(f)) + algo_c(lo(f));
}

// Algorithm ZUNIQ from Knuth vol. 4 fascicle 1 sec. 7.1.4.
zdd_t zuniq(elem_t v, zdd_t l, zdd_t h) {
  return l + single(v) * h;
}

// Seminumerical Algorithms from Knuth vol. 2, sec. 3.2-3.3.
#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC (1.0/MBIG)
double ran3(int* idum) {
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

void sort_zdd(zdd_t f, vector<vector<zdd_t> >* stacks,
              unordered_set<word_t>* visited, elem_t* max_elem) {
  assert(stacks != nullptr && visited != nullptr);
  if (is_term(f)) return;
  if (visited->find(id(f)) != visited->end()) return;
  (*stacks)[elem(f)].push_back(f);
  visited->insert(id(f));
  if (max_elem != nullptr && elem(f) > *max_elem)
    *max_elem = elem(f);
  sort_zdd(lo(f), stacks, visited, max_elem);
  sort_zdd(hi(f), stacks, visited, max_elem);
}

}  // namespace illion
