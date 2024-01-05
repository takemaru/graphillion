#include "BalancedPartition.h"

#include <numeric>

#include "ComponentRatioSpec.h"
#include "ComponentWeightSpec.h"
#include "graphillion/partition/Partition.h"
#include "subsetting/DdStructure.hpp"
#include "subsetting/eval/ToZBDD.hpp"
#include "subsetting/util/Graph.hpp"
#include "graphillion/convert_weight_list.h"

/**
 * @param g:           input graph.
 * @param weight_list: list of the vertex weights.
 * @param ratio:       the maximum value of the allowed disparity.
 * @param k:           the number of connected components.
 *                     If not specified, enumerates partitions that devide
 *                     the graph into any connected components.
 */
tdzdd::DdStructure<2> constructRatioDd(const tdzdd::Graph &g,
                                       const std::vector<weight_t> &weight_list,
                                       double ratio, weight_t _lower,
                                       weight_t _upper, int k = -1) {
  auto dd = (k == -1 ? constructPartitionDd(g) : constructPartitionDd(g, k, k));

  auto sum =
      std::accumulate(weight_list.begin(), weight_list.end(), weight_t{0});
  weight_t initial_lower = 0;
  if (k != -1) {
    initial_lower = std::max(
        initial_lower, static_cast<weight_t>(floor(static_cast<double>(sum) /
                                                   (ratio * (k - 1) + 1))));
  }
  weight_t initial_upper = sum;
  if (k != -1) {
    initial_upper =
        std::min(initial_upper,
                 static_cast<weight_t>(ceil(ratio * static_cast<double>(sum) /
                                            (ratio + (k - 1)))));
  }

  ComponentWeightSpec cwspec(g, weight_list, initial_lower, initial_upper);
  dd.zddSubset(cwspec);
  dd.zddReduce();

  ComponentRatioSpec crspec(g, weight_list, _lower, _upper, ratio);
  dd.zddSubset(crspec);
  dd.zddReduce();

  return dd;
}

/**
 * @param g:           input graph.
 * @param weight_list: list of the vertex weights.
 * @param lower:       the minimum weights of a connected component.
 * @param upper:       the maximum weights of a connected component.
 * @param k:           the number of connected components.
 *                     If not specified, enumerates partitions that devide
 *                     the graph into any connected components.
 */
tdzdd::DdStructure<2> constructWeightDd(
    const tdzdd::Graph &g, const std::vector<weight_t> &weight_list,
    weight_t lower = 0, weight_t upper = std::numeric_limits<weight_t>::max(),
    int k = -1) {
  auto dd = (k == -1 ? constructPartitionDd(g) : constructPartitionDd(g, k, k));

  ComponentWeightSpec cwspec(g, weight_list, lower, upper);
  dd.zddSubset(cwspec);
  dd.zddReduce();
  return dd;
}

namespace graphillion {

setset SearchRatioPartitions(const std::vector<edge_t> &edges,
                             const std::map<std::string, weight_t> &weight_list,
                             double ratio, weight_t lower, weight_t upper,
                             int k) {
  tdzdd::Graph g;
  for (const auto &e : edges) {
    g.addEdge(e.first, e.second);
  }
  g.update();

  auto dd = constructRatioDd(g, convert_weight_list<weight_t>(g, weight_list), ratio,
                             lower, upper, k);
  dd.useMultiProcessors(false);
  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

setset SearchWeightPartitions(
    const std::vector<edge_t> &edges,
    const std::map<std::string, weight_t> &weight_list, weight_t lower,
    weight_t upper, int k) {
  tdzdd::Graph g;
  for (const auto &e : edges) {
    g.addEdge(e.first, e.second);
  }
  g.update();

  auto dd = constructWeightDd(g, convert_weight_list<weight_t>(g, weight_list), lower,
                              upper, k);
  dd.useMultiProcessors(false);
  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

setset SearchBalancedPartitions(const std::vector<edge_t> &edges,
                           const std::map<std::string, weight_t> &weight_list,
                           double ratio, weight_t lower, weight_t upper,
                           int k) {
  if (ratio < 1.0) {
    return SearchWeightPartitions(edges, weight_list, lower, upper, k);
  }
  return SearchRatioPartitions(edges, weight_list, ratio, lower, upper, k);
}

}  // namespace graphillion
