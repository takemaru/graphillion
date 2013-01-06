#include <cassert>
#include <cstdio>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "illion/setset.h"

#define e0 (illion::zdd::top())
#define e1 (illion::zdd::single(1))
#define e2 (illion::zdd::single(2))
#define e3 (illion::zdd::single(3))
#define e4 (illion::zdd::single(4))
#define e5 (illion::zdd::single(5))

namespace illion {

using namespace std;

class setset_test {
 public:
  static void constructors() {
    setset ss;
    assert(ss.empty());

    set<int> s = {};
    ss = setset(s);
    assert(!zdd::initialized_);
    assert(zdd::num_elems_ == 0);
    assert(ss.zdd_ == e0);

    s = {1, 2};
    ss = setset(s);
    assert(zdd::initialized_);
    assert(zdd::num_elems_ == 2);
    assert(ss.zdd_ == e1*e2);

    vector<set<int>> v = {{}, {1, 2}, {1, 3}};
    ss = setset(v);
    assert(zdd::num_elems_ == 3);
    assert(ss.zdd_ == e0 + e1*e2 + e1*e3);

    map<string, set<int>> m = {{"include", {1, 2}}, {"exclude", {4}}};
    ss = setset(m);
    assert(zdd::num_elems_ == 4);
    assert(ss.zdd_ == e1*e2 + e1*e2*e3);

    vector<map<string, set<int>>> u = {{{"include", {1, 2}}, {"exclude", {4}}},
                                       {{"include", {1, 3, 4}}},
                                       {{"exclude", {2, 3}}}};
    ss = setset(u);
    assert(zdd::num_elems_ == 4);
    assert(ss.zdd_ == e0 + e1 + e1*e2 + e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4
           + e1*e4 + e4);

    // initializer_list
    ss = setset({{1}, {2}});
    assert(ss.zdd_ == e1 + e2);

    ss = setset({{}, {1, 2}, {1, 3}});
    assert(ss.zdd_ == e0 + e1*e2 + e1*e3);
  }

  static void unary_operators() {
    setset ss({{1, 2}});
    assert(ss == setset({{1, 2}}));
    assert(ss != setset({{1, 3}}));

    ss = setset({{}, {1}, {1, 2}, {1, 2, 3}, {1, 2, 3, 4}, {1, 3, 4}, {1, 4},
                 {4}});
    assert(zdd::num_elems_ == 4);
    assert((~ss).zdd_ == e1*e2*e4 + e1*e3 + e2 + e2*e3 + e2*e3*e4 + e2*e4 + e3
           + e3*e4);
    assert(ss.smaller(2).zdd_ == e0 + e1 + e1*e2 + e1*e4 + e4);

    ss = setset({{1, 2}, {1, 4}, {2, 3}, {3, 4}});
    assert(zdd::num_elems_ == 4);
    assert(ss.hitting().zdd_ == e1*e2*e3 + e1*e2*e3*e4 + e1*e2*e4 + e1*e3
           + e1*e3*e4 + e2*e3*e4 + e2*e4);

    ss = setset({{1, 2}, {1, 2, 3}, {1, 2, 3, 4}, {2, 4, 5}});
    assert(ss.minimal().zdd_ == e1*e2 + e2*e4*e5);
    assert(ss.maximal().zdd_ == e1*e2*e3*e4 + e2*e4*e5);

    // TODO: test operator~() and hitting() after e5 inserted; add max_elem_
    // member to setset objects for the test
  }

  static void binary_operators() {
    vector<set<int>> u = {{}, {1}, {1, 2}, {1, 2, 3}, {1, 2, 3, 4}, {1, 3, 4},
                          {1, 4}, {4}};
    vector<set<int>> v = {{1, 2}, {1, 4}, {2, 3}, {3, 4}};
    setset ss = setset(u) & setset(v);
    assert(ss.zdd_ == e1*e2 + e1*e4);

    ss = setset(u);
    ss &= setset(v);
    assert(ss.zdd_ == e1*e2 + e1*e4);

    ss = setset(u) | setset(v);
    assert(ss.zdd_ == e0 + e1 + e1*e2 + e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4
           + e1*e4 + e2*e3 + e3*e4 + e4);

    ss = setset(u);
    ss |= setset(v);
    assert(ss.zdd_ == e0 + e1 + e1*e2 + e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4
           + e1*e4 + e2*e3 + e3*e4 + e4);

    ss = setset(u) - setset(v);
    assert(ss.zdd_ == e0 + e1 + e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4 + e4);

    ss = setset(u);
    ss -= setset(v);
    assert(ss.zdd_ == e0 + e1 + e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4 + e4);

    ss = setset(u) * setset(v);
    assert(ss.zdd_ == e1*e2 + e1*e2*e3 + e1*e2*e4 + e1*e2*e3*e4 + e1*e3*e4
           + e1*e4 + e2*e3 + e2*e3*e4 + e3*e4);

    ss = setset(u);
    ss *= setset(v);
    assert(ss.zdd_ == e1*e2 + e1*e2*e3 + e1*e2*e4 + e1*e2*e3*e4 + e1*e3*e4
           + e1*e4 + e2*e3 + e2*e3*e4 + e3*e4);

    ss = setset(u) ^ setset(v);
    assert(ss.zdd_ == e0 + e1 + e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4 + e2*e3
           + e3*e4 + e4);

    ss = setset(u);
    ss ^= setset(v);
    assert(ss.zdd_ == e0 + e1 + e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4 + e2*e3
           + e3*e4 + e4);

    v = {{1, 2}};
    ss = setset(u) / setset(v);
    assert(ss.zdd_ == e0 + e3 + e3*e4);

    ss = setset(u);
    ss /= setset(v);
    assert(ss.zdd_ == e0 + e3 + e3*e4);

    ss = setset(u) % setset(v);
    assert(ss.zdd_ == e0 + e1 + e1*e3*e4 + e1*e4 + e4);

    ss = setset(u);
    ss %= setset(v);
    assert(ss.zdd_ == e0 + e1 + e1*e3*e4 + e1*e4 + e4);

    v = {{1, 2}, {1, 4}, {2, 3}, {3, 4}};
    ss = setset(u).subsets(setset(v));
    assert(ss.zdd_ == e0 + e1 + e1*e2 + e1*e4 + e4);

    ss = setset(u).supersets(setset(v));
    assert(ss.zdd_ == e1*e2 + e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4 + e1*e4);

    ss = setset(u).nonsubsets(setset(v));
    assert(ss.zdd_ == e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4);

    ss = setset(u).nonsupersets(setset(v));
    assert(ss.zdd_ == e0 + e1 + e4);
  }

  static void testers() {
    vector<set<int> > v = {{}, {1, 2}, {1, 3}};
    setset ss(v);
    assert(ss.is_disjoint(setset({{1}, {1, 2, 3}})));
    assert(!ss.is_disjoint(setset({{1}, {1, 2}})));

    assert(ss.is_subset(setset(v)));
    assert(!ss.is_subset(setset({{}, {1, 2}})));
    assert(ss <= setset(v));
    assert(!(ss <= setset({{}, {1, 2}})));
    assert(ss <= setset({{}, {1}, {1, 2}, {1, 3}}));
    assert(!(ss < setset(v)));

    assert(ss.is_superset(setset(v)));
    assert(!ss.is_superset(setset({{1}, {1, 2}})));
    assert(ss >= setset(v));
    assert(!(ss >= setset({{1}, {1, 2}})));
    assert(ss > setset({{}, {1, 2}}));
    assert(!(ss > setset(v)));
  }

  static void capacity() {
    setset ss;
    assert(ss.empty());

    ss = setset({{}, {1, 2}, {1, 3}});
    assert(!ss.empty());

    assert(ss.size() == "3");
  }

  static void iterators() {
    setset ss1({{}, {1, 2}, {1, 3}});
    setset ss2;
    for (const auto& s : ss1)
      ss2 |= setset(s);
    assert(ss1 == ss2);

    ss2.clear();
    for (auto& s : ss1)
      ss2 |= setset(s);
    assert(ss1 == ss2);

    setset ss({{}, {1}, {1, 2}, {1, 2, 3}, {1, 2, 3, 4}, {1, 3, 4}, {1, 4},
               {4}});
    ss.set_weights({0 /* 1-offset */, 3, -2, -2, 4, 0});  // TODO: remove trailing zero
    setset::iterator i = ss.begin();
    assert(*i == set<int>({1, 4}));
    ++i;
    assert(*i == set<int>({1, 3, 4}));
    ++i;
    assert(*i == set<int>({4}));
  }

  static void lookup() {
    setset ss({{}, {1, 2}, {1, 3}});
    setset::const_iterator i = ss.find({1, 2});
    assert(i != ss.end());
    assert(*i == set<int>({1, 2}));
    assert(setset(i.zdd_).find({1, 2}) == ss.end());
    i = ss.find({1});
    assert(i == ss.end());

    assert(ss.count({1, 2}) == 1);
    assert(ss.count({2, 3}) == 0);
  }

  static void modifiers() {
    vector<set<int>> v = {{}, {1, 2}, {1, 3}};
    setset ss(v);
    pair<setset::iterator, bool> p = ss.insert({1});
    assert(ss.find({1}) != ss.end());
    assert(p.first.s_ == set<int>({1}));
    assert(p.second);
    p = ss.insert({1});
    assert(p.first.s_ == set<int>({1}));
    assert(!p.second);

    setset::iterator i = ss.insert(p.first, {1});
    assert(i.s_ == set<int>({1}));

    ss.insert({{1}, {2}});
    assert(ss.find({2}) != ss.end());

    i = ss.erase(i);
    assert(ss.find({1}) == ss.end());
    assert(i == ss.end());

    assert(ss.erase({1}) == 0);
    assert(ss.erase({1, 2}) == 1);
    assert(ss.find({1, 2}) == ss.end());

    ss = setset(v);
    assert(!ss.empty());
    ss.clear();
    assert(ss.empty());

    set<int> s = {1, 2};
    setset ss1(s);
    setset ss2(v);
    ss1.swap(ss2);
    assert(ss1 == setset(v));
    assert(ss2 == setset(s));
  }
};

}  // namespace illion

int main() {
  illion::setset_test::constructors();
  illion::setset_test::unary_operators();
  illion::setset_test::binary_operators();
  illion::setset_test::testers();
  illion::setset_test::capacity();
  illion::setset_test::iterators();
  illion::setset_test::lookup();
  illion::setset_test::modifiers();
  printf("ok\n");
  return 0;
}
