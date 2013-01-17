#include <cassert>
#include <cstdio>

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "illion/zdd.h"
#include "illion/setset.h"

#define e0 (illion::top())
#define e1 (illion::single(1))
#define e2 (illion::single(2))
#define e3 (illion::single(3))
#define e4 (illion::single(4))
#define e5 (illion::single(5))

namespace illion {

using namespace std;

extern bool initialized_;

class TestSetset {
 public:
  static void init() {
    assert(!initialized_);
    assert(setset::num_elems() == 0);
  }

  static void constructors() {
    setset ss;
    assert(ss.empty());

    set<int> s = {};
    ss = setset(s);
    assert(!initialized_);
    assert(setset::num_elems() == 0);
    assert(ss.zdd_ == e0);

    s = {1, 2};
    ss = setset(s);
    assert(initialized_);
    assert(setset::num_elems() == 2);
    assert(ss.zdd_ == e1*e2);

    vector<set<int>> v = {{}, {1, 2}, {1, 3}};
    ss = setset(v);
    assert(setset::num_elems() == 3);
    assert(ss.zdd_ == e0 + e1*e2 + e1*e3);

    map<string, set<int>> m = {{"include", {1, 2}}, {"exclude", {4}}};
    ss = setset(m);
    assert(setset::num_elems() == 4);
    assert(ss.zdd_ == e1*e2 + e1*e2*e3);

    vector<map<string, set<int>>> u = {{{"include", {1, 2}}, {"exclude", {4}}},
                                       {{"include", {1, 3, 4}}},
                                       {{"exclude", {2, 3}}}};
    ss = setset(u);
    assert(setset::num_elems() == 4);
    assert(ss.zdd_ == e0 + e1 + e1*e2 + e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4
           + e1*e4 + e4);

    // initializer_list
    ss = setset({{1}, {2}});
    assert(ss.zdd_ == e1 + e2);

    ss = setset({{}, {1, 2}, {1, 3}});
    assert(ss.zdd_ == e0 + e1*e2 + e1*e3);

    // copy constructor
    ss = setset(setset({{1}, {2}}));
    assert(ss.zdd_ == e1 + e2);
  }

  static void comparison() {
    setset ss({{1, 2}});
    assert(ss == setset({{1, 2}}));
    assert(ss != setset({{1, 3}}));

    vector<set<int> > v = {{}, {1, 2}, {1, 3}};
    ss = setset(v);
    assert(ss.is_disjoint(setset({{1}, {1, 2, 3}})));
    assert(!ss.is_disjoint(setset({{1}, {1, 2}})));

    assert(ss.is_subset(setset(v)));
    assert(!ss.is_subset(setset({{}, {1, 2}})));
    assert(ss <= setset(v));
    assert(!(ss <= setset({{}, {1, 2}})));
    assert(ss < setset({{}, {1}, {1, 2}, {1, 3}}));
    assert(!(ss < setset(v)));

    assert(ss.is_superset(setset(v)));
    assert(!ss.is_superset(setset({{1}, {1, 2}})));
    assert(ss >= setset(v));
    assert(!(ss >= setset({{1}, {1, 2}})));
    assert(ss > setset({{}, {1, 2}}));
    assert(!(ss > setset(v)));
  }

  static void unary_operators() {
    setset ss({{}, {1}, {1, 2}, {1, 2, 3}, {1, 2, 3, 4}, {1, 3, 4}, {1, 4},
               {4}});
    assert(setset::num_elems() == 4);
    assert((~ss).zdd_ == e1*e2*e4 + e1*e3 + e2 + e2*e3 + e2*e3*e4 + e2*e4 + e3
           + e3*e4);
    assert(ss.smaller(2).zdd_ == e0 + e1 + e1*e2 + e1*e4 + e4);

    ss = setset({{1, 2}, {1, 4}, {2, 3}, {3, 4}});
    assert(setset::num_elems() == 4);
    assert(ss.hitting().zdd_ == e1*e2*e3 + e1*e2*e3*e4 + e1*e2*e4 + e1*e3
           + e1*e3*e4 + e2*e3*e4 + e2*e4);

    ss = setset({{1, 2}, {1, 2, 3}, {1, 2, 3, 4}, {2, 4, 5}});
    assert(ss.minimal().zdd_ == e1*e2 + e2*e4*e5);
    assert(ss.maximal().zdd_ == e1*e2*e3*e4 + e2*e4*e5);

    // TODO: test operator~() and hitting() after e5 inserted; add max_elem_
    // member to setset objects for the tests
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
    /*
    ss = setset(u) * setset(v);
    assert(ss.zdd_ == e1*e2 + e1*e2*e3 + e1*e2*e4 + e1*e2*e3*e4 + e1*e3*e4
           + e1*e4 + e2*e3 + e2*e3*e4 + e3*e4);

    ss = setset(u);
    ss *= setset(v);
    assert(ss.zdd_ == e1*e2 + e1*e2*e3 + e1*e2*e4 + e1*e2*e3*e4 + e1*e3*e4
           + e1*e4 + e2*e3 + e2*e3*e4 + e3*e4);
    */
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
    ss = setset(u).join(setset(v));
    assert(ss.zdd_ == e1*e2 + e1*e2*e3 + e1*e2*e4 + e1*e2*e3*e4 + e1*e3*e4
           + e1*e4 + e2*e3 + e2*e3*e4 + e3*e4);

    ss = setset(u).meet(setset(v));
    assert(ss.zdd_ == e0 + e1 + e1*e2 + e1*e4 + e2 + e2*e3 + e3 + e3*e4 + e4);

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
    setset::iterator i = ss.begin({0 /* 1-offset */, 3, -2, -2, 4});
    assert(*i == set<int>({1, 4}));
    ++i;
    assert(*i == set<int>({1, 3, 4}));
    ++i;
    assert(*i == set<int>({4}));
  }

  static void lookup() {
    setset ss({{}, {1, 2}, {1, 3}});
    setset::const_iterator i = ss.find({1, 2});
    assert(i != setset::end());
    assert(*i == set<int>({1, 2}));
    assert(setset(i.zdd_).find({1, 2}) == setset::end());
    i = ss.find(set<int>({1}));
    assert(i == setset::end());

    assert(ss.find(1).zdd_ == e1*e2 + e1*e3);

    assert(ss.not_find(2).zdd_ == e0 + e1*e3);

    assert(ss.count({1, 2}) == 1);
    assert(ss.count({2, 3}) == 0);
  }

  static void modifiers() {
    vector<set<int>> v = {{}, {1, 2}, {1, 3}};
    setset ss(v);
    pair<setset::iterator, bool> p = ss.insert({1});
    assert(ss.find(set<int>({1})) != setset::end());
    assert(p.first != setset::end());
    assert(p.first.s_ == set<int>({1}));
    assert(p.second);
    p = ss.insert({1});
    assert(p.first != setset::end());
    assert(p.first.s_ == set<int>({1}));
    assert(!p.second);

    setset::iterator i = ss.insert(p.first, {1});
    assert(i != setset::end());
    assert(i.s_ == set<int>({1}));

    ss.insert({{1}, {2}});
    assert(ss.find(set<int>({2})) != setset::end());

    i = ss.erase(i);
    assert(ss.find(set<int>({1})) == setset::end());
    assert(i == setset::end());

    assert(ss.erase(set<int>({1})) == 0);
    assert(ss.erase({1, 2}) == 1);
    assert(ss.find({1, 2}) == setset::end());

    ss = setset(v);
    assert(ss.erase(1) == 2);
    assert(ss.find({2, 3}) == setset::end());

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

  static void io() {
    stringstream sstr;
    setset ss;
    sstr << ss;
    assert(sstr.str() == "B\n.\n");
    ss.clear();
    sstr >> ss;
    assert(sstr.good());
    assert(ss == setset());

    sstr.clear(); sstr.str("");
    ss = setset({{}});
    sstr << ss;
    assert(sstr.str() == "T\n.\n");
    ss.clear();
    sstr >> ss;
    assert(sstr.good());
    assert(ss == setset({{}}));

    sstr.clear(); sstr.str("");
    vector<set<int> > v = {{}, {1}, {1, 2}, {1, 2, 3}, {1, 2, 3, 4}, {1, 3, 4},
                           {1, 4}, {4}};
    ss = setset(v);
    sstr << ss;
    ss.clear();
    sstr >> ss;
    assert(sstr.good());
    assert(ss == setset(v));

    sstr.clear(); sstr.str("");
    sstr << ss;
    ss = setset(sstr);
    assert(ss == setset(v));

    string str1 = "hello", str2 = "bye";
    sstr.clear(); sstr.str("");
    sstr << str1 << " " << endl << ss << endl << str2 << endl;
    ss.clear();
    sstr >> str1 >> ss >> str2;
    assert(sstr.good());
    assert(str1 == "hello" && ss == setset(v) && str2 == "bye");

    sstr.clear(); sstr.str("");
    ss.dump(sstr);
    ss.clear();
    ss.load(sstr);
    assert(ss == setset(v));

    FILE* fp = fopen("/tmp/illion_", "w");
    ss.dump(fp);
    fclose(fp);
    ss.clear();
    fp = fopen("/tmp/illion_", "r");
    ss.load(fp);
    fclose(fp);
    assert(ss == setset(v));
  }
};

}  // namespace illion

int main() {
  illion::TestSetset::init();
  illion::TestSetset::constructors();
  illion::TestSetset::comparison();
  illion::TestSetset::unary_operators();
  illion::TestSetset::binary_operators();
  illion::TestSetset::capacity();
  illion::TestSetset::iterators();
  illion::TestSetset::lookup();
  illion::TestSetset::modifiers();
  illion::TestSetset::io();
  printf("ok\n");
  return 0;
}
