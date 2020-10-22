#ifndef GRAPHILLION_PARTITION_H_
#define GRAPHILLION_PARTITION_H_

#include "graphillion/setset.h"
#include "graphillion/type.h"
#include "subsetting/DdStructure.hpp"
#include "subsetting/util/Graph.hpp"

/**
 * @brief construct a DD to enumerate partitions that divide the graph into
 *        more than or equal to lb and less than or equal to ub components.
 *
 * @param g      : input graph.
 * @param comp_lb: the minimam number of components.
 * @param comp_ub: the maximum number of components.
 */
tdzdd::DdStructure<2> constructPartitionDd(
    const tdzdd::Graph &g, int16_t comp_lb = 1,
    int16_t comp_ub = std::numeric_limits<int16_t>::max());

namespace graphillion {

setset SearchPartitions(const std::vector<edge_t> &edges, int16_t comp_lb,
                        int16_t comp_ub);

}  // namespace graphillion

#endif  // GRAPHILLION_PARTITION_H_
