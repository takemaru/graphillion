#include "graphillion/setset.h"

#include "subsetting/DdSpec.hpp"
#include "subsetting/DdStructure.hpp"
#include "subsetting/eval/ToZBDD.hpp"
#include "subsetting/util/Graph.hpp"

#include <assert.h>

class OddEdgeSubgraphSpec : public tdzdd::DdSpec<OddEdgeSubgraphSpec, bool, 2> {
  const int n;

 public:
  OddEdgeSubgraphSpec(tdzdd::Graph const& _g) : n(_g.edgeSize()) {}

  int getRoot(bool& is_odd) const {
    is_odd = false;
    return n;
  }

  int getChild(bool& is_odd, int level, int take) const {
    assert(1 <= level && level <= n);

    if (take) {
      is_odd = !is_odd;
    }
    if (--level == 0) return (is_odd ? -1 : 0);
    return level;
  }
};

namespace graphillion {
setset SearchOddEdgeSubgraphs(const std::vector<edge_t>& edges) {
  tdzdd::Graph g;
  for (const auto& e : edges) {
    g.addEdge(e.first, e.second);
  }
  g.update();

#ifdef _OPENMP
  bool use_mp = (omp_get_num_procs() >= 2);
#else
  bool use_mp = false;
#endif

  OddEdgeSubgraphSpec spec(g);
  auto dd = tdzdd::DdStructure<2>(spec, use_mp);
  dd.zddReduce();

  dd.useMultiProcessors(false);
  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}
}  // namespace graphillion
