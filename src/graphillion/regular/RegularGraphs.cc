#include "RegularGraphs.h"

#include "graphillion/setset.h"
#include "graphillion/graphset.h"
#include "graphillion/forbidden_induced/FrontierManager.h"
#include "subsetting/DdSpec.hpp"
#include "subsetting/util/IntSubset.hpp"
#include "subsetting/eval/ToZBDD.hpp"
#include "subsetting/spec/SapporoZdd.hpp"

#include <vector>
#include <climits>

typedef unsigned char uchar;
typedef unsigned char RData;

const int RData_MAX = UCHAR_MAX;

class RegularSpec
  : public tdzdd::PodArrayDdSpec<RegularSpec, RData, 2> {
private:
  // input graph
  const tdzdd::Graph& graph_;
  // number of vertices
  const int n_;
  // number of edges
  const int m_;

  const int deg_lower_;
  const int deg_upper_;
  // make subgraphs connected or not
  const bool is_connected_;

  const FrontierManager fm_;

  const int fixedDegStart_;

  // This function gets deg of v.
  int getDeg(RData* data, int v) const {
    return static_cast<int>(data[is_connected_ ?
                  (fm_.vertexToPos(v) * 2) :
                  fm_.vertexToPos(v)]);
  }

  // This function sets deg of v to be d.
  void setDeg(RData* data, int v, int d) const {
    data[is_connected_ ?
      (fm_.vertexToPos(v) * 2) :
      fm_.vertexToPos(v)] = static_cast<uchar>(d);
  }

  // This function gets comp of v.
  int getComp(RData* data, int v, int index) const {
    assert(is_connected_);
    return fm_.posToVertex(index, data[fm_.vertexToPos(v) * 2 + 1]);
  }

  // This function sets comp of v to be c.
  void setComp(RData* data, int v, int c) const {
    assert(is_connected_);
    data[fm_.vertexToPos(v) * 2 + 1] =
      static_cast<uchar>(fm_.vertexToPos(c));
  }

  void clearComp(RData* data, int v) const {
    assert(is_connected_);
    data[fm_.vertexToPos(v) * 2 + 1] = static_cast<uchar>(-1);
  }

  int getFixedDeg(RData* data) const {
    return data[fixedDegStart_];
  }

  void setFixedDeg(RData* data, int value) const {
    data[fixedDegStart_] = value;
  }

  void initializeData(RData* data) const {
    for (int i = 0; i < fixedDegStart_ + 1; ++i) {
      data[i] = 0;
    }
  }

public:
  RegularSpec(const tdzdd::Graph& graph,
              const int degree_lower,
              const int degree_upper,
              const bool is_connected)
    : graph_(graph),
      n_(static_cast<short>(graph_.vertexSize())),
      m_(graph_.edgeSize()),
      deg_lower_(degree_lower),
      deg_upper_(degree_upper),
      is_connected_(is_connected),
      fm_(graph_),
      fixedDegStart_(is_connected ? (fm_.getMaxFrontierSize() * 2) :
                      fm_.getMaxFrontierSize())
  {
    if (graph_.vertexSize() > SHRT_MAX) { // SHRT_MAX == 32767
      std::cerr << "The number of vertices should be at most "
            << SHRT_MAX << std::endl;
      exit(1);
    }

    setArraySize(fixedDegStart_ + 1);
  }

  int getRoot(RData* data) const {
    initializeData(data);
    return m_;
  }

  int getChild(RData* data, int level, int value) const {
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

      int fixed_deg = getFixedDeg(data);

      if (fixed_deg > 0) {
        if (getDeg(data, edge.v1) >= fixed_deg) {
          return 0;
        }
        if (getDeg(data, edge.v2) >= fixed_deg) {
          return 0;
        }
      }

      if (getDeg(data, edge.v1) >= RData_MAX ||
        getDeg(data, edge.v2) >= RData_MAX) {
        std::cerr << "The degree exceeded "
          << RData_MAX << "." << std::endl;
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

      int fixed_deg = getFixedDeg(data);
      if (fixed_deg > 0) {
        if (d != fixed_deg && d > 0) {
          return 0;
        }
      } else if (d > 0) {
        if (d < deg_lower_ || d > deg_upper_) {
          return 0;
        }
        setFixedDeg(data, d);
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
              return -1;
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
        if (getFixedDeg(data) == 0) {
          // If we come here, the edge set is empty (taking no edge).
          return 0;
        } else {
          return -1;
        }
      }
    }
    assert(level - 1 > 0);
    return level - 1;
  }
};

tdzdd::DdStructure<2>
constructRegularGraphs(const tdzdd::Graph &g,
                        const int degree_lower,
                        const int degree_upper,
                        const bool is_connected,
                        const graphillion::zdd_t* search_space,
                        const int offset) {

#ifdef _OPENMP
  bool use_mp = (omp_get_num_procs() >= 2);
#else
  bool use_mp = false;
#endif

  tdzdd::DdStructure<2> dd;
  if (search_space != NULL) {
    SapporoZdd f(*search_space, offset);
    dd = tdzdd::DdStructure<2>(f, use_mp);
  } else {
    dd = tdzdd::DdStructure<2>(g.edgeSize(), use_mp);
  }

  RegularSpec ddspec(g, degree_lower,
                      degree_upper, is_connected);

  dd.zddSubset(ddspec);
  dd.zddReduce();

  return dd;
}

namespace graphillion {

setset
SearchRegularGraphs(const std::vector<edge_t> &edges,
                    const int degree_lower,
                    const int degree_upper,
                    const bool is_connected,
                    const setset* search_space) {
  tdzdd::Graph g;
  for (const auto &e : edges) {
    g.addEdge(e.first, e.second);
  }
  g.update();

  const zdd_t* search_space_z =
  ((search_space == NULL) ? NULL : &search_space->zdd_);

  auto dd = constructRegularGraphs(g, degree_lower,
              degree_upper, is_connected, search_space_z,
              setset::max_elem() - setset::num_elems());
  dd.useMultiProcessors(false);
  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

}  // namespace graphillion
