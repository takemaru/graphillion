#include "InducedGraphs.h"
#include "ConnectedInducedSubgraphSpec.h"

#include "graphillion/graphset.h"
#include "subsetting/eval/ToZBDD.hpp"

tdzdd::DdStructure<2> constructInducedGraphs(const tdzdd::Graph &g) {
  ConnectedInducedSubgraphSpec scspec(g);

#ifdef _OPENMP
  bool use_mp = (omp_get_num_procs() >= 2);
#else
  bool use_mp = false;
#endif

  auto dd = tdzdd::DdStructure<2>(scspec, use_mp);
  dd.zddReduce();
  return dd;
}

namespace graphillion {

setset SearchInducedGraphs(const std::vector<edge_t> &edges) {
  tdzdd::Graph g;
  for (const auto &e : edges) {
    g.addEdge(e.first, e.second);
  }
  g.update();

  auto dd = constructInducedGraphs(g);
  dd.useMultiProcessors(false);
  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

}  // namespace graphillion

