#include "ComponentWeightInducedSpec.h"
#include "InducedGraphs.h"
#include "WeightedInducedGraphs.h"
#include "subsetting/eval/ToZBDD.hpp"

std::vector<uint32_t> convert_weight_list(
    const tdzdd::Graph &g,
    const std::map<std::string, uint32_t> &_weight_list) {
  std::vector<uint32_t> weight_list(g.vertexSize(), 1);
  for (const auto &p : _weight_list) {
    weight_list[g.getVertex(p.first) - 1] = p.second;
  }
  return weight_list;
}

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

  ComponentWeightInducedSpec cwispec(g, convert_weight_list(g, weight_list),
                                     lower, upper);
  dd.zddSubset(cwispec);
  dd.zddReduce();

  auto f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}
}  // namespace graphillion
