#ifndef GRAPHILLION_DEGREE_DISTRIBUTION_GRAPHS_H_
#define GRAPHILLION_DEGREE_DISTRIBUTION_GRAPHS_H_

#include "graphillion/type.h"

namespace graphillion {
/**
 * @return A setset that represents the set of subgraphs having specified degree distribution.
 */
setset
SearchDegreeDistributionGraphs(const std::vector<edge_t> &edges,
                                const std::vector<int>& degRanges,
                                const bool is_connected);
}  // namespace graphillion

#endif
