#ifndef GRAPHILLION_DEGREE_DISTRIBUTION_GRAPHS_H_
#define GRAPHILLION_DEGREE_DISTRIBUTION_GRAPHS_H_

#include "graphillion/type.h"
#include "graphillion/setset.h"

namespace graphillion {
/**
 * @return A setset that represents the set of subgraphs having specified degree distribution.
 */
setset
SearchDegreeDistributionGraphs(const std::vector<edge_t> &edges,
                                const std::vector<int>& degRanges,
                                const bool is_connected,
                                const setset* search_space);
}  // namespace graphillion

#endif
