#ifndef GRAPHILLION_INDUCED_GRAPHS_H_
#define GRAPHILLION_INDUCED_GRAPHS_H_

#include "graphillion/setset.h"
#include "graphillion/type.h"
#include "subsetting/DdStructure.hpp"
#include "subsetting/util/Graph.hpp"

/**
 * @brief construct a DD to enumerate induced graphs from the given graph.
 *
 * @param g: input graph.
 */
tdzdd::DdStructure<2> constructInducedGraphs(const tdzdd::Graph &g);

namespace graphillion {
setset SearchInducedGraphs(const std::vector<edge_t> &edges);
}  // namespace graphillion

#endif  // GRAPHILLION_INDUCED_GRAPHS_H_
