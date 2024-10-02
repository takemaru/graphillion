#ifndef GRAPHILLION_INDUCING_COLORING_SPEC_H_
#define GRAPHILLION_INDUCING_COLORING_SPEC_H_

#include "subsetting/DdEval.hpp"
#include "subsetting/DdSpec.hpp"
#include "subsetting/DdSpecOp.hpp"
#include "subsetting/DdStructure.hpp"
#include "subsetting/util/Graph.hpp"

#include "FrontierManager.h"

class InducingColoringSpec
    : public tdzdd::HybridDdSpec<InducingColoringSpec, tdzdd::NodeId, short,
                                 3> {
 private:
  const tdzdd::Graph& graph_;
  const tdzdd::DdStructure<2>& dd_;
  const int m_;

  const FrontierManager fm_;

  // pair: first -> edge index, second -> adjacent node index
  std::vector<std::vector<std::pair<short, short> > > adj_list_;

  short getValue(short* data, short v) const {
    return data[fm_.vertexToPos(v)];
  }

  void setValue(short* data, short v, short c) const {
    data[fm_.vertexToPos(v)] = c;
  }

  bool useVertex(short u, int edge_index, const tdzdd::Graph::EdgeInfo& e,
                 short* data) const {
    for (size_t j = 0; j < adj_list_[u].size(); ++j) {
      const std::pair<short, short>& adj = adj_list_[u][j];
      int w = adj.second;
      const std::vector<int>& frontier_vs = fm_.getFrontierVs(edge_index);

      // check whether w is in frontier or not
      bool w_is_in_frontier = false;
      for (size_t k = 0; k < frontier_vs.size(); ++k) {
        int v = frontier_vs[k];
        if (v == w) {
          w_is_in_frontier = true;
          break;
        }
      }

      if (w_is_in_frontier) {
        if (adj.first < edge_index) {
          if (getValue(data, w) >= 1) {
            return false;
          } else {
            setValue(data, w, -1);
          }
        }
      }
    }
    return true;
  }

 public:
  InducingColoringSpec(const tdzdd::Graph& graph,
                       const tdzdd::DdStructure<2>& dd)
      : graph_(graph), dd_(dd), m_(graph.edgeSize()), fm_(graph) {
    setArraySize(fm_.getMaxFrontierSize());

    adj_list_.resize(graph.vertexSize() + 1);
    for (short i = 0; i < m_; ++i) {
      const tdzdd::Graph::EdgeInfo& e = graph_.edgeInfo(i);
      adj_list_[e.v1].push_back(std::make_pair(i, e.v2));
      adj_list_[e.v2].push_back(std::make_pair(i, e.v1));
    }
  }

  int getRoot(tdzdd::NodeId& node, short* data) const {
    node = dd_.root();
    for (int i = 0; i < fm_.getMaxFrontierSize(); ++i) {
      data[i] = 0;
    }
    return m_;
  }

  int getChild(tdzdd::NodeId& node, short* data, int level, int take) const {
    int edge_index = m_ - level;
    tdzdd::Graph::EdgeInfo const& edge = graph_.edgeInfo(edge_index);

    // initialize value of the vertices newly entering the frontier
    const std::vector<int>& entering_vs = fm_.getEnteringVs(edge_index);
    for (size_t i = 0; i < entering_vs.size(); ++i) {
      int v = entering_vs[i];
      // initially the value of deg is 0
      setValue(data, v, 0);
    }

    short w1 = getValue(data, edge.v1);
    short w2 = getValue(data, edge.v2);

    if (node.row() == level) {
      node = dd_.child(node, (take == 1 ? 1 : 0));
      if (node == 0) {  // 0-terminal
        return 0;
      }
    } else {
      assert(node.row() < level);
      if (take == 1) {
        return 0;
      }
    }

    if (take == 0) {
      if (w1 >= 1 && w2 >= 1) {
        return 0;
      } else if (w1 >= 1) {
        setValue(data, edge.v2, -1);
      } else if (w2 >= 1) {
        setValue(data, edge.v1, -1);
      }
    } else if (take == 1) {
      if (w1 == -1 || w2 == -1) {
        return 0;
      }
      if (w1 == 0) {
        if (!useVertex(edge.v1, edge_index, edge, data)) {
          return 0;
        }
      }
      setValue(data, edge.v1, 2);
      if (w2 == 0) {
        if (!useVertex(edge.v2, edge_index, edge, data)) {
          return 0;
        }
      }
      setValue(data, edge.v2, 2);
    } else if (take == 2) {
      if (w1 == -1 || w2 == -1) {
        return 0;
      }
      if (w1 == 0) {
        if (!useVertex(edge.v1, edge_index, edge, data)) {
          return 0;
        }
        setValue(data, edge.v1, 1);
      }
      if (w2 == 0) {
        if (!useVertex(edge.v2, edge_index, edge, data)) {
          return 0;
        }
        setValue(data, edge.v2, 1);
      }
    } else {
      assert(false);
    }

    // vertices that are leaving the frontier
    const std::vector<int>& leaving_vs = fm_.getLeavingVs(edge_index);
    for (size_t i = 0; i < leaving_vs.size(); ++i) {
      int v = leaving_vs[i];
      if (getValue(data, v) == 1) {
        return 0;
      }
      setValue(data, v, -1);
    }

    if (level == 1) {  // terminal
      if (node == 0) {
        return 0;
      } else {
        assert(node == 1);
        return -1;
      }
    } else {
      return level - 1;
    }
  }
};

#endif  // GRAPHILLION_INDUCING_COLORING_SPEC_H_
