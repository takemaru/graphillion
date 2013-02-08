#include <cassert>
#include <cstdio>

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "graphillion/zdd.h"
#include "graphillion/setset.h"

#define e0 (graphillion::top())
#define e1 (graphillion::single(1))
#define e2 (graphillion::single(2))
#define e3 (graphillion::single(3))
#define e4 (graphillion::single(4))
#define e5 (graphillion::single(5))

namespace graphillion {

using namespace std;

set<int> S(const string& str) {
  set<int> s;
  vector<string> v = split(str, "{}, ");
  for (vector<string>::const_iterator i = v.begin(); i != v.end(); ++i)
    s.insert(strtol(i->c_str(), NULL, NULL));
  return s;
}

vector<set<int> > V(const string& str) {
  char* buf = new char[str.size() + 1];
  strcpy(buf, str.c_str());
  vector<set<int> > v;
  int begin = 0;
  for (int i = 0; i < static_cast<int>(str.size()); ++i) {
    if (buf[i] == '{') {
      begin = i + 1;
    } else if (buf[i] == '}') {
      buf[i] = '\0';
      v.push_back(S(&buf[begin]));
    }
  }
  delete [] buf;
  return v;
}

class TestSetset {
 public:
  void run() {
    this->init();
    this->constructors();
    this->comparison();
    this->unary_operators();
    this->binary_operators();
    this->capacity();
    this->iterators();
    this->lookup();
    this->modifiers();
    this->io();
    this->large();
  }

  void init() {
    assert(num_elems() == 0);
    assert(setset::num_elems() == 0);

    setset::num_elems(2);
    assert(setset::num_elems() == 2);

    map<string, vector<int> > m;
    setset ss(m);
    assert(ss.zdd_ == e0 + e1 + e1*e2 + e2);

    setset::num_elems(1);
    assert(setset::num_elems() == 1);

    ss = setset(m);
    assert(ss.zdd_ == e0 + e1);
  }

  void constructors() {
    setset ss;
    assert(ss.empty());

    set<int> s = S("{}");
    ss = setset(s);
    assert(ss.zdd_ == e0);

    s = S("{1, 2}");
    ss = setset(s);
    assert(ss.zdd_ == e1*e2);

    vector<set<int> > v = V("{{}, {1, 2}, {1, 3}}");
    ss = setset(v);
    assert(ss.zdd_ == e0 + e1*e2 + e1*e3);

    map<string, vector<int> > m;
    m["include"].push_back(1);
    m["include"].push_back(2);
    m["exclude"].push_back(4);
    ss = setset(m);
    assert(ss.zdd_ == e1*e2 + e1*e2*e3);

    // copy constructor
    ss = setset(setset(V("{{1}, {2}}")));
    assert(ss.zdd_ == e1 + e2);
  }

  void comparison() {
    setset ss(V("{{1, 2}}"));
    assert(ss == setset(V("{{1, 2}}")));
    assert(ss != setset(V("{{1, 3}}")));

    vector<set<int> > v = V("{{}, {1, 2}, {1, 3}}");
    ss = setset(v);
    assert(ss.is_disjoint(setset(V("{{1}, {1, 2, 3}}"))));
    assert(!ss.is_disjoint(setset(V("{{1}, {1, 2}}"))));

    assert(ss.is_subset(setset(v)));
    assert(!ss.is_subset(setset(V("{{}, {1, 2}}"))));
    assert(ss <= setset(v));
    assert(!(ss <= setset(V("{{}, {1, 2}}"))));
    assert(ss < setset(V("{{}, {1}, {1, 2}, {1, 3}}")));
    assert(!(ss < setset(v)));

    assert(ss.is_superset(setset(v)));
    assert(!ss.is_superset(setset(V("{{1}, {1, 2}}"))));
    assert(ss >= setset(v));
    assert(!(ss >= setset(V("{{1}, {1, 2}}"))));
    assert(ss > setset(V("{{}, {1, 2}}")));
    assert(!(ss > setset(v)));
  }

  void unary_operators() {
    setset::num_elems(4);
    assert(setset::num_elems() == 4);

    setset ss(V("{{}, {1}, {1, 2}, {1, 2, 3}, {1, 2, 3, 4}, {1, 3, 4}, {1, 4},"
                "{4}}"));
    assert((~ss).zdd_ == e1*e2*e4 + e1*e3 + e2 + e2*e3 + e2*e3*e4 + e2*e4 + e3
           + e3*e4);
    assert(ss.smaller(3).zdd_ == e0 + e1 + e1*e2 + e1*e4 + e4);
    assert(ss.larger(3).zdd_ == e1*e2*e3*e4);
    assert(ss.equal(3).zdd_ == e1*e2*e3 + e1*e3*e4);

    ss = setset(V("{{1, 2}, {1, 4}, {2, 3}, {3, 4}}"));
    assert(ss.hitting().zdd_ == e1*e2*e3 + e1*e2*e3*e4 + e1*e2*e4 + e1*e3
           + e1*e3*e4 + e2*e3*e4 + e2*e4);

    ss = setset(V("{{1, 2}, {1, 2, 3}, {1, 2, 3, 4}, {2, 4, 5}}"));
    assert(ss.minimal().zdd_ == e1*e2 + e2*e4*e5);
    assert(ss.maximal().zdd_ == e1*e2*e3*e4 + e2*e4*e5);
  }

  void binary_operators() {
    vector<set<int> > u = V("{{}, {1}, {1, 2}, {1, 2, 3}, {1, 2, 3, 4}, {1, 3, 4},"
                            "{1, 4}, {4}}");
    vector<set<int> > v = V("{{1, 2}, {1, 4}, {2, 3}, {3, 4}}");
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

    ss = setset(u) ^ setset(v);
    assert(ss.zdd_ == e0 + e1 + e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4 + e2*e3
           + e3*e4 + e4);

    ss = setset(u);
    ss ^= setset(v);
    assert(ss.zdd_ == e0 + e1 + e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4 + e2*e3
           + e3*e4 + e4);

    v = V("{{1, 2}}");
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

    ss = setset(u).invert(1);
    assert(ss.zdd_ == e0 + e1 + e1*e4 + e2 + e2*e3 + e2*e3*e4 + e3*e4 + e4);

    v = V("{{1, 2}, {1, 4}, {2, 3}, {3, 4}}");
    ss = setset(u).join(setset(v));
    assert(ss.zdd_ == e1*e2 + e1*e2*e3 + e1*e2*e4 + e1*e2*e3*e4 + e1*e3*e4
           + e1*e4 + e2*e3 + e2*e3*e4 + e3*e4);

    ss = setset(u).meet(setset(v));
    assert(ss.zdd_ == e0 + e1 + e1*e2 + e1*e4 + e2 + e2*e3 + e3 + e3*e4 + e4);

    v = V("{{1, 2}, {1, 4}, {2, 3}, {3, 4}}");
    ss = setset(u).subsets(setset(v));
    assert(ss.zdd_ == e0 + e1 + e1*e2 + e1*e4 + e4);

    ss = setset(u).supersets(setset(v));
    assert(ss.zdd_ == e1*e2 + e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4 + e1*e4);

    ss = setset(u).nonsubsets(setset(v));
    assert(ss.zdd_ == e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4);

    ss = setset(u).nonsupersets(setset(v));
    assert(ss.zdd_ == e0 + e1 + e4);
  }

  void capacity() {
    setset ss;
    assert(ss.empty());

    ss = setset(V("{{}, {1, 2}, {1, 3}}"));
    assert(!ss.empty());

    assert(ss.size() == "3");
  }

  void iterators() {
    setset ss1(V("{{}, {1, 2}, {1, 3}}"));
    setset ss2;
    for (setset::const_iterator s = ss1.begin(); s != ss1.end(); ++s)
      ss2 |= setset(*s);
    assert(ss1 == ss2);

    ss2.clear();
    for (setset::iterator s = ss1.begin(); s != ss1.end(); ++s)
      ss2 |= setset(*s);
    assert(ss1 == ss2);

    setset ss(V("{{}, {1}, {1, 2}, {1, 2, 3}, {1, 2, 3, 4}, {1, 3, 4}, {1, 4},"
                "{4}}"));
    vector<double> w;
    w.push_back(0);  // 1-offset
    w.push_back(.3);
    w.push_back(-.2);
    w.push_back(-.2);
    w.push_back(.4);

    setset::iterator i = ss.maximize(w);
    assert(*i == S("{1, 4}"));
    ++i;
    assert(*i == S("{1, 3, 4}"));
    ++i;
    assert(*i == S("{4}"));

    i = ss.minimize(w);
    assert(*i == S("{1, 2, 3}"));
    ++i;
    assert(*i == S("{}"));
    ++i;
    assert(*i == S("{1, 2}"));
  }

  void lookup() {
    setset ss(V("{{}, {1, 2}, {1, 3}}"));
    setset::const_iterator i = ss.find(S("{1, 2}"));
    assert(i != setset::end());
    assert(*i == S("{1, 2}"));
    assert(setset(i.zdd_).find(S("{1, 2}")) == setset::end());
    i = ss.find(S("{1}"));
    assert(i == setset::end());

    assert(ss.include(1).zdd_ == e1*e2 + e1*e3);

    assert(ss.exclude(2).zdd_ == e0 + e1*e3);

    assert(ss.count(S("{1, 2}")) == 1);
    assert(ss.count(S("{2, 3}")) == 0);
  }

  void modifiers() {
    vector<set<int> > v = V("{{}, {1, 2}, {1, 3}}");
    setset ss(v);
    pair<setset::iterator, bool> p = ss.insert(S("{1}"));
    assert(ss.find(S("{1}")) != setset::end());
    assert(p.first != setset::end());
    assert(p.first.s_ == S("{1}"));
    assert(p.second);

    p = ss.insert(S("{1}"));
    assert(p.first != setset::end());
    assert(p.first.s_ == S("{1}"));
    assert(!p.second);

    setset::iterator i = ss.insert(p.first, S("{1}"));
    assert(i != setset::end());
    assert(i.s_ == S("{1}"));

    i = ss.erase(i);
    assert(ss.find(S("{1}")) == setset::end());
    assert(i == setset::end());

    assert(ss.erase(S("{1}")) == 0);
    assert(ss.erase(S("{1, 2}")) == 1);
    assert(ss.find(S("{1, 2}")) == setset::end());

    ss = setset(v);
    ss.insert(2);
    assert(ss == setset(V("{{1, 2}, {1, 2, 3}, {2}}")));

    ss = setset(v);
    ss.erase(2);
    assert(ss == setset(V("{{}, {1}, {1, 3}}")));

    ss = setset(v);
    assert(!ss.empty());
    ss.clear();
    assert(ss.empty());

    set<int> s = S("{1, 2}");
    setset ss1(s);
    setset ss2(v);
    ss1.swap(ss2);
    assert(ss1 == setset(v));
    assert(ss2 == setset(s));
  }

  void io() {
    stringstream sstr;
    setset ss;
    sstr << ss;
    assert(sstr.str() == "B\n.\n");
    ss.clear();
    sstr >> ss;
    assert(sstr.good());
    assert(ss == setset());

    sstr.clear(); sstr.str("");
    ss = setset(V("{{}}"));
    sstr << ss;
    assert(sstr.str() == "T\n.\n");
    ss.clear();
    sstr >> ss;
    assert(sstr.good());
    assert(ss == setset(V("{{}}")));

    sstr.clear(); sstr.str("");
    vector<set<int> > v = V("{{}, {1}, {1, 2}, {1, 2, 3}, {1, 2, 3, 4},"
                            "{1, 3, 4}, {1, 4}, {4}}");
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

    FILE* fp = fopen("/tmp/graphillion_", "w");
    ss.dump(fp);
    fclose(fp);
    ss.clear();
    fp = fopen("/tmp/graphillion_", "r");
    ss.load(fp);
    fclose(fp);
    assert(ss == setset(v));
  }

  void large() {
    setset::num_elems(10000);
    map<string, vector<int> > m;
    setset ss = setset(m) - setset(V("{{1}, {1, 2}}"));
    assert(ss.size().size() == 3011);
  }
};

}  // namespace graphillion

int main() {
  graphillion::TestSetset().run();
  printf("ok\n");
  return 0;
}
