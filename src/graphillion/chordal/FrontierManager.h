#ifndef GRAPHILLION_FRONTIER_MANAGER_H_
#define GRAPHILLION_FRONTIER_MANAGER_H_

#include "subsetting/util/Graph.hpp"

// This class manages vertex numbers on the frontier
// and where deg/comp of each vertex is stored.
class FrontierManager {
 private:
  // input graph
  const tdzdd::Graph& graph_;

  // frontier_vss_[i] stores the vertices each of
  // which is incident to both at least one of e_0, e_1,...,e_{i-1}
  // and at least one of e_{i+1},e_{i+2},...,e_{m-1}, and also stores
  // both endpoints of e_i, where m is the number of edges.
  // Note that the definition of the frontier is different from
  // that in the paper [Kawahara+ 2017].
  // "vss" stands for "vertex set set".
  std::vector<std::vector<int> > frontier_vss_;

  // entering_vss_[i] stores the vertex numbers
  // that newly enter the frontier when processing the i-th edge.
  std::vector<std::vector<int> > entering_vss_;

  // leaving_vss_[i] stores the vertex numbers
  // that leave the frontier after the i-th edge is processed.
  std::vector<std::vector<int> > leaving_vss_;

  std::vector<std::vector<int> > remaining_vss_;

  // translate the vertex number to the position in the PodArray
  std::vector<int> vertex_to_pos_;
  std::vector<std::vector<int> > pos_to_vertex_;

  // the maximum frontier size
  int max_frontier_size_;

  void constructEnteringAndLeavingVss() {
    const int m = graph_.edgeSize();

    entering_vss_.resize(m);
    leaving_vss_.resize(m);

    // compute entering_vss_
    std::set<int> entered_vs;
    for (int i = 0; i < m; ++i) {
      const tdzdd::Graph::EdgeInfo& e = graph_.edgeInfo(i);
      if (entered_vs.count(e.v1) == 0) {
        entering_vss_[i].push_back(e.v1);
        entered_vs.insert(e.v1);
      }
      if (entered_vs.count(e.v2) == 0) {
        entering_vss_[i].push_back(e.v2);
        entered_vs.insert(e.v2);
      }
    }
    assert(static_cast<int>(entered_vs.size()) == n);

    // compute leaving_vss_
    std::set<int> leaved_vs;
    for (int i = m - 1; i >= 0; --i) {
      const tdzdd::Graph::EdgeInfo& e = graph_.edgeInfo(i);
      if (leaved_vs.count(e.v1) == 0) {
        leaving_vss_[i].push_back(e.v1);
        leaved_vs.insert(e.v1);
      }
      if (leaved_vs.count(e.v2) == 0) {
        leaving_vss_[i].push_back(e.v2);
        leaved_vs.insert(e.v2);
      }
    }
    assert(static_cast<int>(leaved_vs.size()) == n);
  }

  void construct() {
    const int n = graph_.vertexSize();
    const int m = graph_.edgeSize();
    max_frontier_size_ = 0;

    constructEnteringAndLeavingVss();

    std::vector<int> unused;
    for (int i = n - 1; i >= 0; --i) {
      unused.push_back(i);
    }

    vertex_to_pos_.resize(n + 1);
    pos_to_vertex_.resize(m);
    for (int i = 0; i < m; ++i) {
      pos_to_vertex_[i].resize(n + 1);
    }

    std::set<int> current_vs;
    for (int i = 0; i < m; ++i) {
      if (i > 0) {
        for (int j = 0; j < n + 1; ++j) {
          pos_to_vertex_[i][j] = pos_to_vertex_[i - 1][j];
        }
      }
      const std::vector<int>& entering_vs = entering_vss_[i];
      for (size_t j = 0; j < entering_vs.size(); ++j) {
        int v = entering_vs[j];
        current_vs.insert(v);
        int u = unused.back();
        unused.pop_back();
        vertex_to_pos_[v] = u;
        pos_to_vertex_[i][u] = v;
      }

      if (static_cast<int>(current_vs.size()) > max_frontier_size_) {
        max_frontier_size_ = current_vs.size();
      }

      const std::vector<int>& leaving_vs = leaving_vss_[i];

      frontier_vss_.push_back(std::vector<int>());
      std::vector<int>& vs = frontier_vss_.back();
      remaining_vss_.push_back(std::vector<int>());
      std::vector<int>& rs = remaining_vss_.back();
      for (std::set<int>::const_iterator itor = current_vs.begin();
           itor != current_vs.end(); ++itor) {
        vs.push_back(*itor);

        bool found_leaving = false;
        for (size_t j = 0; j < leaving_vs.size(); ++j) {
          int v = leaving_vs[j];
          if (v == *itor) {
            found_leaving = true;
            break;
          }
        }
        if (!found_leaving) {
          rs.push_back(*itor);
        }
      }

      for (size_t j = 0; j < leaving_vs.size(); ++j) {
        int v = leaving_vs[j];
        current_vs.erase(v);
        unused.push_back(vertex_to_pos_[v]);
      }
    }
  }

 public:
  FrontierManager(const tdzdd::Graph& graph) : graph_(graph) { construct(); }

  // This function returns the maximum frontier size.
  int getMaxFrontierSize() const { return max_frontier_size_; }

  // This function returns the vector that stores the vertex numbers
  // that newly enter the frontier when processing the (index)-th edge.
  const std::vector<int>& getEnteringVs(int index) const {
    return entering_vss_[index];
  }

  // This function returns the vector that stores the vertex numbers
  // that leave the frontier after the (index)-th edge is processed.
  const std::vector<int>& getLeavingVs(int index) const {
    return leaving_vss_[index];
  }

  // This function returns the vector that stores the vertex numbers
  // that leave the frontier after the (index)-th edge is processed.
  const std::vector<int>& getFrontierVs(int index) const {
    return frontier_vss_[index];
  }

  const std::vector<int>& getRemainingVs(int index) const {
    return remaining_vss_[index];
  }

  // This function translates the vertex number to the position
  // in the PodArray used by FrontierExampleSpec.
  int vertexToPos(int v) const { return vertex_to_pos_[v]; }

  int posToVertex(int index, int pos) const {
    return pos_to_vertex_[index][pos];
  }

  int getVerticesEnteringLevel(short v) const {
    for (size_t i = 0; i < entering_vss_.size(); ++i) {
      for (size_t j = 0; j < entering_vss_[i].size(); ++j) {
        if (entering_vss_[i][j] == v) {
          return static_cast<int>(i);
        }
      }
    }
    return -1;
  }

  int getAllVerticesEnteringLevel() const {
    int n = static_cast<int>(entering_vss_.size());

    for (int i = n - 1; i >= 0; --i) {
      if (entering_vss_[i].size() > 0) {
        return i;
      }
    }
    return -1;
  }
};

#endif  // GRAPHILLION_FRONTIER_MANAGER_H_
