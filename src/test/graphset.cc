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

class TestGraphset {
 public:
  void run() {
    // graph
    //   1 --- 2 --- 3
    //   |     |     |
    //   4 --- 5 --- 6

    // edges
    int e12 = 1;
    int e14 = 2;
    int e23 = 3;
    int e25 = 4;
    int e36 = 5;
    int e45 = 6;
    int e56 = 7;
    
    vector<edge_t> graph;
    graph.push_back(make_pair("1", "2"));
    graph.push_back(make_pair("1", "4"));
    graph.push_back(make_pair("2", "3"));
    graph.push_back(make_pair("2", "5"));
    graph.push_back(make_pair("3", "6"));
    graph.push_back(make_pair("4", "5"));
    graph.push_back(make_pair("5", "6"));

    vector<vertex_t> vertices;
    for (int v = 1; v <= 6; ++v) {
      stringstream sstr;
      sstr << v;
      vertices.push_back(sstr.str());
    }

    setset::num_elems(7);

    // any subgraph
    setset ss = FrontierSearch(graph);
    assert(ss.size() == "128");

    // subgraphs separating [1, 5] and [2]
    vector<vector<vertex_t> > vertex_groups = V("{{1, 5}, {2}}");
    ss = FrontierSearch(graph, &vertex_groups);
    assert(ss.size() == "7");
    set<int> s = S(2, e14, e45);
    assert(ss.find(s) != ss.end());
    s.insert(e12);
    assert(ss.find(s) == ss.end());

    // matching
    map<vertex_t, Range> degree_constraints;
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(0, 2);
    ss = FrontierSearch(graph, NULL, &degree_constraints);
    assert(ss.size() == "22");
    s = S(2, e12, e36);
    assert(ss.find(s) != ss.end());
    s.insert(e23);
    assert(ss.find(s) == ss.end());
    for (setset::const_iterator g = ss.begin(); g != ss.end(); ++g)
      assert((*g).size() < 4);

    // subgraphs with 1 or 2 edges
    Range num_edges(1, 3);
    ss = FrontierSearch(graph, NULL, NULL, &num_edges);
    assert(ss.size() == "28");
    for (setset::const_iterator g = ss.begin(); g != ss.end(); ++g)
      assert(1 <= (*g).size() && (*g).size() < 3);

    // single connected component and vertex islands
    ss = FrontierSearch(graph, NULL, NULL, NULL, 1);
    assert(ss.size() == "80");
    s = S(2, e12, e23);
    assert(ss.find(s) != ss.end());
    s.insert(e45);
    assert(ss.find(s) == ss.end());

    // any forest
    ss = FrontierSearch(graph, NULL, NULL, NULL, -1, true);
    assert(ss.size() == "112");
    s = S(3, e12, e14, e25);
    assert(ss.find(s) != ss.end());
    s.insert(e45);
    assert(ss.find(s) == ss.end());
    for (setset::const_iterator g = ss.begin(); g != ss.end(); ++g)
      assert((*g).size() < 6);

    // subsetting method
    ss = FrontierSearch(graph, NULL, NULL, NULL, 1, false, &ss);
    assert(ss.size() == "66");
    s = S(3, e12, e14, e25);
    assert(ss.find(s) != ss.end());
    s.insert(e45);
    assert(ss.find(s) == ss.end());

    // two clusters
    vertex_groups = V("{{1, 5}, {2}}");
    ss = FrontierSearch(graph, &vertex_groups, NULL, NULL, 0);
    assert(ss.size() == "6");
    s = S(2, e14, e45);
    assert(ss.find(s) != ss.end());
    s.insert(e36);
    assert(ss.find(s) == ss.end());

    // single connected component
    degree_constraints.clear();
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(1);
    ss = FrontierSearch(graph, NULL, &degree_constraints, NULL, 1);
    assert(ss.size() == "23");
    s = S(5, e12, e14, e23, e25, e36);
    assert(ss.find(s) != ss.end());
    s = S(5, e12, e14, e23, e25, e45);
    assert(ss.find(s) == ss.end());
    for (setset::const_iterator g = ss.begin(); g != ss.end(); ++g)
      assert((*g).size() > 4);

    // spanning tree
    ss = FrontierSearch(graph, NULL, &degree_constraints, NULL, 1, true);
    assert(ss.size() == "15");
    s = S(5, e12, e14, e23, e25, e36);
    assert(ss.find(s) != ss.end());
    s = S(5, e12, e14, e23, e25, e45);
    assert(ss.find(s) == ss.end());
    for (setset::const_iterator g = ss.begin(); g != ss.end(); ++g)
      assert((*g).size() == 5);

    // rooted forest
    vertex_groups = V("{{1}, {3}}");
    degree_constraints.clear();
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      if (*v != "1" && *v != "3")
        degree_constraints[*v] = Range(1);
    ss = FrontierSearch(graph, &vertex_groups, &degree_constraints, NULL, 0,
                        true);
    assert(ss.size() == "20");
    s = S(4, e12, e14, e25, e36);
    assert(ss.find(s) != ss.end());
    s = S(4, e12, e14, e23, e25);
    assert(ss.find(s) == ss.end());
    for (setset::const_iterator g = ss.begin(); g != ss.end(); ++g)
      assert((*g).size() == 4);

    // cycles
    degree_constraints.clear();
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(0, 3, 2);
    ss = FrontierSearch(graph, NULL, &degree_constraints);
    assert(ss.size() == "4");
    s = S(0);
    assert(ss.find(s) != ss.end());
    s = S(3, e12, e14, e23);
    assert(ss.find(s) == ss.end());

    // single cycle
    degree_constraints.clear();
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(0, 3, 2);
    ss = FrontierSearch(graph, NULL, &degree_constraints, NULL, 1);
    assert(ss.size() == "3");
    s = S(4, e12, e14, e25, e45);
    assert(ss.find(s) != ss.end());
    s = S(0);
    assert(ss.find(s) == ss.end());

    // hamilton cycle
    degree_constraints.clear();
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(2, 3);
    ss = FrontierSearch(graph, NULL, &degree_constraints, NULL, 1);
    assert(ss.size() == "1");
    s = S(6, e12, e14, e23, e36, e45, e56);
    assert(ss.find(s) != ss.end());

    // any path
    degree_constraints.clear();
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(0, 3);
    ss = FrontierSearch(graph, NULL, &degree_constraints, NULL, -1, true);
    assert(ss.size() == "95");
    s = S(4, e12, e14, e36, e45);
    assert(ss.find(s) != ss.end());
    s = S(3, e12, e23, e25);
    assert(ss.find(s) == ss.end());

    // pinned path
    vertex_groups = V("{{1, 6}}");
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v]
          = *v == "1" || *v == "6" ? Range(1, 2) : Range(0, 3, 2);
    ss = FrontierSearch(graph, &vertex_groups, &degree_constraints, NULL, 0,
                        true);
    assert(ss.size() == "4");
    s = S(3, e12, e23, e36);
    assert(ss.find(s) != ss.end());
    s = S(3, e12, e23, e56);
    assert(ss.find(s) == ss.end());

    // rooted path
    vertex_groups = V("{{1}, {6}}");
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v]
          = *v == "1" || *v == "6" ? Range(1, 2) : Range(0, 3);
    ss = FrontierSearch(graph, &vertex_groups, &degree_constraints, NULL, 0,
                        true);
    assert(ss.size() == "16");
    s = S(3, e12, e23, e56);
    assert(ss.find(s) != ss.end());
    s = S(3, e12, e23, e36);
    assert(ss.find(s) == ss.end());

    // hamilton path
    vertex_groups = V("{{1, 6}}");
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v]
          = *v == "1" || *v == "6" ? Range(1, 2) : Range(2, 3);
    ss = FrontierSearch(graph, &vertex_groups, &degree_constraints, NULL, 0,
                        true);
    assert(ss.size() == "1");
    s = S(5, e14, e23, e25, e36, e45);
    assert(ss.find(s) != ss.end());

    // graph
    //   1 --- 2 --- 3
    //   |  6  |  /
    //   4 --- 5 

    // edges
    e12 = 1;
    e14 = 2;
    int e16 = 3;
    e23 = 4;
    e25 = 5;
    int e26 = 6;
    int e35 = 7;
    e45 = 8;
    int e46 = 9;
    e56 = 10;

    graph.clear();
    graph.push_back(make_pair("1", "2"));
    graph.push_back(make_pair("1", "4"));
    graph.push_back(make_pair("1", "6"));
    graph.push_back(make_pair("2", "3"));
    graph.push_back(make_pair("2", "5"));
    graph.push_back(make_pair("2", "6"));
    graph.push_back(make_pair("3", "5"));
    graph.push_back(make_pair("4", "5"));
    graph.push_back(make_pair("4", "6"));
    graph.push_back(make_pair("5", "6"));

    vertices.clear();
    for (int v = 1; v <= 6; ++v) {
      stringstream sstr;
      sstr << v;
      vertices.push_back(sstr.str());
    }

    setset::num_elems(10);

    // edge disjoint path (XXX two paths must be crossed)
    /*degree_constraints.clear();
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v]
          = *v != "5" && *v != "6" ? Range(1, 2) : Range(0, vertices.size(), 2);
    ss = FrontierSearch(graph, NULL, &degree_constraints, NULL, 1, true);
    assert(ss.size() == "2");
    s = S(5, e16, e26, e35, e46, e56);
    assert(ss.find(s) != ss.end());*/

    // clique
    int k = 3;
    degree_constraints.clear();
    for (vector<vertex_t>::const_iterator v = vertices.begin();
         v != vertices.end(); ++v) 
      degree_constraints[*v] = Range(0, k, k - 1);
    num_edges = Range(k * (k - 1) / 2, k * (k - 1) / 2 + 1);
    ss = FrontierSearch(graph, NULL, &degree_constraints, &num_edges, 1);
    assert(ss.size() == "5");
    s = S(3, e12, e16, e26);
    assert(ss.find(s) != ss.end());
    s = S(4, e12, e14, e26, e46);
    assert(ss.find(s) == ss.end());
  }
};

}  // namespace graphillion

int main() {
  graphillion::TestGraphset().run();
  printf("ok\n");
  return 0;
}
