#pragma once

#include "subsetting/DdSpec.hpp"
#include "subsetting/util/Graph.hpp"

struct ComponentRatioSpecCount {
  int32_t lower;
  int32_t upper;

  ComponentRatioSpecCount() : lower(INT_MAX), upper(0) {}

  size_t hash() const { return lower + upper * 31415926535; }

  bool operator==(ComponentRatioSpecCount const& o) const {
    return lower == o.lower && upper == o.upper;
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  ComponentRatioSpecCount const& o) {
    return os << o.lower << "," << o.upper;
  }
};

class ComponentRatioSpecMate {
 public:
  typedef int32_t Offset;
  typedef uint32_t uOffset;

 private:
  Offset hoc;  ///< offset to head or vertex-weight.
  Offset nxt;  ///< offset to next connected vertex.

 public:
  ComponentRatioSpecMate(Offset hoc = 0) : hoc(hoc), nxt(0) {}

  bool operator==(ComponentRatioSpecMate const& o) const { return this == &o; }

  bool operator!=(ComponentRatioSpecMate const& o) const { return this != &o; }

  void clear() {
    hoc = 0;
    nxt = 0;
  }

  bool isHead() const { return hoc >= 0; }

  bool isTail() const { return nxt == 0; }

  bool isIsolated() const { return isHead() && isTail(); }

  ComponentRatioSpecMate& head() { return isHead() ? *this : *(this + hoc); }

  ComponentRatioSpecMate const& head() const {
    return isHead() ? *this : *(this + hoc);
  }

  ComponentRatioSpecMate& next() { return *(this + nxt); }

  ComponentRatioSpecMate const& next() const { return *(this + nxt); }

  int getWeight() const {
    assert(head().hoc >= 0);
    return head().hoc;
  }

  void print() const { std::cerr << "(" << hoc << "," << nxt << ")"; }

  void print(std::ostream& ost) const {
    ost << "(" << hoc << "," << nxt << ")";
  }

  void print(FILE* fp) const { fprintf(fp, "(%d,%d)", hoc, nxt); }

  void mergeLists(ComponentRatioSpecMate& o1, ComponentRatioSpecMate& o2,
                  ComponentRatioSpecMate* mate) {
    ComponentRatioSpecMate* p1 = &o1.head();
    ComponentRatioSpecMate* p2 = &o2.head();
    if (p1 == p2) return;
    if (p1 > p2) std::swap(p1, p2);

    p1->hoc += p2->hoc;

    for (ComponentRatioSpecMate* q = p2;; q += q->nxt) {
      q->hoc = p1 - q;
      if (q->nxt == 0) break;
    }

    ComponentRatioSpecMate* p = p1;
    ComponentRatioSpecMate* q = p2;

    while (true) {
      assert(p != q);
      ComponentRatioSpecMate* pp = p + p->nxt;
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

  void replaceHeadWith(ComponentRatioSpecMate& newHead,
                       ComponentRatioSpecMate* mate) const {
    ComponentRatioSpecMate const* p = &head();
    ComponentRatioSpecMate* q = &newHead;

    Offset v = p->hoc;
    assert(v >= 0);

    q->hoc = v;

    while (q->nxt > 0) {
      q += q->nxt;
      q->hoc = &newHead - q;
    }
  }

  void removeFromList(ComponentRatioSpecMate const& o) {
    if (o.nxt == 0) {
      for (ComponentRatioSpecMate* p = this; p <= &o; ++p) {
        if (p + p->nxt == &o) p->nxt = 0;
      }
    } else {
      for (ComponentRatioSpecMate* p = this; p <= &o; ++p) {
        if (p + p->nxt == &o) p->nxt += o.nxt;
      }
    }
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  ComponentRatioSpecMate const& o) {
    return os << "<" << o.hoc << "," << o.nxt << ">";
  }
};

class ComponentRatioSpec
    : public tdzdd::HybridDdSpec<ComponentRatioSpec, ComponentRatioSpecCount,
                                 ComponentRatioSpecMate, 2> {
  typedef ComponentRatioSpecMate Mate;
  typedef ComponentRatioSpecCount Count;

  tdzdd::Graph const& graph;
  int const m;
  int const n;
  int const lower;
  int const upper;
  double const ratio;
  int const mateSize;
  std::vector<Mate> initialMate;
  bool const lookahead;

  bool updateRatio(Count& count, int weight) const {
    if (weight < lower) {
      return false;
    } else {
      if (weight > count.upper) {
        count.upper = weight;
      }
      if (weight < count.lower) {
        count.lower = weight;
      }
    }
    if (static_cast<double>(count.upper) >
        static_cast<double>(count.lower) * ratio) {
      return false;
    }
    return true;
  }

  int takable(Count& count, Mate const* mate,
              tdzdd::Graph::EdgeInfo const& e) const {
    Mate const& w1 = mate[e.v1 - e.v0];
    Mate const& w2 = mate[e.v2 - e.v0];

    if (w1.head() != w2.head() &&
        w1.getWeight() + w2.getWeight() > upper) {  // distinct components
      return false;
    }

    if (e.v1final && e.v2final) {
      if (w1.isIsolated() &&
          w2.isIsolated()) {  // new component leaves immediately
        if (!updateRatio(count, w1.getWeight() + w2.getWeight())) {
          return false;
        }
      } else if (w1.isHead() && w2 == w1.next() &&
                 w2.isTail()) {  // existing component leaves
        if (!updateRatio(count, w1.getWeight())) {
          return false;
        }
      }
    }
    return true;
  }

  bool doTake(Count& count, Mate* mate, tdzdd::Graph::EdgeInfo const& e) const {
    if (!takable(count, mate, e)) return false;

    mate[0].mergeLists(mate[e.v1 - e.v0], mate[e.v2 - e.v0], mate);
    assert(mate[0].getWeight() <= upper);
    return true;
  }

  bool doNotTake(Count& count, Mate* mate,
                 tdzdd::Graph::EdgeInfo const& e) const {
    Mate& w1 = mate[e.v1 - e.v0];
    Mate& w2 = mate[e.v2 - e.v0];

    if (e.v1final && w1.isIsolated()) {
      if (!updateRatio(count, w1.getWeight())) {
        return false;
      }
    }

    if (e.v2final && w2.isIsolated()) {
      if (!updateRatio(count, w2.getWeight())) {
        return false;
      }
    }

    if (e.v1final && e.v2final && w1.isHead() && w2 == w1.next() &&
        w2.isTail()) {  // existing component leaves
      if (!updateRatio(count, w1.getWeight())) {
        return false;
      }
    }

    return true;
  }

  void update(Mate* mate, tdzdd::Graph::EdgeInfo const& e,
              tdzdd::Graph::EdgeInfo const& ee) const {
    int const d = ee.v0 - e.v0;
    assert(d >= 0);
    Mate* p1 = &mate[e.v1 - e.v0];
    Mate* p2 = &mate[e.v2 - e.v0];
    Mate* pd = p1 + d;

    for (Mate* q = p1; q < pd; ++q) {
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
  // weight_list: [weight of vertex 1, weight of vertex 2,..., weight of vertex
  // m]
  ComponentRatioSpec(tdzdd::Graph const& graph,
                     const std::vector<uint32_t>& weight_list, uint32_t lower,
                     uint32_t upper, double ratio, bool /*noLoop*/ = false,
                     bool lookahead = true)
      : graph(graph),
        m(graph.vertexSize()),
        n(graph.edgeSize()),
        lower(lower),
        upper(upper),
        ratio(ratio),
        mateSize(graph.maxFrontierSize()),
        initialMate(1 + m + mateSize),
        lookahead(lookahead) {
    this->setArraySize(mateSize);
    for (int u = 0; u < m; ++u) {
      initialMate[u + 1] = Mate(ComponentRatioSpecMate::Offset(weight_list[u]));
    }
  }

  int getRoot(Count& count, Mate* mate) const {
    for (int v = 1; v <= m; ++v) {
      if (initialMate[v].getWeight() > upper) {
        return 0;
      }
    }

    int const v0 = graph.edgeInfo(0).v0;

    for (int i = 0; i < mateSize; ++i) {
      mate[i] = initialMate[v0 + i];
    }
    assert(count.lower == INT_MAX);
    assert(count.upper == 0);

    return n;
  }

  int getChild(Count& count, Mate* mate, int level, int take) const {
    assert(1 <= level && level <= n);
    int i = n - level;
    tdzdd::Graph::EdgeInfo const* e = &graph.edgeInfo(i);

    Count c = count;

    if (take) {
      if (!doTake(c, mate, *e)) return 0;
    } else {
      if (!doNotTake(c, mate, *e)) return 0;
    }

    if (++i == n) return -1;

    count = c;

    tdzdd::Graph::EdgeInfo const* ee = &graph.edgeInfo(i);
    update(mate, *e, *ee);

    while (lookahead) {
      e = ee;
      Count cc = count;
      if (takable(cc, mate, *e)) break;
      if (!doNotTake(count, mate, *e)) return 0;

      if (++i == n) return -1;

      ee = &graph.edgeInfo(i);
      update(mate, *e, *ee);
    }

    assert(i < n);
    return n - i;
  }
};
