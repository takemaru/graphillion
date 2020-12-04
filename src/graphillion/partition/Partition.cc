#include "Partition.h"

#include "GraphRangePartitionSpec.h"
#include "graphillion/graphset.h"
#include "subsetting/eval/ToZBDD.hpp"

tdzdd::DdStructure<2> constructPartitionDd(const tdzdd::Graph &g,
                                           int16_t comp_lb, int16_t comp_ub) {
  tdzdd::DdStructure<2> dd;
  constexpr bool is_noloop = false;
  GraphPartitionSpec gpspec(g, comp_lb, comp_ub, is_noloop, true, false);
  dd = tdzdd::DdStructure<2>(gpspec);
  dd.zddReduce();
  return dd;
}

namespace graphillion {

setset SearchPartitions(const std::vector<edge_t> &edges, int16_t comp_lb,
                        int16_t comp_ub) {
  tdzdd::Graph g;
  for (const auto &e : edges) {
    g.addEdge(e.first, e.second);
  }
  g.update();

  auto dd = constructPartitionDd(g, comp_lb, comp_ub);
  dd.useMultiProcessors(false);
  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

}  // namespace graphillion
