#include "InducedGraphs.h"
#include "ConnectedInducedSubgraphSpec.hpp"

#include "graphillion/graphset.h"
#include "subsetting/eval/ToZBDD.hpp"

tdzdd::DdStructure<2> constructInducedGraphs(const tdzdd::Graph &g) {
  constexpr bool is_noloop = false;
  ConnectedInducedSubgraphSpec scspec(g, !is_noloop);
  auto dd = tdzdd::DdStructure<2>(scspec);
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
