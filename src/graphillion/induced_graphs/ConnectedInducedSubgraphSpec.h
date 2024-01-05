/*
 * ConnectedInducedSubgraphSpec.h
 */

#ifndef GRAPHILLION_CONNECTED_INDUCED_SUBGRAPH_SPEC_H_
#define GRAPHILLION_CONNECTED_INDUCED_SUBGRAPH_SPEC_H_

#include "subsetting/DdSpec.hpp"
#include "subsetting/util/Graph.hpp"

class ConnectedInducedSubgraphSpecMate {
 public:
  using Offset = int32_t;
  using uOffset = uint32_t;

 private:
  Offset hoc;  // offset to head or FPS.
  Offset nxt;  // offset to next connected vertex.

 public:
  ConnectedInducedSubgraphSpecMate(Offset hoc = 0) : hoc(hoc), nxt(0) {}

  bool operator==(ConnectedInducedSubgraphSpecMate const& o) const {
    return this == &o;
  }

  bool operator!=(ConnectedInducedSubgraphSpecMate const& o) const {
    return this != &o;
  }

  void clear() {
    hoc = 0;
    nxt = 0;
  }

  bool isHead() const { return hoc >= 0; }

  bool isTail() const { return nxt == 0; }

  bool isIsolated() const { return isHead() && isTail(); }

  ConnectedInducedSubgraphSpecMate& head() {
    return isHead() ? *this : *(this + hoc);
  }

  ConnectedInducedSubgraphSpecMate const& head() const {
    return isHead() ? *this : *(this + hoc);
  }

  ConnectedInducedSubgraphSpecMate& next() { return *(this + nxt); }

  ConnectedInducedSubgraphSpecMate const& next() const { return *(this + nxt); }

  void addMark() { hoc |= 1; }

  bool isMarked() const { return (hoc & 1) == 1; }

  void addTouched() { hoc |= 2; }

  bool isTouched() const { return (hoc & 2) == 2; }

  bool isComponent() const { return !isIsolated() || isTouched(); }

  void mergeLists(ConnectedInducedSubgraphSpecMate& o1,
                  ConnectedInducedSubgraphSpecMate& o2,
                  ConnectedInducedSubgraphSpecMate* mate) {
    auto p1 = &o1.head();
    auto p2 = &o2.head();
    if (p1 == p2) return;
    if (p1 > p2) std::swap(p1, p2);

    p1->addTouched();

    for (auto q = p2;; q += q->nxt) {
      q->hoc = p1 - q;
      if (q->nxt == 0) break;
    }

    auto p = p1, q = p2;

    while (true) {
      assert(p != q);
      auto pp = p + p->nxt;
      assert(p <= pp && pp != q);

      while (p < pp && pp < q) {
        p = pp;
        pp += pp->nxt;
        assert(p <= pp && pp != q);
      }

      assert(p == pp || q < pp);
      p->nxt = q - p;
      if (p == pp) break;
      p = q, q = pp;
    }
  }

  void replaceHeadWith(ConnectedInducedSubgraphSpecMate& newHead,
                       ConnectedInducedSubgraphSpecMate* mate) const {
    auto p = &head();
    auto q = &newHead;

    q->hoc = p->hoc;

    while (q->nxt > 0) {
      q += q->nxt;
      q->hoc = &newHead - q;
    }
  }

  void removeFromList(ConnectedInducedSubgraphSpecMate const& o) {
    if (o.nxt == 0) {
      for (auto p = this; p <= &o; ++p) {
        if (p + p->nxt == &o) p->nxt = 0;
      }
    } else {
      for (auto p = this; p <= &o; ++p) {
        if (p + p->nxt == &o) p->nxt += o.nxt;
      }
    }
  }
};

class ConnectedInducedSubgraphSpec
    : public tdzdd::PodArrayDdSpec<ConnectedInducedSubgraphSpec,
                                   ConnectedInducedSubgraphSpecMate, 2> {
  using Mate = ConnectedInducedSubgraphSpecMate;

  tdzdd::Graph const& graph;
  int const m;
  int const n;
  int const mateSize;
  std::vector<Mate> initialMate;
  bool const lookahead;

  struct Vec2 {
    std::vector<int> list1;
    std::vector<int> list2;
  };
  std::vector<Vec2> neighborList;

  int takable(Mate const* mate, tdzdd::Graph::EdgeInfo const& e) const {
    Mate const &w1 = mate[e.v1 - e.v0], w2 = mate[e.v2 - e.v0];

    if (!w1.isComponent() && w1.isMarked()) {
      return 0;
    }
    if (!w2.isComponent() && w2.isMarked()) {
      return 0;
    }

    if (e.v1final && e.v2final) {
      if (w1.isIsolated() &&
          w2.isIsolated()) {  // new component leaves immediately
        for (int i = 0; i < mateSize; ++i) {
          if (i != e.v1 - e.v0 && i != e.v2 - e.v0 && mate[i].isComponent()) {
            return 0;
          }
        }
        return -1;
      } else if (w1.isHead() && w2 == w1.next() &&
                 w2.isTail()) {  // existing component leaves
        assert(w1.isTouched());
        for (int i = 0; i < mateSize; ++i) {
          if (i != e.v1 - e.v0 && i != e.v2 - e.v0 && mate[i].isComponent()) {
            return 0;
          }
        }
        return -1;
      }
    }

    return 1;
  }

  int doTake(Mate* mate, tdzdd::Graph::EdgeInfo const& e, int index) const {
    int c = takable(mate, e);
    if (c != 1) {
      return c;
    }

    Mate& w1 = mate[e.v1 - e.v0];
    Mate& w2 = mate[e.v2 - e.v0];

    if (!w1.isComponent()) {
      for (size_t i = 0; i < neighborList[index].list1.size(); ++i) {
        int v = neighborList[index].list1[i];
        mate[v - e.v0].addMark();
      }
    }
    if (!w2.isComponent()) {
      for (size_t i = 0; i < neighborList[index].list2.size(); ++i) {
        int v = neighborList[index].list2[i];
        mate[v - e.v0].addMark();
      }
    }

    mate[0].mergeLists(mate[e.v1 - e.v0], mate[e.v2 - e.v0], mate);
    return 1;
  }

  int doNotTake(Mate* mate, tdzdd::Graph::EdgeInfo const& e) const {
    Mate& w1 = mate[e.v1 - e.v0];
    Mate& w2 = mate[e.v2 - e.v0];

    if (w1.isComponent() && w2.isComponent()) {
      return 0;
    } else if (w1.isComponent()) {
      w2.addMark();
    } else if (w2.isComponent()) {
      w1.addMark();
    }

    if (e.v1final && w1.isIsolated()) {
      if (w1.isTouched()) {
        for (int i = 0; i < mateSize; ++i) {
          if (i != e.v1 - e.v0 && mate[i].isComponent()) {
            return 0;
          }
        }
        return -1;
      }
    }

    if (e.v2final && w2.isIsolated()) {
      if (w2.isTouched()) {
        for (int i = 0; i < mateSize; ++i) {
          if (i != e.v2 - e.v0 && mate[i].isComponent()) {
            return 0;
          }
        }
        return -1;
      }
    }

    if (e.v1final && e.v2final && w1.isHead() && w2 == w1.next() &&
        w2.isTail()) {  // existing component leaves) {
      assert(w1.isTouched());
      for (int i = 0; i < mateSize; ++i) {
        if (i != e.v1 - e.v0 && i != e.v2 - e.v0 && mate[i].isComponent()) {
          return 0;
        }
      }
      return -1;
    }

    if (e.finalEdge) return 0;
    return 1;
  }

  void update(Mate* mate, tdzdd::Graph::EdgeInfo const& e,
              tdzdd::Graph::EdgeInfo const& ee) const {
    int const d = ee.v0 - e.v0;
    assert(d >= 0);
    auto p1 = &mate[e.v1 - e.v0], p2 = &mate[e.v2 - e.v0], pd = p1 + d;

    for (auto q = p1; q < pd; ++q) {
      Mate* qq = &q->next();
      if (qq >= pd) {
        q->replaceHeadWith(*qq, mate);
      }
    }

    if (e.v2final) {
      mate[0].removeFromList(*p2);
      p2->clear();
    }

    if (e.v1final) {
      mate[0].removeFromList(*p1);
      p1->clear();
    }

    if (d > 0) {
      std::memmove(p1, pd, (mateSize - d) * sizeof(*mate));
      for (int i = mateSize - d; i < mateSize; ++i) {
        p1[i] = initialMate[ee.v0 + i];
      }
    }
  }

 public:
  ConnectedInducedSubgraphSpec(tdzdd::Graph const& graph, bool lookahead = true)
      : graph(graph),
        m(graph.vertexSize()),
        n(graph.edgeSize()),
        mateSize(graph.maxFrontierSize()),
        initialMate(1 + m + mateSize),
        lookahead(lookahead) {
    this->setArraySize(mateSize);

    for (int v = 1; v <= m; ++v) {
      initialMate[v] = Mate(0);
    }

    neighborList.resize(n);

    for (int i = 0; i < n; ++i) {
      tdzdd::Graph::EdgeInfo const& e = graph.edgeInfo(i);
      for (int j = 0; j < i; ++j) {
        tdzdd::Graph::EdgeInfo const& ee = graph.edgeInfo(j);
        if (ee.v1 == e.v1 && e.v0 <= ee.v2) {
          neighborList[i].list1.push_back(ee.v2);
        }
        if (ee.v2 == e.v1 && e.v0 <= ee.v1) {
          neighborList[i].list1.push_back(ee.v1);
        }
        if (ee.v1 == e.v2 && e.v0 <= ee.v2) {
          neighborList[i].list2.push_back(ee.v2);
        }
        if (ee.v2 == e.v2 && e.v0 <= ee.v1) {
          neighborList[i].list2.push_back(ee.v1);
        }
      }
    }
  }

  int getRoot(Mate* mate) const {
    int const v0 = graph.edgeInfo(0).v0;

    for (int i = 0; i < mateSize; ++i) {
      mate[i] = initialMate[v0 + i];
    }

    return n;
  }

  int getChild(Mate* mate, int level, int take) const {
    assert(1 <= level && level <= n);
    int i = n - level;
    tdzdd::Graph::EdgeInfo const* e = &graph.edgeInfo(i);

    if (take) {
      int c = doTake(mate, *e, i);
      if (c != 1) {
        return c;
      }
    } else {
      int c = doNotTake(mate, *e);
      if (c != 1) {
        return c;
      }
    }

    if (++i == n) return -1;

    tdzdd::Graph::EdgeInfo const* ee = &graph.edgeInfo(i);
    update(mate, *e, *ee);

    while (lookahead) {
      e = ee;

      if (takable(mate, *e) != 0) break;
      int c2 = doNotTake(mate, *e);
      if (c2 != 1) {
        return c2;
      }

      if (++i == n) return -1;

      ee = &graph.edgeInfo(i);
      update(mate, *e, *ee);
    }

    assert(i < n);
    return n - i;
  }
};

#endif  // GRAPHILLION_CONNECTED_INDUCED_SUBGRAPH_SPEC_H_
