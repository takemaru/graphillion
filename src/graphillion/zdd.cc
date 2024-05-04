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

#include "graphillion/zdd.h"

//#include <cassert>
#include <climits>

#include <algorithm>
#include <map>
#include <string>

namespace graphillion {

using std::binary_search;
using std::endl;
using std::getline;
using std::istream;
using std::map;
using std::ostream;
using std::pair;
using std::set;
using std::string;
using std::vector;

static const string WORD_FMT = "%lld";

static bool initialized_ = false;

// number of elements activated in the ZDD package
static elem_t max_elem_ = 0;

// size of universe, which must not be larger than max_elem_
static elem_t num_elems_ = 0;

ZBDD operator|(const ZBDD& f, const ZBDD& g) {
  return f + g;
}

void init() {
  if (initialized_) return;
  BDD_Init(10000, 8000000000LL);
  initialized_ = true;
}

elem_t elem_limit() {
  return BDD_MaxVar;
}

elem_t max_elem() {
  assert(BDD_VarUsed() == max_elem_);
  return max_elem_;
}

void new_elems(elem_t max_elem) {
  assert(max_elem <= elem_limit());
  if (!initialized_) init();
  if (num_elems_ < max_elem) num_elems_ = max_elem;
  while (max_elem_ < max_elem) {
    top().Change(BDD_NewVarOfLev(1));
    num_elems_ = ++max_elem_;
  }
  assert(num_elems_ <= max_elem_);
  assert(BDD_VarUsed() == max_elem_);
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
  static map<word_t, zdd_t> cache;
  if (is_term(f)) return f;
  map<word_t, zdd_t>::iterator i = cache.find(id(f));
  if (i != cache.end())
    return i->second;
  zdd_t rl = minimal(lo(f));
  zdd_t r = minimal(hi(f));
  zdd_t rh = non_supersets(r, rl);
  r = zuniq(elem(f), rl, rh);
  return cache[id(f)] = r;
}

zdd_t maximal(zdd_t f) {
  static map<word_t, zdd_t> cache;
  if (is_term(f)) return f;
  map<word_t, zdd_t>::iterator i = cache.find(id(f));
  if (i != cache.end())
    return i->second;
  zdd_t r = maximal(lo(f));
  zdd_t rh = maximal(hi(f));
  zdd_t rl = non_subsets(r, rh);
  r = zuniq(elem(f), rl, rh);
  return cache[id(f)] = r;
}

zdd_t hitting(zdd_t f) {
  if (f == bot()) return top();
  if (f == top()) return bot();
  vector<vector<zdd_t> > stacks(num_elems_ + 1);
  set<word_t> visited;
  sort_zdd(f, &stacks, &visited);
  map<word_t, zdd_t> cache;
  cache[id(bot())] = bot();
  cache[id(top())] = bot();
  for (elem_t v = num_elems_; v > 0; --v) {
    while (!stacks[v].empty()) {
      zdd_t n = stacks[v].back();
      stacks[v].pop_back();
      zdd_t l = cache.at(id(lo(n)));
      if (lo(n) != bot()) {
        elem_t j = lo(n) == top() ? num_elems_ : elem(lo(n)) - 1;
        for (; j > v; --j)
          l = l + l.Change(j);
      }
      zdd_t h = cache.at(id(hi(n)));
      if (hi(n) != bot()) {
        elem_t j = hi(n) == top() ? num_elems_ : elem(hi(n)) - 1;
        for (; j > v; --j)
          h = h + h.Change(j);
      }
      if (lo(n) == bot()) {
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

zdd_t non_subsets(zdd_t f, zdd_t g) {
  static map<pair<word_t, word_t>, zdd_t> cache;
  if (g == bot())
    return f;
  else if (g == top())
    return f - top();
  else if (f == bot() || f == top() || f == g)
    return bot();
  pair<word_t, word_t> k = make_key(f, g);
  map<pair<word_t, word_t>, zdd_t>::iterator i = cache.find(k);
  if (i != cache.end())
    return i->second;
  zdd_t r, r2, rl, rh;
  if (elem(f) < elem(g)) {
    rl = non_subsets(lo(f), g);
    rh = hi(f);
    r = zuniq(elem(f), rl, rh);
  } else if (elem(f) == elem(g)) {
//    r = non_subsets(lo(f), lo(g));
//    r2 = non_subsets(lo(f), hi(g));
//    rl = r & r2;
    r2 = lo(g) | hi(g);
    rl = non_subsets(lo(f), r2);
    rh = non_subsets(hi(f), hi(g));
    r = zuniq(elem(f), rl, rh);
  } else {
    r2 = lo(g) | hi(g);
    r = non_subsets(f, r2);
  }
  return cache[k] = r;
}

zdd_t non_supersets(zdd_t f, zdd_t g) {
  static map<pair<word_t, word_t>, zdd_t> cache;
  if (g == bot())
    return f;
  else if (f == bot() || g == top() || f == g)
    return bot();
  else if (f == top())
    return top();
  else if (elem(f) > elem(g))
    return non_supersets(f, lo(g));
  pair<word_t, word_t> k = make_key(f, g);
  map<pair<word_t, word_t>, zdd_t>::iterator i = cache.find(k);
  if (i != cache.end())
    return i->second;
  elem_t v = elem(f);
  zdd_t r;
  zdd_t rl;
  zdd_t rh;
  if (elem(f) < elem(g)) {
    rl = non_supersets(lo(f), g);
    rh = non_supersets(hi(f), g);
  } else {
    rl = non_supersets(hi(f), hi(g));
    r = non_supersets(hi(f), lo(g));
    rh = r & rl;
    rl = non_supersets(lo(f), lo(g));
  }
  r = zuniq(v, rl, rh);
  return cache[k] = r;
}

bool choose(zdd_t f, vector<elem_t>* stack) {
  assert(stack != NULL);
  int last = stack->size() - 1;
  if (f == bot())
    return false;
  else if (f == top())
    return true;
  // if elem(f) > any in stack
  if (last < 0 || elem(f) > (*stack)[last]) {
    stack->push_back(elem(f));
    if (choose(hi(f), stack))
      return true;
  } else {
    // if elem(f) in stack
    if (binary_search(stack->begin(), stack->end(), elem(f))) {
      // if not elem(f) is last element in stack
      if (elem(f) != (*stack)[last] && choose(hi(f), stack))
        return true;
    } else {
      // if elem(f) not in stack
      if (lo(f) != bot() && choose(lo(f), stack))
        return true;
      return false;
    }
  }
  // fail in hi(f) and try lo(f)
  last = stack->size() - 1;
  // if elem(f) is last element in stack
  if (last >= 0 && elem(f) == (*stack)[last]) {
    stack->pop_back();
    if (lo(f) != bot() && choose(lo(f), stack))
      return true;
  }
  return false;
}

zdd_t choose_random(zdd_t f, vector<elem_t>* stack) {
  assert(stack != NULL);
  if (is_term(f)) {
    if (f == top()) {
      zdd_t g = top();
      for (int i = 0; i <= static_cast<int>(stack->size()) - 1; i++)
        g = g * single((*stack)[i]);
      return g;
    }
    assert(false);
  }
  double ch = algo_c(hi(f));
  double cl = algo_c(lo(f));
  if (rand_xor128() > cl / (ch + cl)) {
    stack->push_back(elem(f));
    return choose_random(hi(f), stack);
  } else {
    return choose_random(lo(f), stack);
  }
}

zdd_t choose_best(zdd_t f, const vector<double>& weights, set<elem_t>* s) {
  assert(s != NULL);
  if (f == bot()) return bot();
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
  if (f == bot()) {
    out << "B" << endl;
  } else if (f == top()) {
    out << "T" << endl;
  } else {
    vector<vector<zdd_t> > stacks(num_elems_ + 1);
    set<word_t> visited;
    sort_zdd(f, &stacks, &visited);
    for (elem_t v = num_elems_; v > 0; --v) {
      while (!stacks[v].empty()) {
        zdd_t g = stacks[v].back();
        stacks[v].pop_back();
        zdd_t l = lo(g);
        zdd_t h = hi(g);
        out << id(g) << " " << elem(g) << " ";
        if      (l == bot()) out << "B";
        else if (l == top()) out << "T";
        else                 out << id(l);
        out << " ";
        if      (h == bot()) out << "B";
        else if (h == top()) out << "T";
        else                 out << id(h);
        out << endl;
      }
    }
  }
  out << "." << endl;
}

void dump(zdd_t f, FILE* fp) {
  if (f == bot()) {
    fprintf(fp, "B\n");
  } else if (f == top()) {
    fprintf(fp, "T\n");
  } else {
    vector<vector<zdd_t> > stacks(num_elems_ + 1);
    set<word_t> visited;
    sort_zdd(f, &stacks, &visited);
    for (elem_t v = num_elems_; v > 0; --v) {
      while (!stacks[v].empty()) {
        zdd_t g = stacks[v].back();
        stacks[v].pop_back();
        zdd_t l = lo(g);
        zdd_t h = hi(g);
        fprintf(fp, (WORD_FMT +" %d ").c_str(), id(g), elem(g));
        if      (l == bot()) fprintf(fp, "B");
        else if (l == top()) fprintf(fp, "T");
        else                 fprintf(fp, WORD_FMT.c_str(), id(l));
        fprintf(fp, " ");
        if      (h == bot()) fprintf(fp, "B");
        else if (h == top()) fprintf(fp, "T");
        else                 fprintf(fp, WORD_FMT.c_str(), id(h));
        fprintf(fp, "\n");
      }
    }
  }
  fprintf(fp, ".\n");
}

zdd_t load(istream& in) {
  string line;
  getline(in, line);
  if      (line[0] == 'B' && (line.length() == 1 || is_space(line.substr(1)))) return bot();
  else if (line[0] == 'T' && (line.length() == 1 || is_space(line.substr(1)))) return top();
  map<word_t, zdd_t> n;
  n[id(bot())] = bot();
  n[id(top())] = top();
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
             :                        strtoll(sl, NULL, 0);
    word_t h = strcmp(sh, "B") == 0 ? id(bot())
             : strcmp(sh, "T") == 0 ? id(top())
             :                        strtoll(sh, NULL, 0);
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
  if      (line[0] == 'B' && (line.length() == 1 || is_space(line.substr(1)))) return bot();
  else if (line[0] == 'T' && (line.length() == 1 || is_space(line.substr(1)))) return top();
  map<word_t, zdd_t> n;
  n[id(bot())] = bot();
  n[id(top())] = top();
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
             :                        strtoll(sl, NULL, 0);
    word_t h = strcmp(sh, "B") == 0 ? id(bot())
             : strcmp(sh, "T") == 0 ? id(top())
             :                        strtoll(sh, NULL, 0);
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
  if (out.rdbuf() == std::cout.rdbuf() || out.rdbuf() == std::cerr.rdbuf())
    out << endl;
}

void _enum(zdd_t f, FILE* fp,
           const pair<const char*, const char*>& outer_braces,
           const pair<const char*, const char*>& inner_braces) {
  vector<elem_t> stack;
  fprintf(fp, "%s", outer_braces.first);
  bool first = true;
  _enum(f, fp, &stack, &first, inner_braces);
  fprintf(fp, "%s", outer_braces.second);
  if (fp == stdout || fp == stderr)
    fprintf(fp, "\n");
}

void _enum(zdd_t f, ostream& out, vector<elem_t>* stack, bool* first,
           const pair<const char*, const char*>& inner_braces) {
  assert(stack != NULL);
  if (is_term(f)) {
    if (f == top()) {
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
  assert(stack != NULL);
  if (is_term(f)) {
    if (f == top()) {
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
void algo_b(zdd_t f, const vector<double>& w, vector<bool>* x) {
  assert(x != NULL);
  assert(f != bot());
  if (f == top()) return;
  vector<vector<zdd_t> > stacks(num_elems_ + 1);
  set<word_t> visited;
  elem_t max_elem = 0;
  sort_zdd(f, &stacks, &visited, &max_elem);
  assert(w.size() > static_cast<size_t>(max_elem));
  x->clear();
  x->resize(max_elem + 1, false);
  map<word_t, bool> t;
  map<word_t, double> ms;
  ms[id(bot())] = INT_MIN;
  ms[id(top())] = 0;
  for (elem_t v = max_elem; v > 0; --v) {
    while (!stacks[v].empty()) {
      zdd_t g = stacks[v].back();
      stacks[v].pop_back();
      word_t k = id(g);
      elem_t v = elem(g);
      word_t l = id(lo(g));
      word_t h = id(hi(g));
      if (lo(g) != bot())
        ms[k] = ms.at(l);
      if (hi(g) != bot()) {
        double m = ms.at(h) + w[v];
        if (lo(g) == bot() || m > ms.at(k)) {
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
double algo_c(zdd_t f) {
  static map<word_t, double> counts;
  if (is_term(f))
    return f == top() ? 1 : 0;
  else if (counts.find(id(f)) != counts.end())
    return counts.at(id(f));
  else
    return counts[id(f)] = algo_c(hi(f)) + algo_c(lo(f));
}

static double skip_probability(elem_t e, zdd_t f, const vector<double>& probabilities) {
  double p = 1;
  for (int i = e; i < (is_term(f) ? num_elems() + 1 : elem(f)); ++i)
    p *= 1 - probabilities[i];
  return p;
}

double probability(elem_t e, zdd_t f, const vector<double>& probabilities,
                   map<word_t, double>& cache) {
  zdd_t l = lo(f);
  zdd_t h = hi(f);
  if (cache.find(id(l)) == cache.end())
    cache[id(l)] = probability(elem(l), l, probabilities, cache);
  if (cache.find(id(h)) == cache.end())
    cache[id(h)] = probability(elem(h), h, probabilities, cache);
  double pl = (1 - probabilities[elem(f)]) *
      skip_probability(elem(f) + 1, l, probabilities) * cache.at(id(l));
  double ph = probabilities[elem(f)] *
      skip_probability(elem(f) + 1, h, probabilities) * cache.at(id(h));
  return skip_probability(e, f, probabilities) * (pl + ph);
}

// Algorithm ZUNIQ from Knuth vol. 4 fascicle 1 sec. 7.1.4.
zdd_t zuniq(elem_t v, zdd_t l, zdd_t h) {
  return l + single(v) * h;
}

// G. Marsaglia, "Xorshift RNGs," Journal of Statistical Software, vol.8,
// issue.14, 2003.  http://www.jstatsoft.org/v08/i14/
double rand_xor128() {
    static unsigned long x = 123456789, y = 362436069, z = 521288629,
        w = 88675123;
    unsigned long t;
    t = (x^(x << 11));
    x = y;
    y = z;
    z = w;
    w = (w^(w >> 19))^(t^(t >> 8));
    return static_cast<double>(w) / ULONG_MAX;
}

void sort_zdd(zdd_t f, vector<vector<zdd_t> >* stacks,
              set<word_t>* visited, elem_t* max_elem) {
  assert(stacks != NULL && visited != NULL);
  if (is_term(f)) return;
  if (visited->find(id(f)) != visited->end()) return;
  (*stacks)[elem(f)].push_back(f);
  visited->insert(id(f));
  if (max_elem != NULL && elem(f) > *max_elem)
    *max_elem = elem(f);
  sort_zdd(lo(f), stacks, visited, max_elem);
  sort_zdd(hi(f), stacks, visited, max_elem);
}

}  // namespace graphillion
