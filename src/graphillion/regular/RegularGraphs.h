#ifndef GRAPHILLION_REGULAR_GRAPHS_H_
#define GRAPHILLION_REGULAR_GRAPHS_H_

#include "graphillion/type.h"
#include "graphillion/setset.h"

namespace graphillion {
/**
 * @return A setset that represents the set of regular subgraphs.
 */
setset
SearchRegularGraphs(const std::vector<edge_t> &edges,
                    const int degree_lower,
                    const int degree_upper,
                    const bool is_connected,
                    const setset* search_space);
}  // namespace graphillion

#endif
