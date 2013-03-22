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
#include <cstdarg>
#include <cstdio>

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "graphillion/graphset.h"
#include "graphillion/setset.h"
#include "graphillion/util.h"

namespace graphillion {

using namespace std;

set<int> S(int n, ...) {
  va_list args;
  set<int> s;
  va_start(args, n);
  for (int i = 0; i < n; ++i)
    s.insert(va_arg(args, int));
  va_end(args);
  return s;
}

vector<vertex_t> _V(const string& str) {
  vector<vertex_t> u;
  vector<string> v = split(str, "{}, ");
  for (vector<string>::const_iterator i = v.begin(); i != v.end(); ++i)
    u.push_back(*i);
  return u;
}

vector<vector<vertex_t> > V(const string& str) {
  char* buf = new char[str.size() + 1];
  strcpy(buf, str.c_str());
  vector<vector<vertex_t> > v;
  int begin = 0;
  for (int i = 0; i < static_cast<int>(str.size()); ++i) {
    if (buf[i] == '{') {
      begin = i + 1;
    } else if (buf[i] == '}') {
      buf[i] = '\0';
      v.push_back(_V(&buf[begin]));
    }
  }
  delete [] buf;
  return v;
}

int e12 = 0;
int e13 = 0;
int e14 = 0;
int e15 = 0;
int e23 = 0;
int e24 = 0;
int e25 = 0;
int e34 = 0;
int e35 = 0;
int e36 = 0;
int e45 = 0;
int e56 = 0;

vector<edge_t> graph;
vector<vertex_t> vertices;

void setup() {
  // 1 --- 2 --- 3
  // |     |     |
  // 4 --- 5 --- 6

  e12 = 1;
  e14 = 2;
  e23 = 3;
  e25 = 4;
  e36 = 5;
  e45 = 6;
  e56 = 7;

  graph.clear();
  graph.push_back(make_pair("1", "2"));
  graph.push_back(make_pair("1", "4"));
  graph.push_back(make_pair("2", "3"));
  graph.push_back(make_pair("2", "5"));
  graph.push_back(make_pair("3", "6"));
  graph.push_back(make_pair("4", "5"));
  graph.push_back(make_pair("5", "6"));

  vertices.clear();
  for (int v = 1; v <= 6; ++v) {
    stringstream sstr;
    sstr << v;
    vertices.push_back(sstr.str());
  }

  setset::num_elems(7);
}

void setup_clique() {  // 5-clique
  e12 = 1;
  e13 = 2;
  e14 = 3;
  e15 = 4;
  e23 = 5;
  e24 = 6;
  e25 = 7;
  e34 = 8;
  e35 = 9;
  e45 = 10;

  graph.clear();
  graph.push_back(make_pair("1", "2"));
  graph.push_back(make_pair("1", "3"));
  graph.push_back(make_pair("1", "4"));
  graph.push_back(make_pair("1", "5"));
  graph.push_back(make_pair("2", "3"));
  graph.push_back(make_pair("2", "4"));
  graph.push_back(make_pair("2", "5"));
  graph.push_back(make_pair("3", "4"));
  graph.push_back(make_pair("3", "5"));
  graph.push_back(make_pair("4", "5"));

  vertices.clear();
  for (int v = 1; v <= 5; ++v) {
    stringstream sstr;
    sstr << v;
    vertices.push_back(sstr.str());
  }

  setset::num_elems(10);
}

void setup_large(int n) {
  // n x n grid

  graph.clear();
  for (int v = 1; v <= n * n; ++v) {
    stringstream sstr1;
    sstr1 << v;
    if (v % n != 0) {
      stringstream sstr2;
      sstr2 << v + 1;
      graph.push_back(make_pair(sstr1.str(), sstr2.str()));
    }
    if (v <= (n - 1) * n) {
      stringstream sstr2;
      sstr2 << v + n;
      graph.push_back(make_pair(sstr1.str(), sstr2.str()));
    }
  }

  vertices.clear();
  for (int v = 1; v <= n * n; ++v) {
    stringstream sstr;
    sstr << v;
    vertices.push_back(sstr.str());
  }

  setset::num_elems(graph.size());
}

class TestGraphSet {
 public:
  void run() {
    this->any_subgraphs();
    this->two_clusters();
    this->matchings();
    this->small_subgraphs();
    this->single_components();
    this->any_forests();
    this->constrained_by_setset();
    this->two_clusters_only();
    this->single_components_only();
    this->cliques();
    this->spanning_trees();
    this->rooted_forests();
    this->cycles();
    this->single_cycles();
    this->hamilton_cycles();
    this->any_paths();
    this->pinned_paths();
    this->rooted_paths();
    this->hamilton_paths();
    this->large();
  }

  void any_subgraphs() {
    setup();
    setset ss = SearchGraphs(graph);
    assert(ss.size() == "128");
    assert(ss.find(S(1, e12)) != ss.end());
  }

  void two_clusters() {  // subgraphs separating [1, 5] and [2]
    setup();
    vector<vector<vertex_t> > vertex_groups = V("{{1, 5}, {2}}");
    setset ss = SearchGraphs(graph, &vertex_groups);
    assert(ss.size() == "7");
    assert(ss.find(S(2, e14, e45)) != ss.end());
    assert(ss.find(S(3, e12, e14, e45)) == ss.end());
  }

  void matchings() {
    setup();
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(0, 2);
    setset ss = SearchGraphs(graph, NULL, &degree_constraints);
    assert(ss.size() == "22");
    assert(ss.find(S(2, e12, e36)) != ss.end());
    assert(ss.find(S(3, e12, e23, e36)) == ss.end());
    for (setset::const_iterator g = ss.begin(); g != ss.end(); ++g)
      assert((*g).size() < 4);
  }

  void small_subgraphs() {  // subgraphs with 1 or 2 edges
    setup();
    Range num_edges(1, 3);
    setset ss = SearchGraphs(graph, NULL, NULL, &num_edges);
    assert(ss.size() == "28");
    for (setset::const_iterator g = ss.begin(); g != ss.end(); ++g)
      assert(1 <= (*g).size() && (*g).size() < 3);
  }

  void single_components() {  // with vertex islands
    setup();
    setset ss = SearchGraphs(graph, NULL, NULL, NULL, 1);
    assert(ss.size() == "80");
    assert(ss.find(S(2, e12, e23)) != ss.end());
    assert(ss.find(S(3, e12, e23, e45)) == ss.end());
  }

  void any_forests() {
    setup();
    setset ss = SearchGraphs(graph, NULL, NULL, NULL, -1, true);
    assert(ss.size() == "112");
    assert(ss.find(S(3, e12, e14, e25)) != ss.end());
    assert(ss.find(S(4, e12, e14, e25, e45)) == ss.end());
    for (setset::const_iterator g = ss.begin(); g != ss.end(); ++g)
      assert((*g).size() < 6);
  }

  void constrained_by_setset() {  // subsetting method
    setup();
    setset ss = SearchGraphs(graph, NULL, NULL, NULL, -1, true);  // any forest
    ss = SearchGraphs(graph, NULL, NULL, NULL, 1, false, &ss);
    assert(ss.size() == "66");
    assert(ss.find(S(3, e12, e14, e25)) != ss.end());
    assert(ss.find(S(4, e12, e14, e25, e45)) == ss.end());
  }

  void two_clusters_only() {
    setup();
    vector<vector<vertex_t> > vertex_groups = V("{{1, 5}, {2}}");
    setset ss = SearchGraphs(graph, &vertex_groups, NULL, NULL, 0);
    assert(ss.size() == "6");
    assert(ss.find(S(2, e14, e45)) != ss.end());
    assert(ss.find(S(3, e14, e36, e45)) == ss.end());
  }

  void single_components_only() {
    setup();
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(1, vertices.size());
    setset ss = SearchGraphs(graph, NULL, &degree_constraints, NULL, 1);
    assert(ss.size() == "23");
    assert(ss.find(S(5, e12, e14, e23, e25, e36)) != ss.end());
    assert(ss.find(S(5, e12, e14, e23, e25, e45)) == ss.end());
    for (setset::const_iterator g = ss.begin(); g != ss.end(); ++g)
      assert((*g).size() > 4);
  }

  void cliques() {
    setup_clique();
    int k = 4;
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(0, k, k - 1);
    Range num_edges(k * (k - 1) / 2, k * (k - 1) / 2 + 1);
    setset ss = SearchGraphs(graph, NULL, &degree_constraints, &num_edges, 1);
    assert(ss.size() == "5");
    assert(ss.find(S(6, e12, e13, e14, e23, e24, e34)) != ss.end());
    assert(ss.find(S(6, e12, e13, e14, e23, e24, e35)) == ss.end());
  }

  void spanning_trees() {
    setup();
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(1, vertices.size());
    setset ss = SearchGraphs(graph, NULL, &degree_constraints, NULL, 1, true);
    assert(ss.size() == "15");
    assert(ss.find(S(5, e12, e14, e23, e25, e36)) != ss.end());
    assert(ss.find(S(5, e12, e14, e23, e25, e45)) == ss.end());
    for (setset::const_iterator g = ss.begin(); g != ss.end(); ++g)
      assert((*g).size() == 5);
  }

  void rooted_forests() {
    setup();
    vector<vector<vertex_t> > vertex_groups = V("{{1}, {3}}");
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      if (*v != "1" && *v != "3")
        degree_constraints[*v] = Range(1, vertices.size());
    setset ss = SearchGraphs(graph, &vertex_groups, &degree_constraints, NULL,
                             0, true);
    assert(ss.size() == "20");
    assert(ss.find(S(4, e12, e14, e25, e36)) != ss.end());
    assert(ss.find(S(4, e12, e14, e23, e25)) == ss.end());
    for (setset::const_iterator g = ss.begin(); g != ss.end(); ++g)
      assert((*g).size() == 4);
  }

  void cycles() {
    setup();
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(0, 3, 2);
    setset ss = SearchGraphs(graph, NULL, &degree_constraints);
    assert(ss.size() == "4");
    assert(ss.find(S(0)) != ss.end());
    assert(ss.find(S(3, e12, e14, e23)) == ss.end());
  }

  void single_cycles() {
    setup();
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(0, 3, 2);
    setset ss = SearchGraphs(graph, NULL, &degree_constraints, NULL, 1);
    assert(ss.size() == "3");
    assert(ss.find(S(4, e12, e14, e25, e45)) != ss.end());
    assert(ss.find(S(0)) == ss.end());
  }

  void hamilton_cycles() {
    setup();
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(2, 3);
    setset ss = SearchGraphs(graph, NULL, &degree_constraints, NULL, 1);
    assert(ss.size() == "1");
    assert(ss.find(S(6, e12, e14, e23, e36, e45, e56)) != ss.end());
  }

  void any_paths() {
    setup();
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(0, 3);
    setset ss = SearchGraphs(graph, NULL, &degree_constraints, NULL, -1, true);
    assert(ss.size() == "95");
    assert(ss.find(S(4, e12, e14, e36, e45)) != ss.end());
    assert(ss.find(S(3, e12, e23, e25)) == ss.end());
  }

  void pinned_paths() {
    setup();
    vector<vector<vertex_t> > vertex_groups = V("{{1, 6}}");
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v]
          = *v == "1" || *v == "6" ? Range(1, 2) : Range(0, 3, 2);
    setset ss = SearchGraphs(graph, &vertex_groups, &degree_constraints, NULL,
                             0, true);
    assert(ss.size() == "4");
    assert(ss.find(S(3, e12, e23, e36)) != ss.end());
    assert(ss.find(S(3, e12, e23, e56)) == ss.end());
  }

  void rooted_paths() {
    setup();
    vector<vector<vertex_t> > vertex_groups = V("{{1}, {6}}");
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v]
          = *v == "1" || *v == "6" ? Range(1, 2) : Range(0, 3);
    setset ss = SearchGraphs(graph, &vertex_groups, &degree_constraints, NULL,
                             0, true);
    assert(ss.size() == "16");
    assert(ss.find(S(3, e12, e23, e56)) != ss.end());
    assert(ss.find(S(3, e12, e23, e36)) == ss.end());
  }

  void hamilton_paths() {
    setup();
    vector<vector<vertex_t> > vertex_groups = V("{{1, 6}}");
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v]
          = *v == "1" || *v == "6" ? Range(1, 2) : Range(2, 3);
    setset ss = SearchGraphs(graph, &vertex_groups, &degree_constraints, NULL,
                             0, true);
    assert(ss.size() == "1");
    assert(ss.find(S(5, e14, e23, e25, e36, e45)) != ss.end());
  }

  void large() {
    setup_large(8);
    vector<vector<vertex_t> > vertex_groups = V("{{1, 64}}");
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v)
      degree_constraints[*v]
          = *v == "1" || *v == "64" ? Range(1, 2) : Range(0, 3, 2);
    setset ss = SearchGraphs(graph, &vertex_groups, &degree_constraints, NULL,
                             0, true);
    assert(ss.size() == "789360053252");
  }
};

}  // namespace graphillion

int main() {
  graphillion::TestGraphSet().run();
  printf("ok\n");
  return 0;
}
