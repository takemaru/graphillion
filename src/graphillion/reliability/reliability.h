/*
 * Reliability: Computing the network reliability with TdZdd
 */

#include <string>
#include <vector>

#include "graphillion/type.h"

namespace graphillion {

double reliability(const std::vector<edge_t>& edges,
                   const std::vector<double>& prob_list,
                   const std::vector<std::string>& terminals);
}  // namespace graphillion
