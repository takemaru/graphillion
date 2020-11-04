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

#include <cassert>
#include <cmath>
#include <cstdio>

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "graphillion/zdd.h"
#include "graphillion/setset.h"
#include "graphillion/util.h"

#define e0 (graphillion::top())
#define e1 (graphillion::single(1))
#define e2 (graphillion::single(2))
#define e3 (graphillion::single(3))
#define e4 (graphillion::single(4))
#define e5 (graphillion::single(5))

#define s0 (e0)
#define s1 (e1)
#define s2 (e2)
#define s3 (e3)
#define s4 (e4)
#define s12 (e1*e2)
#define s13 (e1*e3)
#define s14 (e1*e4)
#define s23 (e2*e3)
#define s24 (e2*e4)
#define s34 (e3*e4)
#define s123 (e1*e2*e3)
#define s124 (e1*e2*e4)
#define s134 (e1*e3*e4)
#define s234 (e2*e3*e4)
#define s1234 (e1*e2*e3*e4)

namespace graphillion {

using namespace std;

set<int> S(const string& str) {
  set<int> s;
  vector<string> v = split(str, "{}, ");
  for (vector<string>::const_iterator i = v.begin(); i != v.end(); ++i)
    s.insert(strtol(i->c_str(), NULL, 0));
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

#define assert_almost_equal(a, b) assert(b - 1e-6 < a && a < b + 1e-6)

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
    this->probability();
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
    assert(ss.zdd_ == s0 + s1 + s12 + s2);

    setset::num_elems(1);
    assert(setset::num_elems() == 1);

    ss = setset(m);
    assert(ss.zdd_ == s0 + s1);
  }

  void constructors() {
    setset ss;
    assert(ss.empty());

    ss = setset(V("{{}, {1,2}, {1,3}}"));
    assert(ss.zdd_ == s0 + s12 + s13);

    map<string, vector<int> > m;
    ss = setset(m);
    assert(ss.zdd_ == s0 + s1 + s2 + s3 + s12 + s13 + s23 + s123);

    m["include"].push_back(1);
    m["include"].push_back(2);
    m["exclude"].push_back(4);
    ss = setset(m);
    assert(ss.zdd_ == s12 + s123);

    // copy constructor
    ss = setset(setset(V("{{1}, {2}}")));
    assert(ss.zdd_ == s1 + s2);
  }

  void comparison() {
    setset ss(V("{{1,2}}"));
    assert(ss == setset(V("{{1,2}}")));
    assert(ss != setset(V("{{1,3}}")));

    vector<set<int> > v = V("{{}, {1,2}, {1,3}}");
    ss = setset(v);
    assert(ss.is_disjoint(setset(V("{{1}, {1,2,3}}"))));
    assert(!ss.is_disjoint(setset(V("{{1}, {1,2}}"))));

    assert(ss.is_subset(setset(v)));
    assert(!ss.is_subset(setset(V("{{}, {1,2}}"))));
    assert(ss <= setset(v));
    assert(!(ss <= setset(V("{{}, {1,2}}"))));
    assert(ss < setset(V("{{}, {1}, {1,2}, {1,3}}")));
    assert(!(ss < setset(v)));

    assert(ss.is_superset(setset(v)));
    assert(!ss.is_superset(setset(V("{{1}, {1,2}}"))));
    assert(ss >= setset(v));
    assert(!(ss >= setset(V("{{1}, {1,2}}"))));
    assert(ss > setset(V("{{}, {1,2}}")));
    assert(!(ss > setset(v)));
  }

  void unary_operators() {
    setset::num_elems(4);

    setset ss(V("{{}, {1}, {1,2}, {1,2,3}, {1,2,3,4}, {1,3,4}, {1,4}, {4}}"));
    assert((~ss).zdd_ == s124 + s13 + s2 + s23 + s234 + s24 + s3 + s34);

    assert(ss.smaller(3).zdd_ == s0 + s1 + s12 + s14 + s4);
    assert(ss.larger(3).zdd_ == s1234);
    assert(ss.set_size(3).zdd_ == s123 + s134);

    ss = setset(V("{{1,2}, {1,2,3}, {2,3,4}}"));
    assert(ss.minimal().zdd_ == s12 + s234);
    assert(ss.maximal().zdd_ == s123 + s234);

    ss = setset(V("{{1,2}, {1,4}, {2,3}, {3,4}}"));
    assert(ss.hitting().zdd_ == s123 + s1234 + s124 + s13 + s134 + s234 + s24);
  }

  void binary_operators() {
    setset::num_elems(4);

    vector<set<int> > u
        = V("{{}, {1}, {1,2}, {1,2,3}, {1,2,3,4}, {1,3,4}, {1,4}, {4}}");
    vector<set<int> > v = V("{{1,2}, {1,4}, {2,3}, {3,4}}");
    setset ss = setset(u) | setset(v);
    assert(ss.zdd_ == s0 + s1 + s12 + s123 + s1234 + s134 + s14 + s23 + s34
           + s4);

    ss = setset(u);
    ss |= setset(v);
    assert(ss.zdd_ == s0 + s1 + s12 + s123 + s1234 + s134 + s14 + s23 + s34
           + s4);

    ss = setset(u) & setset(v);
    assert(ss.zdd_ == s12 + s14);

    ss = setset(u);
    ss &= setset(v);
    assert(ss.zdd_ == s12 + s14);

    ss = setset(u) - setset(v);
    assert(ss.zdd_ == s0 + s1 + s123 + s1234 + s134 + s4);

    ss = setset(u);
    ss -= setset(v);
    assert(ss.zdd_ == s0 + s1 + s123 + s1234 + s134 + s4);

    ss = setset(u) ^ setset(v);
    assert(ss.zdd_ == s0 + s1 + s123 + s1234 + s134 + s23 + s34 + s4);

    ss = setset(u);
    ss ^= setset(v);
    assert(ss.zdd_ == s0 + s1 + s123 + s1234 + s134 + s23 + s34 + s4);

    v = V("{{1,2}}");
    ss = setset(u) / setset(v);
    assert(ss.zdd_ == s0 + s3 + s34);

    ss = setset(u);
    ss /= setset(v);
    assert(ss.zdd_ == s0 + s3 + s34);

    ss = setset(u) % setset(v);
    assert(ss.zdd_ == s0 + s1 + s134 + s14 + s4);

    ss = setset(u);
    ss %= setset(v);
    assert(ss.zdd_ == s0 + s1 + s134 + s14 + s4);

    v = V("{{1,2}, {1,4}, {2,3}, {3,4}}");
    ss = setset(u).join(setset(v));
    assert(ss.zdd_ == s12 + s123 + s124 + s1234 + s134 + s14 + s23 + s234 + s34);

    ss = setset(u).meet(setset(v));
    assert(ss.zdd_ == s0 + s1 + s12 + s14 + s2 + s23 + s3 + s34 + s4);

    v = V("{{1,2}, {1,4}, {2,3}, {3,4}}");
    ss = setset(u).subsets(setset(v));
    assert(ss.zdd_ == s0 + s1 + s12 + s14 + s4);

    ss = setset(u).supersets(setset(v));
    assert(ss.zdd_ == s12 + s123 + s1234 + s134 + s14);

    ss = setset(u).non_subsets(setset(v));
    assert(ss.zdd_ == s123 + s1234 + s134);

    ss = setset(u).non_supersets(setset(v));
    assert(ss.zdd_ == s0 + s1 + s4);

    ss = setset(V("{{}, {1,2}, {1,3}}"));
    assert(ss.supersets(1).zdd_ == s12 + s13);
    assert(ss.non_supersets(2).zdd_ == s0 + s13);
  }

  void capacity() {
    setset ss;
    assert(ss.empty());

    ss = setset(V("{{}, {1,2}, {1,3}}"));
    assert(!ss.empty());

    assert(ss.size() == "3");
  }

  void iterators() {
    setset ss(V("{{}, {1,2}, {1,3}}"));
    vector<set<int> > v;
    for (setset::iterator s = ss.begin(); s != ss.end(); ++s)
      v.push_back(*s);
    assert(v.size() == 3);
    assert(ss == setset(v));

    v.clear();
    for (setset::const_iterator s = ss.begin(); s != ss.end(); ++s)
      v.push_back(*s);
    assert(v.size() == 3);
    assert(ss == setset(v));

    ss = setset(V("{{1}, {1,2}, {1,3}}"));
    v.clear();
    for (setset::iterator s = ss.begin(); s != ss.end(); ++s)
      v.push_back(*s);
    assert(v.size() == 3);
    assert(ss == setset(v));

    v.clear();
    for (setset::random_iterator s = ss.begin_randomly(); s != ss.end(); ++s)
      v.push_back(*s);
    assert(v.size() == 3);
    assert(ss == setset(v));

    ss = setset(V("{{}}"));
    v.clear();
    for (setset::random_iterator s = ss.begin_randomly(); s != ss.end(); ++s)
      v.push_back(*s);
    assert(v.size() == 1);
    assert(ss == setset(v));

    ss = setset(V("{{}, {1}, {1,2}, {1,2,3}, {1,2,3,4}, {1,3,4}, {1,4}, {4}}"));
    vector<double> w;
    w.push_back(0);  // 1-offset
    w.push_back(.3);
    w.push_back(-.2);
    w.push_back(-.2);
    w.push_back(.4);

    v.clear();
    for (setset::weighted_iterator s = ss.begin_from_max(w); s != ss.end(); ++s)
      v.push_back(*s);
    assert(v.size() == 8);
    assert(v[0] == S("{1,4}"));
    assert(v[1] == S("{1,3,4}"));
    assert(v[2] == S("{4}"));

    v.clear();
    for (setset::weighted_iterator s = ss.begin_from_min(w); s != ss.end(); ++s)
      v.push_back(*s);
    assert(v.size() == 8);
    assert(v[0] == S("{1,2,3}"));
    assert(v[1] == S("{}"));
    assert(v[2] == S("{1,2}"));
  }

  void lookup() {
    setset ss(V("{{}, {1,2}, {1,3}}"));
    setset::const_iterator s = ss.find(S("{1,2}"));
    assert(s != ss.end());
    assert(*s == S("{1,2}"));

    assert(ss.find(S("{1}")) == ss.end());

    assert(ss.count(S("{1,2}")) == 1);
    assert(ss.count(S("{2,3}")) == 0);
  }

  void modifiers() {
    vector<set<int> > v = V("{{}, {1,2}, {1,3}}");
    setset ss(v);
    pair<setset::iterator, bool> p = ss.insert(S("{1}"));
    assert(ss.find(S("{1}")) != ss.end());
    assert(p.first != ss.end());
    assert(*(p.first) == S("{1}"));
    assert(p.second);

    p = ss.insert(S("{1}"));
    assert(p.first != ss.end());
    assert(*(p.first) == S("{1}"));
    assert(!p.second);

    setset::iterator i = ss.insert(p.first, S("{1}"));
    assert(i != ss.end());
    assert(*i == S("{1}"));

    i = ss.erase(i);
    assert(ss.find(S("{1}")) == ss.end());
    assert(i == ss.end());

    assert(ss.erase(S("{1}")) == 0);
    assert(ss.erase(S("{1,2}")) == 1);
    assert(ss.find(S("{1,2}")) == ss.end());

    ss = setset(v);
    ss.insert(2);
    assert(ss == setset(V("{{1,2}, {1,2,3}, {2}}")));

    ss = setset(v);
    ss.erase(2);
    assert(ss == setset(V("{{}, {1}, {1,3}}")));

    ss = setset(v);
    assert(!ss.empty());
    ss.clear();
    assert(ss.empty());

    vector<set<int> > u = V("{{1,2}}");
    setset ss1(u);
    setset ss2(v);
    ss1.swap(ss2);
    assert(ss1 == setset(v));
    assert(ss2 == setset(u));

    u = V("{{}, {1}, {1,2}, {1,2,3}, {1,2,3,4}, {1,3,4}, {1,4}, {4}}");
    ss = setset(u);
    ss.flip(1);
    assert(ss.zdd_ == s0 + s1 + s14 + s2 + s23 + s234 + s34 + s4);

    ss = setset(u);
    ss.flip();
    assert(ss.zdd_ == s0 + s123 + s1234 + s2 + s23 + s234 + s34 + s4);
  }

  void probability() {
    vector<double> p;
    p.push_back(0);  // 1-offset
    p.push_back(.9);
    p.push_back(.8);
    p.push_back(.7);
    p.push_back(.6);

    setset ss = setset();
    assert(ss.probability(p) == 0);

    ss = setset(V("{{}}"));
    assert_almost_equal(ss.probability(p), .0024);

    ss = setset(V("{{1}}"));
    assert_almost_equal(ss.probability(p), .0216);

    ss = setset(V("{{2}}"));
    assert_almost_equal(ss.probability(p), .0096);

    ss = setset(V("{{1,2}, {1,3}}"));
    assert_almost_equal(ss.probability(p), .1368);

    ss = setset(V("{{1,2,3,4}}"));
    assert_almost_equal(ss.probability(p), .3024);

    ss = setset(V("{{}, {1}, {2}, {1,2}, {1,3}, {1,2,3,4}}"));
    assert_almost_equal(ss.probability(p), .4728);
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
    vector<set<int> > v
        = V("{{}, {1}, {1,2}, {1,2,3}, {1,2,3,4}, {1,3,4}, {1,4}, {4}}");
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
    ss = setset::load(sstr);
    assert(ss == setset(v));

    FILE* fp = fopen("/tmp/graphillion_", "w");
    ss.dump(fp);
    fclose(fp);
    fp = fopen("/tmp/graphillion_", "r");
    ss = setset::load(fp);
    fclose(fp);
    assert(ss == setset(v));
  }

  void large() {
    int n = 1000;
    setset::num_elems(n);
    map<string, vector<int> > m;
    setset ss = setset(m) - setset(V("{{1}, {1,2}}"));
    assert(ss.size() == "10715086071862673209484250490600018105614048117055336074437503883703510511249361224931983788156958581275946729175531468251871452856923140435984577574698574803934567774824230985421074605062371141877954182153046474983581941267398767559165543946077062914571196477686542167660429831652624386837205668069374");

    int i = 0;
    for (setset::const_iterator s = ss.begin(); s != ss.end(); ++s)
      if (++i > 100) break;
  }
};

}  // namespace graphillion

int main() {
  graphillion::TestSetset().run();
  printf("ok\n");
  return 0;
}
