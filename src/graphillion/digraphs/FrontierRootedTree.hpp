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

#ifndef FRONTIER_TREE_HPP
#define FRONTIER_TREE_HPP

#include <climits>
#include <vector>

#include "FrontierData.hpp"
#include "FrontierManager.hpp"
#include "subsetting/DdSpec.hpp"
#include "Digraph.hpp"

using namespace tdzdd;

typedef unsigned short ushort;

typedef unsigned short FrontierTreeData;

class FrontierRootedTreeSpec
    : public tdzdd::PodArrayDdSpec<FrontierRootedTreeSpec, DirectedFrontierData,
                                   2> {
 private:
  // input graph
  const graphillion::Digraph& graph_;
  // number of vertices
  const short n_;
  // number of edges
  const int m_;

  // root node
  const ushort root_;

  const bool isSpanning_;

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

  void resetDeg(DirectedFrontierData* data, short v) const {
    setIndeg(data, v, 0);
    setOutdeg(data, v, 0);
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
  FrontierRootedTreeSpec(const graphillion::Digraph& graph, ushort root,
                         bool isSpanning)
      : graph_(graph),
        n_(graph_.vertexSize()),
        m_(graph_.edgeSize()),
        root_(root),
        isSpanning_(isSpanning),
        fm_(graph_) {
    if (graph_.vertexSize() >= (1 << 15)) {
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
      // initially the value of comp is the vertex number itself
      resetDeg(data, v);
      setComp(data, v, static_cast<ushort>(v));
    }

    // vertices on the frontier
    const std::vector<int>& frontier_vs = fm_.getFrontierVs(edge_index);

    if (value == 1) {  // if we take the edge (go to 1-arc)
      ushort c1 = getComp(data, edge.v1);
      ushort c2 = getComp(data, edge.v2);

      if (c1 == c2) {  // Any cycle must not occur.
        return 0;
      }

      // increment deg of v1 and v2 (recall that edge = {v1, v2})
      auto outdeg1 = getOutdeg(data, edge.v1);
      auto indeg2 = getIndeg(data, edge.v2);
      setIndeg(data, edge.v2, indeg2 + 1);
      setOutdeg(data, edge.v1, outdeg1 + 1);

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

      if (isSpanning_) {
        if (getIndeg(data, v) + getOutdeg(data, v) ==
            0) {  // the degree of v must be at least 1
          return 0;
        }
      }

      if (v == root_) {
        // the in-degree of root node must be 0.
        if (getIndeg(data, v) != 0) {
          return 0;
        }
        // the out-degdee of root node must not be 0.
        // if (getOutdeg(data, v) == 0) {
        //  return 0;
        //}
      } else {
        // if v has no incoming edge, v cant have outgoing edges.
        if (getIndeg(data, v) == 0 && getOutdeg(data, v) > 0) {
          return 0;
        }
        // in-degree of non-root node must be 0 or 1.
        if (getIndeg(data, v) > 1) {
          return 0;
        }
      }

      // The degree of v must be 0 or 2.
      // if (getDeg(data, v) != 0 && getDeg(data, v) != 2) {
      //    return 0;
      //}
      bool comp_found = false;
      bool deg_found = false;
      bool frontier_exists = false;
      // Search a vertex that has the component number same as that of v.
      // Also check whether a vertex whose degree is at least 1 exists
      // on the frontier.
      for (size_t j = 0; j < frontier_vs.size(); ++j) {
        int w = frontier_vs[j];
        if (w == v) {  // skip if w is the leaving vertex
          continue;
        }
        // skip if w is one of the vertices that
        // has already leaved the frontier
        bool found_leaved = false;
        for (size_t k = 0; k < i; ++k) {
          if (w == leaving_vs[k]) {
            found_leaved = true;
            break;
          }
        }
        if (found_leaved) {
          continue;
        }
        frontier_exists = true;
        // w has the component number same as that of v
        if (getComp(data, w) == getComp(data, v)) {
          comp_found = true;
        }
        // The degree of w is at least 1.
        if (getIndeg(data, w) + getOutdeg(data, w)) {
          deg_found = true;
        }
        if (deg_found && comp_found) {
          break;
        }
      }
      // There is no vertex that has the component number
      // same as that of v. That is, the connected component
      // of v becomes determined.
      if (!comp_found) {
        // Here, deg of v is 0 or 2. If deg of v is 0,
        // this means that v is isolated.
        // If deg of v is 2, and there is a vertex whose
        // deg is at least 1, this means that there is a
        // connected component other than that of v.
        // That is, the generated subgraph is not connected.
        // Then, we return the 0-terminal.
        // assert(getDeg(data, v) == 0 || getDeg(data, v) == 2);
        if (getIndeg(data, v) + getOutdeg(data, v) > 0 && deg_found) {
          return 0;  // return the 0-terminal.
        } else if (getIndeg(data, v) +
                   getOutdeg(data, v)) {  // If deg of v is 2,
          // and there is no vertex whose deg is at least 1
          // a single cycle is completed.
          // Then, we return the 1-terminal

          if (isSpanning_) {
            if (frontier_exists) {
              return 0;
            } else {
              return -1;  // return the 1-terminal
            }
          } else {
            return -1;  // return the 1-terminal
          }
        }
      }

      // Since deg and comp of v are never used until the end,
      // we erase the values.
      resetDeg(data, v);
      setComp(data, v, 0);
    }
    if (level == 1) {
      // If we come here, the edge set is empty (taking no edge).
      return -1;  // return the 1-terminal
    }

    assert(level - 1 > 0);
    return level - 1;
  }
};

#endif  // FRONTIER_TREE_HPP
