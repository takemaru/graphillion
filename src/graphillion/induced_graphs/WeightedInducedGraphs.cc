#include "ComponentWeightInducedSpec.h"
#include "InducedGraphs.h"
#include "WeightedInducedGraphs.h"
#include "subsetting/eval/ToZBDD.hpp"
#include "graphillion/convert_weight_list.h"

namespace graphillion {
setset SearchWeightedInducedGraphs(
    const std::vector<edge_t> &edges,
    const std::map<std::string, uint32_t> &weight_list, uint32_t lower,
    uint32_t upper) {
  tdzdd::Graph g;
  for (const auto &e : edges) {
    g.addEdge(e.first, e.second);
  }
  g.update();
  auto dd = constructInducedGraphs(g);
  dd.zddReduce();

  ComponentWeightInducedSpec cwispec(g, convert_weight_list<uint32_t>(g, weight_list),
                                     lower, upper);
  dd.zddSubset(cwispec);
  dd.zddReduce();
  dd.useMultiProcessors(false);

  auto f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}
}  // namespace graphillion
