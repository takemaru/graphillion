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

#include "variable_converter.h"

#include <vector>

namespace VariableConverter {

struct ZDDEVSpecConf {
  tdzdd::NodeId node;
};

VariableList::VariableList(const tdzdd::Graph& graph) : m_(graph.edgeSize()),
                      n_(graph.vertexSize()) {
  constructEVArray(graph);
}

void VariableList::constructEVArray(const tdzdd::Graph& graph) {
  int pos = m_ + n_;
  int new_n = n_;
  kind_list_.resize(m_ + n_ + 1);
  variable_number_list_.resize(m_ + n_ + 1);
  ev_to_newv_.resize(m_ + n_ + 1);
  v_to_newv_.resize(n_ + 1);
  newv_to_v_.resize(n_ + 1);
  for (int i = 0; i < m_; ++i) {
    const tdzdd::Graph::EdgeInfo& e = graph.edgeInfo(i);
    kind_list_[pos] = Kind::EDGE;
    variable_number_list_[pos] = i;  // edge number
    --pos;
    if (e.v1final) {
      kind_list_[pos] = Kind::VERTEX;
      variable_number_list_[pos] = e.v1;  // vertex number
      ev_to_newv_[pos] = new_n;
      v_to_newv_[e.v1] = new_n;
      newv_to_v_[new_n] = e.v1;
      --pos;
      --new_n;
    }
    if (e.v2final) {
      kind_list_[pos] = Kind::VERTEX;
      variable_number_list_[pos] = e.v2;  // vertex number
      ev_to_newv_[pos] = new_n;
      v_to_newv_[e.v2] = new_n;
      newv_to_v_[new_n] = e.v2;
      --pos;
      --new_n;
    }
  }
  assert(pos == 0);
}

VariableList::Kind VariableList::getKind(int evindex) const {
  return kind_list_[evindex];
}

int VariableList::getVariableNumber(int evindex) const {
  return variable_number_list_[evindex];
}

int VariableList::evToNewV(int evindex) const {
  return ev_to_newv_[evindex];
}

int VariableList::newVToV(int newv) const {
  return newv_to_v_[newv];
}


class ZDDEVSpec : public tdzdd::PodHybridDdSpec<ZDDEVSpec, ZDDEVSpecConf,
                        uint16_t, 2> {
 private:
  const tdzdd::Graph& graph_;
  tdzdd::DdStructure<2> dd_;
  const VariableList& vlist_;
  const int stateSize_;
  const int n_;
  const int m_;

 public:
  ZDDEVSpec(tdzdd::DdStructure<2> dd, const tdzdd::Graph& graph,
      const VariableList& vlist);
  size_t hashCode(const ZDDEVSpecConf& conf) const;
  int getRoot(ZDDEVSpecConf& conf, uint16_t* state) const;
  int getChild(ZDDEVSpecConf& conf, uint16_t* state, int level,
              int value) const;
};

ZDDEVSpec::ZDDEVSpec(tdzdd::DdStructure<2> dd, const tdzdd::Graph& graph,
      const VariableList& vlist)
  : graph_(graph), dd_(dd), vlist_(vlist),
    stateSize_(graph.vertexSize() + 1),
    n_(graph.vertexSize()),
    m_(graph.edgeSize()) {
  this->setArraySize(stateSize_);
}

size_t ZDDEVSpec::hashCode(const ZDDEVSpecConf& conf) const {
  return conf.node.hash();
}

int ZDDEVSpec::getRoot(ZDDEVSpecConf& conf, uint16_t* state) const {
  conf.node = dd_.root();
  for (int i = 0; i < stateSize_; ++i) {
    state[i] = 0;
  }
  return ((conf.node == 1) ? -1 : m_ + n_);
}

int ZDDEVSpec::getChild(ZDDEVSpecConf& conf, uint16_t* state, int level,
              int value) const {
  if (vlist_.getKind(level) == VariableList::Kind::EDGE) {
    int e_num = vlist_.getVariableNumber(level);  // edge number
    if (conf.node.row() < m_ - e_num) {
      if (value != 0) {  // zero-suppressed rule
        return 0;
      }
    } else {
      if (conf.node.row() > 0) {  // non-terminal
        conf.node = dd_.child(conf.node, value);
      }
      if (conf.node == 0) {  // arrived at 0-terminal
        return 0;
      }
    }

    const tdzdd::Graph::EdgeInfo& edge =
      graph_.edgeInfo(e_num);

    if (value == 1) {  // update state
      state[edge.v1] = 1;
      state[edge.v2] = 1;
    }
  } else {
    assert(vlist_.getKind(level) == VariableList::Kind::VERTEX);
    int v = vlist_.getVariableNumber(level);  // vertex number
    assert(1 <= v && v <= n_);
    if ((state[v] != 0) != (value != 0)) {
      return 0;
    } else {
      state[v] = 0;
    }
  }
  return (level - 1 > 0 ? level - 1 : -1);
}

class ArrangeESpec : public tdzdd::DdSpec<ArrangeESpec,
                      tdzdd::NodeId,
                      2> {
 private:
  tdzdd::DdStructure<2> dd_;
  const VariableList& vlist_;

 public:
  ArrangeESpec(tdzdd::DdStructure<2> dd,
              const VariableList& vlist);
  int getRoot(tdzdd::NodeId& node) const;
  int getChild(tdzdd::NodeId& node, int level, int value) const;
};

ArrangeESpec::ArrangeESpec(tdzdd::DdStructure<2> dd,
              const VariableList& vlist)
  : dd_(dd), vlist_(vlist) { }

int ArrangeESpec::getRoot(tdzdd::NodeId& node) const {
  node = dd_.root();
  if (node.row() > 0) {
    return vlist_.evToNewV(node.row());
  } else {
    return -node.col();
  }
}

int ArrangeESpec::getChild(tdzdd::NodeId& node, int level, int value) const {
  tdzdd::NodeId child = dd_.child(node, value);
  if (child.row() > 0) {
    node = child;
    return vlist_.evToNewV(node.row());
  } else {
    return -child.col();
  }
}

class EVToVEval : public tdzdd::DdEval<EVToVEval, ZBDD> {
 private:
  const VariableList& vlist_;
  const int offset_;

 public:
  EVToVEval(const VariableList& vlist, const int offset)
    : vlist_(vlist), offset_(offset) { }

  void evalTerminal(ZBDD& zbdd, int id) const;

  void evalNode(ZBDD& zbdd, int level,
          const tdzdd::DdValues<ZBDD, 2>& values) const;
};

void EVToVEval::evalTerminal(ZBDD& zbdd, int id) const {
  zbdd = ZBDD(id);
}

void EVToVEval::evalNode(ZBDD& zbdd, int level,
        const tdzdd::DdValues<ZBDD, 2>& values) const {
  ZBDD z0 = values.get(0);
  ZBDD z1 = values.get(1);

  if (vlist_.getKind(level) == VariableList::Kind::EDGE) {
    zbdd = z0 + z1;
  } else {
    assert(vlist_.getKind(level) == VariableList::Kind::VERTEX);
    zbdd = z0 + z1.Change(BDD_VarOfLev(vlist_.evToNewV(level) + offset_));
  }
}

tdzdd::DdStructure<2> eToEvZdd(const tdzdd::DdStructure<2>& dd,
                    const tdzdd::Graph& graph,
                    const VariableList& vlist) {
  ZDDEVSpec spec(dd, graph, vlist);
  tdzdd::DdStructure<2> out_dd(spec);
  return out_dd;
}

ZBDD eToVZdd(const tdzdd::DdStructure<2>& dd,
          const tdzdd::Graph& graph) {
  VariableList vlist(graph);
  return eToVZdd(dd, graph, vlist);
}

ZBDD eToVZdd(const tdzdd::DdStructure<2>& dd,
          const tdzdd::Graph& graph,
          const VariableList& vlist,
          const int offset) {
  tdzdd::DdStructure<2> ev_dd = eToEvZdd(dd, graph, vlist);
  return ev_dd.evaluate(EVToVEval(vlist, offset));
}

std::pair<tdzdd::Graph, VariableList> construct_graph_and_vlist(
  const std::vector<std::vector<std::string>> &edges_from_top
) {
  tdzdd::Graph graph;
  for (std::vector<std::string> edge : edges_from_top) graph.addEdge(edge[0], edge[1]);
  graph.update();
  VariableList vlist(graph);
  return {graph, vlist};
}

std::vector<std::string> get_vertices_from_top(
  const std::vector<std::vector<std::string>> &edges_from_top
) {
  std::pair<tdzdd::Graph, VariableList> graph_and_vlist =
    construct_graph_and_vlist(edges_from_top);
  tdzdd::Graph graph = graph_and_vlist.first;
  VariableList vlist = graph_and_vlist.second;

  std::vector<std::string> vertices_from_top(graph.vertexSize());
  std::vector<std::string>::reverse_iterator rev_itor =
    vertices_from_top.rbegin();
  for (int i = 1; i <= graph.vertexSize(); ++i) {
    int v = vlist.newVToV(i);
    *(rev_itor + i - 1) = graph.vertexName(v);
  }
  return vertices_from_top;
}

}  // namespace VariableConverter
