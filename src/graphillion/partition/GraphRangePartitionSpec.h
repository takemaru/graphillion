#ifndef GRAPH_PARTITION_SPEC_H_
#define GRAPH_PARTITION_SPEC_H_

#include <numeric>

#include "subsetting/DdSpec.hpp"
#include "subsetting/util/Graph.hpp"

// [lb, ub] closed interval!!
struct GraphPartitionSpecCount {
  using count_t = int16_t;
  count_t comp_lb, comp_ub;  ///< uncolored edge component counter.

  GraphPartitionSpecCount()
      : comp_lb(1), comp_ub(std::numeric_limits<count_t>::max()) {}

  GraphPartitionSpecCount(count_t _lb, count_t _ub)
      : comp_lb(_lb), comp_ub(_ub) {}

  size_t hash() const { return (size_t(comp_lb) << 16) | comp_ub; }

  bool operator==(GraphPartitionSpecCount const& o) const {
    return comp_lb == o.comp_lb && comp_ub == o.comp_ub;
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  GraphPartitionSpecCount const& o) {
    return os << o.comp_lb << ", " << o.comp_ub;
  }
};

static int16_t const UNCOLORED = 32766;
static int16_t const UNCOLORED_EDGE_COMPONENT = 32767;

union GraphPartitionSpecMate {
 public:
  typedef int16_t Offset;
  typedef uint16_t uOffset;

 private:
  struct {
    Offset hoc;  ///< offset to head or color.
    Offset nxt;  ///< offset to next connected vertex.
  };
  uint32_t fps_bits;

 public:
  GraphPartitionSpecMate(Offset hoc = 0) : hoc(hoc), nxt(0) {}

  bool operator==(GraphPartitionSpecMate const& o) const { return this == &o; }

  bool operator!=(GraphPartitionSpecMate const& o) const { return this != &o; }

  void clear() {
    hoc = 0;
    nxt = 0;
  }

  bool isHead() const { return hoc >= 0; }

  bool isTail() const { return nxt == 0; }

  bool isIsolated() const { return isHead() && isTail(); }

  GraphPartitionSpecMate& head() { return isHead() ? *this : *(this + hoc); }

  GraphPartitionSpecMate const& head() const {
    return isHead() ? *this : *(this + hoc);
  }

  GraphPartitionSpecMate& next() { return *(this + nxt); }

  GraphPartitionSpecMate const& next() const { return *(this + nxt); }

  bool isColored() const { return head().hoc < UNCOLORED; }

  bool isUncoloredEdgeComponent() const {
    return head().hoc == UNCOLORED_EDGE_COMPONENT;
  }

  bool isColoredTail() const {
    return hoc == 0 || (hoc < 0 && hoc + (this + hoc)->hoc == 0);
  }

  bool hasSameColorAs(GraphPartitionSpecMate const& o) const {
    GraphPartitionSpecMate const* p = &head();
    GraphPartitionSpecMate const* q = &o.head();
    return p + p->hoc == q + q->hoc;
  }

  GraphPartitionSpecMate const* findColorPredecessor(
      GraphPartitionSpecMate const& o) const {
    assert(o.isColoredTail());
    GraphPartitionSpecMate const* p = &o;

    while (--p >= this) {
      GraphPartitionSpecMate const* p1 = &p->head();
      if (p1 + p1->hoc == &o) return p;
    }

    return 0;
  }

  void print() const { std::cerr << "(" << hoc << "," << nxt << ")"; }

  void print(std::ostream& ost) const {
    ost << "(" << hoc << "," << nxt << ")";
  }

  void print(FILE* fp) const { fprintf(fp, "(%d,%d)", hoc, nxt); }

  void mergeLists(GraphPartitionSpecMate& o1, GraphPartitionSpecMate& o2,
                  bool countUEC) {
    GraphPartitionSpecMate* p1 = &o1.head();
    GraphPartitionSpecMate* p2 = &o2.head();
    if (p1 == p2) return;
    if (p1 > p2) std::swap(p1, p2);

    bool painting;  // merging colored and uncolored

    if (p2->hoc < UNCOLORED) {
      painting = (p1->hoc >= UNCOLORED);

      if (painting || p1 + p1->hoc < p2 + p2->hoc) {
        p1->hoc =
            p2->hoc + (p2 - p1);  // updated if uncolored element becomes tail
      }
    } else {
      painting = (p1->hoc < UNCOLORED);

      if (countUEC && p1->hoc == UNCOLORED) {
        p1->hoc = UNCOLORED_EDGE_COMPONENT;
      }
    }

    for (GraphPartitionSpecMate* q = p2;; q += q->nxt) {
      q->hoc = p1 - q;
      if (q->nxt == 0) break;
    }

    GraphPartitionSpecMate* p = p1;
    GraphPartitionSpecMate* q = p2;

    while (true) {
      assert(p != q);
      GraphPartitionSpecMate* pp = p + p->nxt;
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

    if (painting) {
      while (q->nxt) {
        q += q->nxt;
      }

      GraphPartitionSpecMate* pp = p1 + p1->hoc;

      if (pp < q) {  // q must be an uncolored tail.
        for (p = this; p <= pp; ++p) {
          if (p + p->hoc == pp) p->hoc = q - p;
        }
      }
    }
  }

  void replaceHeadWith(GraphPartitionSpecMate& newHead) const {
    GraphPartitionSpecMate const* p = &head();
    GraphPartitionSpecMate* q = &newHead;

    q->hoc = (p->hoc < UNCOLORED) ? p->hoc + (p - q) : p->hoc;

    while (q->nxt > 0) {
      q += q->nxt;
      q->hoc = &newHead - q;
    }
  }

  void removeFromList(GraphPartitionSpecMate const& o) {
    if (o.isColoredTail()) {
      assert(o.nxt == 0);
      GraphPartitionSpecMate const* pp = findColorPredecessor(o);
      if (pp == 0) return;

      for (GraphPartitionSpecMate* p = this; p <= pp; ++p) {
        if (p + p->hoc == &o) p->hoc = pp - p;
        if (p + p->nxt == &o) p->nxt = 0;
      }
    } else if (o.nxt == 0) {
      for (GraphPartitionSpecMate* p = this; p <= &o; ++p) {
        if (p + p->nxt == &o) p->nxt = 0;
      }
    } else {
      for (GraphPartitionSpecMate* p = this; p <= &o; ++p) {
        if (p + p->nxt == &o) p->nxt += o.nxt;
      }
    }
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  GraphPartitionSpecMate const& o) {
    return os << "<" << o.hoc << "," << o.nxt << ">";
  }

  friend class GraphPartitionSpec;
};

class GraphPartitionSpec
    : public tdzdd::HybridDdSpec<GraphPartitionSpec, GraphPartitionSpecCount,
                                 GraphPartitionSpecMate, 2> {
  typedef GraphPartitionSpecCount Count;
  typedef GraphPartitionSpecMate Mate;

  tdzdd::Graph const& graph;
  int const m;
  int const n;
  int const mateSize;
  int const fpsCellSize;
  int const fpsSize;
  std::vector<Mate> initialMate;
  GraphPartitionSpecCount::count_t comp_lb, comp_ub;
  bool const noLoop;
  bool const lookahead;
  bool const countUEC;

  void setF(Mate::Offset v1, Mate::Offset v2, Mate* mate, int value) const {
    assert(value == 0 || value == 1);
    assert(v1 != v2);
    if (v1 > v2) {
      std::swap(v1, v2);
    }
    assert(0 <= v1 && v2 < mateSize);
    int p = -v1 * (v1 + 3) / 2 + v1 * mateSize - 1 + v2;
    if (value == 1) {
      mate[mateSize + p / fpsCellSize].fps_bits |= (1u << (p % fpsCellSize));
    } else {
      mate[mateSize + p / fpsCellSize].fps_bits &= ~(1u << (p % fpsCellSize));
    }
  }

  int getF(Mate::Offset v1, Mate::Offset v2, Mate const* mate) const {
    assert(v1 != v2);
    if (v1 > v2) {
      std::swap(v1, v2);
    }
    assert(0 <= v1 && v2 < mateSize);
    int p = -v1 * (v1 + 3) / 2 + v1 * mateSize - 1 + v2;
    return ((mate[mateSize + p / fpsCellSize].fps_bits >> (p % fpsCellSize)) &
            1u);
  }

  void addToFPS(Mate::Offset v1, Mate::Offset v2, Mate* mate) const {
    setF(v1, v2, mate, 1);
  }

  void changeRepFPS(Mate::Offset v1, Mate::Offset v2, Mate* mate) const {
    for (int i = 0; i < mateSize; ++i) {
      if (i == v1 || i == v2) {
        continue;
      }
      assert(getF(v2, i, mate) == 0);
      setF(v2, i, mate, getF(v1, i, mate));
    }
  }

  void mergeFPS(Mate::Offset v1, Mate::Offset v2, Mate* mate) const {
    if (v1 > v2) {
      std::swap(v1, v2);
    }
    for (int i = 0; i < mateSize; ++i) {
      if (i == v1 || i == v2) {
        continue;
      }
      if (getF(v2, i, mate) != 0) {
        setF(v1, i, mate, 1);
      }
      setF(v2, i, mate, 0);
    }
  }

  void shiftFPS(Mate::Offset d, Mate* mate) const {
    for (int i = 0; i < mateSize - 1; ++i) {
      int j;
      for (j = i + 1; j < mateSize - d; ++j) {
        setF(i, j, mate, getF(i + d, j + d, mate));
      }
      for (; j < mateSize; ++j) {
        setF(i, j, mate, 0);
      }
    }
  }

  bool isInFPS(Mate::Offset v1, Mate::Offset v2, Mate const* mate) const {
    return getF(v1, v2, mate) != 0;
  }

  int takable(Count& c, Mate const* mate,
              tdzdd::Graph::EdgeInfo const& e) const {
    Mate const& w1 = mate[e.v1 - e.v0];
    Mate const& w2 = mate[e.v2 - e.v0];

    // don't connect again
    if (noLoop && w1.head() == w2.head()) return false;

    // don't connect different colors
    if (w1.isColored() && w2.isColored() && !w1.hasSameColorAs(w2))
      return false;

    if (w1.head() != w2.head()) {  // v1 and v2 are in distinct components
      if (isInFPS(&w1.head() - mate, &w2.head() - mate, mate)) {
        return false;
      }
    }

    if (e.v1final && e.v2final) {
      if (w1.isIsolated() &&
          w2.isIsolated()) {  // new component leaves immediately
        if (w2.isColored()) {
          // don't leave the color unconnected
          if (!w2.isColoredTail()) return false;
          if (mate[1].findColorPredecessor(w2)) return false;
        } else {
          if (w1.isColored()) {
            // don't leave the color unconnected
            if (!w1.isColoredTail()) return false;
          } else {
            if (c.comp_ub == 0) return false;
            if (c.comp_ub > 0) {
              c.comp_lb--;
              c.comp_ub--;
            }
          }
        }
      } else if (w1.isHead() && w2 == w1.next() &&
                 w2.isTail()) {  // existing component leaves
        if (w1.isColored()) {
          // don't leave the color unconnected
          if (!w2.isColoredTail()) return false;
          if (w2.findColorPredecessor(mate[1])) return false;
        } else {
          assert(!countUEC || w1.isUncoloredEdgeComponent());
          if (c.comp_ub == 0) return false;
          if (c.comp_ub > 0) {
            c.comp_lb--;
            c.comp_ub--;
          }
        }
      }
    }

    if (e.finalEdge && c.comp_lb > 0) return false;
    return true;
  }

  bool doTake(Count& count, Mate* mate, tdzdd::Graph::EdgeInfo const& e) const {
    Count c = count;

    if (!takable(c, mate, e)) return false;

    count = c;

    Mate const& w1 = mate[e.v1 - e.v0];
    Mate const& w2 = mate[e.v2 - e.v0];
    if (w1.head() != w2.head()) {
      mergeFPS(&w1.head() - mate, &w2.head() - mate, mate);
    }

    mate[0].mergeLists(mate[e.v1 - e.v0], mate[e.v2 - e.v0], countUEC);

    return true;
  }

  bool doNotTake(Count& count, Mate* mate,
                 tdzdd::Graph::EdgeInfo const& e) const {
    Count c = count;
    Mate const& w1 = mate[e.v1 - e.v0];
    Mate const& w2 = mate[e.v2 - e.v0];

    if (w1.head() == w2.head()) {
      return false;
    }
    addToFPS(&w1.head() - mate, &w2.head() - mate, mate);

    if (e.v1final && w1.isIsolated()) {
      if (w1.isColored()) {
        // don't leave the color unconnected
        if (!w1.isColoredTail()) return false;
      } else if (c.comp_ub >= 0) {
        if (!countUEC || w1.isUncoloredEdgeComponent()) {
          if (c.comp_ub == 0) return false;
          c.comp_lb--;
          c.comp_ub--;
        }
      }
    }

    if (e.v2final && w2.isIsolated()) {
      if (w2.isColored()) {
        // don't leave the color unconnected
        if (!w2.isColoredTail()) return false;
        if (mate[1].findColorPredecessor(w2)) return false;
      } else if (c.comp_ub >= 0) {
        if (!countUEC || w2.isUncoloredEdgeComponent()) {
          if (c.comp_ub == 0) return false;
          c.comp_lb--;
          c.comp_ub--;
        }
      }
    }

    if (e.v1final && e.v2final && w1.isHead() && w2 == w1.next() &&
        w2.isTail()) {  // existing component leaves) {
      if (w1.isColored()) {
        // don't leave the color unconnected
        if (!w2.isColoredTail()) return false;
        if (w2.findColorPredecessor(mate[1])) return false;
      } else {
        assert(!countUEC || w1.isUncoloredEdgeComponent());
        if (c.comp_ub == 0) return false;
        if (c.comp_ub > 0) {
          c.comp_lb--;
          c.comp_ub--;
        }
      }
    }

    if (e.finalEdge && c.comp_lb > 0) return false;
    count = c;
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
        changeRepFPS(&q->head() - mate, qq - mate, mate);
        q->replaceHeadWith(*qq);
      }
    }

    // update fps
    if (d > 0) {
      shiftFPS(d, mate);
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
  GraphPartitionSpec(
      tdzdd::Graph const& graph, GraphPartitionSpecCount::count_t _lb = 1,
      GraphPartitionSpecCount::count_t _ub =
          std::numeric_limits<GraphPartitionSpecCount::count_t>::max(),
      bool noLoop = false, bool lookahead = true, bool countUEC = true)
      : graph(graph),
        m(graph.vertexSize()),
        n(graph.edgeSize()),
        mateSize(graph.maxFrontierSize()),
        fpsCellSize(2 * sizeof(Mate::Offset) * 8),
        fpsSize(((mateSize - 1) * mateSize / 2 - 1) / fpsCellSize + 1),
        initialMate(1 + m + mateSize),
        comp_lb(_lb),
        comp_ub(_ub),
        noLoop(noLoop),
        lookahead(lookahead),
        countUEC(countUEC) {
    this->setArraySize(mateSize + fpsSize);

    std::vector<int> rootOfColor(graph.numColor() + 1);
    for (int v = 1; v <= m; ++v) {
      rootOfColor[graph.colorNumber(v)] = v;
    }

    for (int v = 1; v <= m; ++v) {
      int k = graph.colorNumber(v);
      int hoc = (k > 0) ? rootOfColor[k] - v : UNCOLORED;
      initialMate[v] = Mate(hoc);
    }
  }

  int getRoot(Count& count, Mate* mate) const {
    int const v0 = graph.edgeInfo(0).v0;

    count = Count{comp_lb, comp_ub};

    for (int i = 0; i < mateSize; ++i) {
      mate[i] = initialMate[v0 + i];
    }

    // for FPS
    for (int i = 0; i < fpsSize; ++i) {
      mate[mateSize + i].fps_bits = 0;
    }

    return n;
  }

  int getChild(Count& count, Mate* mate, int level, int take) const {
    assert(1 <= level && level <= n);
    int i = n - level;
    tdzdd::Graph::EdgeInfo const* e = &graph.edgeInfo(i);

    if (take) {
      if (!doTake(count, mate, *e)) return 0;
    } else {
      if (!doNotTake(count, mate, *e)) return 0;
    }

    if (++i == n) return -1;

    tdzdd::Graph::EdgeInfo const* ee = &graph.edgeInfo(i);
    update(mate, *e, *ee);

    while (lookahead) {
      e = ee;

      Count c = count;
      if (takable(c, mate, *e)) break;
      if (!doNotTake(count, mate, *e)) return 0;

      if (++i == n) return -1;

      ee = &graph.edgeInfo(i);
      update(mate, *e, *ee);
    }

    assert(i < n);
    return n - i;
  }

  size_t hashCode(Count const& count) const { return count.hash(); }
};

#endif  // GRAPH_PARTITION_SPEC_H_
