/**
Copyright (c) 2021 ComputerAlgorithmsGroupAtKyotoU

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef FRONTIER_FOREST_HPP
#define FRONTIER_FOREST_HPP

#include <climits>
#include <set>
#include <vector>

#include "FrontierData.hpp"
#include "FrontierManager.hpp"
#include "subsetting/DdSpec.hpp"
#include "Digraph.hpp"

using namespace tdzdd;

typedef unsigned short ushort;

class FrontierRootedForestSpec
    : public tdzdd::PodArrayDdSpec<FrontierRootedForestSpec,
                                   DirectedFrontierData, 2> {
 private:
  // input graph
  const graphillion::Digraph& graph_;
  // root verteces
  const std::set<graphillion::Digraph::VertexNumber> roots;
  // spanning forest or not
  bool is_spanning;
  // number of vertices
  const short n_;
  // number of edges
  const int m_;

  const FrontierManager fm_;

  // This function gets deg of v.
  short getIndeg(DirectedFrontierData* data, short v) const {
    return data[fm_.vertexToPos(v)].indeg;
  }

  short getOutdeg(DirectedFrontierData* data, short v) const {
    return data[fm_.vertexToPos(v)].outdeg;
  }

  // This function sets deg of v to be d.
  void setIndeg(DirectedFrontierData* data, short v, short d) const {
    data[fm_.vertexToPos(v)].indeg = d;
  }

  void setOutdeg(DirectedFrontierData* data, short v, short d) const {
    data[fm_.vertexToPos(v)].outdeg = d;
  }

  // This function gets comp of v.
  ushort getComp(DirectedFrontierData* data, short v) const {
    return data[fm_.vertexToPos(v)].comp;
  }

  // This function sets comp of v to be c.
  void setComp(DirectedFrontierData* data, short v, ushort c) const {
    data[fm_.vertexToPos(v)].comp = c;
  }

  void initializeData(DirectedFrontierData* data) const {
    for (int i = 0; i < fm_.getMaxFrontierSize(); ++i) {
      data[i].indeg = 0;
      data[i].outdeg = 0;
      data[i].comp = 0;
    }
  }

 public:
  FrontierRootedForestSpec(const graphillion::Digraph& graph,
                           const std::set<graphillion::Digraph::VertexNumber>& _roots,
                           bool _is_spanning)
      : graph_(graph),
        roots(_roots),
        is_spanning(_is_spanning),
        n_(static_cast<short>(graph_.vertexSize())),
        m_(graph_.edgeSize()),
        fm_(graph_) {
    if (graph_.vertexSize() >= (1 << 16)) {
      std::cerr << "The number of vertices must be smaller than 2^15."
                << std::endl;
      exit(1);
    }
    setArraySize(fm_.getMaxFrontierSize());
  }

  int getRoot(DirectedFrontierData* data) const {
    initializeData(data);
    return m_;
  }

  int getChild(DirectedFrontierData* data, int level, int value) const {
    assert(1 <= level && level <= m_);

    // edge index (starting from 0)
    const int edge_index = m_ - level;
    // edge that we are processing.
    // The endpoints of "edge" are edge.v1 and edge.v2.
    const graphillion::Digraph::EdgeInfo& edge = graph_.edgeInfo(edge_index);

    // initialize deg and comp of the vertices newly entering the frontier
    const std::vector<int>& entering_vs = fm_.getEnteringVs(edge_index);
    for (size_t i = 0; i < entering_vs.size(); ++i) {
      int v = entering_vs[i];
      // initially the value of deg is 0
      setIndeg(data, v, 0);
      setOutdeg(data, v, 0);
      // initially the value of comp is the vertex number itself
      setComp(data, v, static_cast<ushort>(v));
    }

    // vertices on the frontier
    const std::vector<int>& frontier_vs = fm_.getFrontierVs(edge_index);

    if (value == 1) {  // if we take the edge (go to 1-arc)
      // increment deg of v1 and v2 (recall that edge = {v1, v2})
      auto outdeg1 = getOutdeg(data, edge.v1);
      auto indeg2 = getIndeg(data, edge.v2);

      setIndeg(data, edge.v2, indeg2 + 1);
      setOutdeg(data, edge.v1, outdeg1 + 1);

      ushort c1 = getComp(data, edge.v1);
      ushort c2 = getComp(data, edge.v2);

      if (c1 == c2) {  // Any cycle must not occur.
        return 0;
      }

      if (c1 != c2) {  // connected components c1 and c2 become connected
        ushort cmin = std::min(c1, c2);
        ushort cmax = std::max(c1, c2);

        // replace component number cmin with cmax
        for (size_t i = 0; i < frontier_vs.size(); ++i) {
          int v = frontier_vs[i];
          if (getComp(data, v) == cmin) {
            setComp(data, v, cmax);
          }
        }
      }
    }

    // vertices that are leaving the frontier
    const std::vector<int>& leaving_vs = fm_.getLeavingVs(edge_index);
    for (size_t i = 0; i < leaving_vs.size(); ++i) {
      int v = leaving_vs[i];

      // vertex in a spanning forest must not be isolated
      if (is_spanning) {
        if (getIndeg(data, v) == 0 &&
            getOutdeg(data, v) == 0) {  // the degree of v must be at least 1
          return 0;
        }
      }

      if (roots.size()) {
        // roots are specified.
        if (roots.count(v)) {
          // the in-degree of a root vertex must be 0.
          if (getIndeg(data, v) > 0) {
            return 0;
          }
          // the out-degree of a root vertex must be positive.
          if (getOutdeg(data, v) == 0) {
            return 0;
          }
        } else {
          // the in-degree of a non-root vertex must be 1 or the vertex is
          // isolated.
          bool ok = (getIndeg(data, v) == 1) ||
                    (getIndeg(data, v) == 0 && getOutdeg(data, v) == 0);
          if (!ok) {
            return 0;
          }
        }
      } else {
        // root are not specified.
        // the in-degree of a vertex must be 0 or 1.
        if (getIndeg(data, v) > 1) {
          return 0;
        }
      }

      // Since deg and comp of v are never used until the end,
      // we erase the values.
      setIndeg(data, v, 0);
      setOutdeg(data, v, 0);
      setComp(data, v, 0);
    }
    if (level == 1) {
      return -1;
    }
    assert(level - 1 > 0);
    return level - 1;
  }
};

#endif  // FRONTIER_FOREST_HPP
