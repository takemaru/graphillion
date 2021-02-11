#include "graphillion/setset.h"
#include "subsetting/util/Graph.hpp"

namespace graphillion {
setset SearchWeightedInducedGraphs(
    const std::vector<edge_t> &edges,
    const std::map<std::string, uint32_t> &weight_list, uint32_t lower,
    uint32_t upper);
}  // namespace graphillion
