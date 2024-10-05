#ifndef DEGREE_DISTRIBUTION_SPEC_H
#define DEGREE_DISTRIBUTION_SPEC_H

#include <vector>
#include <climits>

#include "subsetting/DdSpec.hpp"
#include "graphillion/forbidden_induced/FrontierManager.h"
#include "subsetting/util/IntSubset.hpp"

typedef unsigned char uchar;
typedef unsigned char DSData;

const int DSData_MAX = UCHAR_MAX;

class DegreeDistributionSpec
  : public tdzdd::PodArrayDdSpec<DegreeDistributionSpec, DSData, 2> {
private:
  // input graph
  const tdzdd::Graph& graph_;
  // number of vertices
  const int n_;
  // number of edges
  const int m_;

  // make subgraphs connected or not
  const bool is_connected_;

  const FrontierManager fm_;

  const int fixedDegStart_;
  const std::vector<tdzdd::IntSubset*> degRanges_;
  const std::vector<bool> storingList_;

  // This function gets deg of v.
  int getDeg(DSData* data, int v) const {
    return static_cast<int>(data[is_connected_ ?
                  (fm_.vertexToPos(v) * 2) :
                  fm_.vertexToPos(v)]);
  }

  // This function sets deg of v to be d.
  void setDeg(DSData* data, int v, int d) const {
    data[is_connected_ ?
      (fm_.vertexToPos(v) * 2) :
      fm_.vertexToPos(v)] = static_cast<uchar>(d);
  }

  // This function gets comp of v.
  int getComp(DSData* data, int v, int index) const {
    assert(is_connected_);
    return fm_.posToVertex(index, data[fm_.vertexToPos(v) * 2 + 1]);
  }

  // This function sets comp of v to be c.
  void setComp(DSData* data, int v, int c) const {
    assert(is_connected_);
    data[fm_.vertexToPos(v) * 2 + 1] =
      static_cast<uchar>(fm_.vertexToPos(c));
  }

  void clearComp(DSData* data, int v) const {
    assert(is_connected_);
    data[fm_.vertexToPos(v) * 2 + 1] =
      static_cast<uchar>(-1);
  }

  void incrementFixedDeg(DSData* data, int d) const {
    ++data[fixedDegStart_ + d];
  }

  void addFixedDeg(DSData* data, int d, int value) const {
    data[fixedDegStart_ + d] += value;
  }

  bool checkFixedDegUpper(DSData* data, int d) const {
    return (data[fixedDegStart_ + d] < degRanges_[d]->upperBound());
  }

  bool checkFixedDeg(DSData* data) const {
    for (size_t deg = 0; deg < degRanges_.size(); ++deg) {
      if (!degRanges_[deg]->contains(data[fixedDegStart_ + deg])) {
        return false;
      }
    }
    return true;
  }

  int getDegUpper(DSData* data) const {
    int deg;
    for (deg = static_cast<int>(degRanges_.size()) - 1; deg >= 0; --deg) {
      if (data[fixedDegStart_ + deg] < degRanges_[deg]->upperBound()) {
        break;
      }
    }
    return deg;
  }

  void initializeData(DSData* data) const {
    for (int i = 0; i < fixedDegStart_ + static_cast<int>(degRanges_.size()); ++i) {
      data[i] = 0;
    }
  }

  std::vector<bool> getStoringList(const std::vector<tdzdd::IntSubset*>& degRanges) const {
    std::vector<bool> storingList;
    for (size_t i = 0; i < degRanges.size(); ++i) {
      if (degRanges[i]->lowerBound() == 0 &&
        degRanges[i]->upperBound() >= n_) {
        storingList.push_back(false);
      } else {
        storingList.push_back(true);
      }
    }
    return storingList;
  }

public:
  DegreeDistributionSpec(const tdzdd::Graph& graph,
                const std::vector<tdzdd::IntSubset*>& degRanges,
                const bool is_connected)
    : graph_(graph),
      n_(static_cast<short>(graph_.vertexSize())),
      m_(graph_.edgeSize()),
      is_connected_(is_connected),
      fm_(graph_),
      fixedDegStart_(is_connected ? (fm_.getMaxFrontierSize() * 2) :
                      fm_.getMaxFrontierSize()),
      degRanges_(degRanges),
      storingList_(getStoringList(degRanges))
  {
    if (graph_.vertexSize() > SHRT_MAX) { // SHRT_MAX == 32767
      std::cerr << "The number of vertices should be at most "
            << SHRT_MAX << std::endl;
      exit(1);
    }

    if (degRanges.size() > DSData_MAX + 1) {
      std::cerr << "The size of array degRanges should be at most "
            << (DSData_MAX + 1) << std::endl;
      exit(1);
    }

    setArraySize(fixedDegStart_ + degRanges_.size());
  }

  int getRoot(DSData* data) const {
    initializeData(data);
    return m_;
  }

  int getChild(DSData* data, int level, int value) const {
    assert(1 <= level && level <= m_);

    // edge index (starting from 0)
    const int edge_index = m_ - level;
    // edge that we are processing.
    // The endpoints of "edge" are edge.v1 and edge.v2.
    const tdzdd::Graph::EdgeInfo& edge = graph_.edgeInfo(edge_index);

    // initialize deg and comp of the vertices newly entering the frontier
    const std::vector<int>& entering_vs = fm_.getEnteringVs(edge_index);
    for (size_t i = 0; i < entering_vs.size(); ++i) {
      int v = entering_vs[i];
      // initially the value of deg is 0
      setDeg(data, v, 0);
      if (is_connected_) {
        // initially the value of comp is the vertex number itself
        setComp(data, v, v);
      }
    }

    // vertices on the frontier
    const std::vector<int>& frontier_vs = fm_.getFrontierVs(edge_index);

    if (value == 1) { // if we take the edge (go to 1-arc)
      // increment deg of v1 and v2 (recall that edge = {v1, v2})

      int upper = getDegUpper(data);
      if (getDeg(data, edge.v1) + 1 > upper) {
        return 0;
      }
      if (getDeg(data, edge.v2) + 1 > upper) {
        return 0;
      }
      if (getDeg(data, edge.v1) >= DSData_MAX ||
        getDeg(data, edge.v2) >= DSData_MAX) {
        std::cerr << "The degree exceeded "
          << DSData_MAX << "." << std::endl;
      }
      setDeg(data, edge.v1, getDeg(data, edge.v1) + 1);
      setDeg(data, edge.v2, getDeg(data, edge.v2) + 1);

      if (is_connected_) {
        short c1 = getComp(data, edge.v1, edge_index);
        short c2 = getComp(data, edge.v2, edge_index);
        if (c1 != c2) { // connected components c1 and c2 become connected
          short cmin = std::min(c1, c2);
          short cmax = std::max(c1, c2);

          // replace component number cmin with cmax
          for (size_t i = 0; i < frontier_vs.size(); ++i) {
            int v = frontier_vs[i];
            if (getComp(data, v, edge_index) == cmin) {
              setComp(data, v, cmax);
            }
          }
        }
      }
    }

    // vertices that are leaving the frontier
    const std::vector<int>& leaving_vs = fm_.getLeavingVs(edge_index);
    for (size_t i = 0; i < leaving_vs.size(); ++i) {
      int v = leaving_vs[i];

      int d = getDeg(data, v);
      if (!checkFixedDegUpper(data, d)) {
        return 0;
      }
      if (storingList_[d]) {
        incrementFixedDeg(data, d);
      }

      if (is_connected_) {
        bool samecomp_found = false;
        bool nonisolated_found = false;

        // Search a vertex that has the component number same as that of v.
        // Also check whether a vertex whose degree is at least 1 exists
        // on the frontier.
        for (size_t j = 0; j < frontier_vs.size(); ++j) {
          int w = frontier_vs[j];
          if (w == v) { // skip if w is the leaving vertex
            continue;
          }
          // skip if w is one of the vertices that
          // has already left the frontier
          bool found_left = false;
          for (size_t k = 0; k < i; ++k) {
            if (w == leaving_vs[k]) {
              found_left = true;
              break;
            }
          }
          if (found_left) {
            continue;
          }
          // w has the component number same as that of v
          if (getComp(data, w, edge_index) == getComp(data, v, edge_index)) {
            samecomp_found = true;
          }
          // The degree of w is at least 1.
          if (getDeg(data, w) > 0) {
            nonisolated_found = true;
          }
          if (nonisolated_found && samecomp_found) {
            break;
          }
        }
        // There is no vertex that has the component number
        // same as that of v. That is, the connected component
        // of v becomes determined.
        if (!samecomp_found) {
          // Check whether v is isolated.
          // If v is isolated (deg of v is 0), nothing occurs.
          if (d > 0) {
            // Check whether there is a
            // connected component other than that of v,
            // that is, the generated subgraph is not connected.
            // If so, we return the 0-terminal.
            if (nonisolated_found) {
              return 0; // return the 0-terminal.
            } else {
              // count the vertices not leaving the frontier yet
              int not_leaving_frontier_count
                = static_cast<int>(leaving_vs.size()) - i - 1;
              for (int k = edge_index + 1; k < m_; ++k) {
                not_leaving_frontier_count
                  += static_cast<int>(fm_.getLeavingVs(k).size());
              }
              // The degree of the vertices not leaving the frontier yet is 0.
              addFixedDeg(data, 0, not_leaving_frontier_count);
              if (checkFixedDeg(data)) {
                return -1;
              } else {
                return 0;
              }
            }
          }
        }
        // Since comp of v are never used until the end,
        // we erase the value.
        clearComp(data, v);
      }
      // Since deg of v are never used until the end,
      // we erase the value.
      setDeg(data, v, -1);
    }
    if (level == 1) {
      if (is_connected_) {
        // If we come here, the edge set is empty (taking no edge).
        return 0;
      } else {
        if (checkFixedDeg(data)) {
          return -1;
        } else {
          return 0;
        }
      }
    }
    assert(level - 1 > 0);
    return level - 1;
  }
};

#endif // DEGREE_DISTRIBUTION_SPEC_H
