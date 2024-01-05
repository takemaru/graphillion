#ifndef GRAPHILLION_WEIGHTED_INDUCED_SPEC_H_
#define GRAPHILLION_WEIGHTED_INDUCED_SPEC_H_

#include "subsetting/DdSpec.hpp"
#include "subsetting/util/Graph.hpp"

class ComponentWeightInducedSpecMate {
 public:
  using offset = int32_t;

  ComponentWeightInducedSpecMate(offset _offset_or_weight = 0)
      : offset_or_weight(_offset_or_weight),
        next_conn(1 << single_vertex_bit) {}

  bool operator==(ComponentWeightInducedSpecMate const& o) const {
    return this == &o;
  }

  bool operator!=(ComponentWeightInducedSpecMate const& o) const {
    return this != &o;
  }

  void clear() {
    offset_or_weight = 0;
    next_conn = (1 << single_vertex_bit);
  }

  bool is_single_vertex() const {
    return ((next_conn >> single_vertex_bit) & 1);
  }

  void set_connected() { next_conn &= next_mask; }

  bool is_head() const { return offset_or_weight >= 0; }

  bool is_tail() const { return (next_conn & next_mask) == 0; }

  bool is_isolated() const { return is_head() && is_tail(); }

  ComponentWeightInducedSpecMate& head() {
    return is_head() ? *this : *(this + offset_or_weight);
  }

  ComponentWeightInducedSpecMate const& head() const {
    return is_head() ? *this : *(this + offset_or_weight);
  }

  ComponentWeightInducedSpecMate& next() {
    return *(this + (next_conn & next_mask));
  }

  ComponentWeightInducedSpecMate const& next() const {
    return *(this + (next_conn & next_mask));
  }

  int get_weight() const {
    assert(head().offset_or_weight >= 0);
    return head().offset_or_weight;
  }

  void merge_lists(ComponentWeightInducedSpecMate& o1,
                   ComponentWeightInducedSpecMate& o2) {
    auto p1 = &o1.head();
    auto p2 = &o2.head();
    if (p1 == p2) return;
    if (p1 > p2) std::swap(p1, p2);

    p1->offset_or_weight += p2->offset_or_weight;

    for (auto q = p2;; q += (q->next_conn & next_mask)) {
      q->offset_or_weight = p1 - q;
      if ((q->next_conn & next_mask) == 0) break;
    }

    auto p = p1, q = p2;

    while (true) {
      assert(p != q);
      auto pp = p + (p->next_conn & next_mask);
      assert(p <= pp && pp != q);

      while (p < pp && pp < q) {
        p = pp;
        pp += (pp->next_conn & next_mask);
        assert(p <= pp && pp != q);
      }

      assert(p == pp || q < pp);
      p->next_conn = q - p;
      if (p == pp) break;
      p = q, q = pp;
    }
  }

  void replace_head_with(ComponentWeightInducedSpecMate& newHead) const {
    auto p = &head();
    auto q = &newHead;

    offset v = p->offset_or_weight;
    assert(v >= 0);

    q->offset_or_weight = v;

    while ((q->next_conn & next_mask) > 0) {
      q += (q->next_conn & next_mask);
      q->offset_or_weight = &newHead - q;
    }
  }

  void remove_from_list(ComponentWeightInducedSpecMate const& o) {
    if ((o.next_conn & next_mask) == 0) {
      for (auto p = this; p <= &o; ++p) {
        if (p + (p->next_conn & next_mask) == &o) {
          p->next_conn &= (1 << single_vertex_bit);
        }
      }
    } else {
      for (auto p = this; p <= &o; ++p) {
        if (p + (p->next_conn & next_mask) == &o) {
          p->next_conn += (o.next_conn & next_mask);
        }
      }
    }
  }

 private:
  static const int single_vertex_bit = 30;
  static const int next_mask = (1 << single_vertex_bit) - 1;

  offset offset_or_weight;  ///< offset to head or vertex-weight.
  // if (next_conn >> single_vertex_bit) & 1 then isolated
  offset next_conn;  ///< offset to next connected vertex.
};

class ComponentWeightInducedSpec
    : public tdzdd::PodArrayDdSpec<ComponentWeightInducedSpec,
                                   ComponentWeightInducedSpecMate, 2> {
  tdzdd::Graph const& graph;
  int const m;
  int const n;
  int const lower;
  int const upper;
  int const mateSize;
  std::vector<ComponentWeightInducedSpecMate> initialMate;

  int takable(ComponentWeightInducedSpecMate const* mate,
              tdzdd::Graph::EdgeInfo const& e) const {
    auto& w1 = mate[e.v1 - e.v0];
    auto& w2 = mate[e.v2 - e.v0];

    if (w1.head() != w2.head() && w1.get_weight() + w2.get_weight() > upper) {
      return false;
    }

    if (e.v1final && e.v2final) {
      if (w1.is_isolated() &&
          w2.is_isolated()) {  // new component leaves immediately
        if (w1.get_weight() + w2.get_weight() < lower) {
          return false;
        }
      } else if (w1.is_head() && w2 == w1.next() &&
                 w2.is_tail()) {  // existing component leaves
        if (w1.get_weight() < lower) {
          return false;
        }
      }
    }
    return true;
  }

  bool doTake(ComponentWeightInducedSpecMate* mate,
              tdzdd::Graph::EdgeInfo const& e) const {
    if (!takable(mate, e)) return false;

    mate[e.v1 - e.v0].set_connected();
    mate[e.v2 - e.v0].set_connected();
    mate[0].merge_lists(mate[e.v1 - e.v0], mate[e.v2 - e.v0]);
    assert(mate[0].get_weight() <= upper);
    return true;
  }

  bool doNotTake(ComponentWeightInducedSpecMate* mate,
                 tdzdd::Graph::EdgeInfo const& e) const {
    auto& w1 = mate[e.v1 - e.v0];
    auto& w2 = mate[e.v2 - e.v0];

    // w1 leaves from the frontier
    if (e.v1final && w1.is_isolated()) {
      if (!w1.is_single_vertex() && w1.get_weight() < lower) {
        return false;
      }
    }

    // w2 leaves from the frontier
    if (e.v2final && w2.is_isolated()) {
      if (!w2.is_single_vertex() && w2.get_weight() < lower) {
        return false;
      }
    }

    if (e.v1final && e.v2final && w1.is_head() && w2 == w1.next() &&
        w2.is_tail()) {  // existing component leaves
      if (w1.get_weight() < lower) {
        return false;
      }
    }
    return true;
  }

  void update(ComponentWeightInducedSpecMate* mate,
              tdzdd::Graph::EdgeInfo const& e,
              tdzdd::Graph::EdgeInfo const& ee) const {
    int const d = ee.v0 - e.v0;
    assert(d >= 0);
    auto p1 = &mate[e.v1 - e.v0];
    auto p2 = &mate[e.v2 - e.v0];
    auto pd = p1 + d;

    for (auto q = p1; q < pd; ++q) {
      auto qq = &q->next();
      if (qq >= pd) {
        q->replace_head_with(*qq);
      }
    }

    if (e.v2final) {
      mate[0].remove_from_list(*p2);
      p2->clear();
    }

    if (e.v1final) {
      mate[0].remove_from_list(*p1);
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
  ComponentWeightInducedSpec(tdzdd::Graph const& graph,
                             const std::vector<uint32_t>& weight_list,
                             uint32_t lower, uint32_t upper)
      : graph(graph),
        m(graph.vertexSize()),
        n(graph.edgeSize()),
        lower(lower),
        upper(upper),
        mateSize(graph.maxFrontierSize()),
        initialMate(1 + m + mateSize) {
    this->setArraySize(mateSize);

    for (int u = 0; u < m; ++u) {
      initialMate[u + 1] = ComponentWeightInducedSpecMate(
          ComponentWeightInducedSpecMate::offset(weight_list[u]));
    }
  }

  int getRoot(ComponentWeightInducedSpecMate* mate) const {
    int const v0 = graph.edgeInfo(0).v0;

    for (int i = 0; i < mateSize; ++i) {
      mate[i] = initialMate[v0 + i];
    }

    return n;
  }

  int getChild(ComponentWeightInducedSpecMate* mate, int level,
               int take) const {
    assert(1 <= level && level <= n);
    int i = n - level;
    tdzdd::Graph::EdgeInfo const* e = &graph.edgeInfo(i);

    if (take) {
      if (!doTake(mate, *e)) return 0;
    } else {
      if (!doNotTake(mate, *e)) return 0;
    }

    if (++i == n) return -1;

    tdzdd::Graph::EdgeInfo const* ee = &graph.edgeInfo(i);
    update(mate, *e, *ee);

    while (true) {
      e = ee;
      if (takable(mate, *e)) break;
      if (!doNotTake(mate, *e)) return 0;

      if (++i == n) return -1;

      ee = &graph.edgeInfo(i);
      update(mate, *e, *ee);
    }

    assert(i < n);
    return n - i;
  }
};

#endif
