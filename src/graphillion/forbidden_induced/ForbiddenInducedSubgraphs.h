#ifndef GRAPHILLION_FORBIDDEN_INDUCED_SUBGRAPHS_H_
#define GRAPHILLION_FORBIDDEN_INDUCED_SUBGRAPHS_H_

#include "subsetting/util/Graph.hpp"

#include "SAPPOROBDD/ZBDD.h"
#include "graphillion/type.h"
#include "graphillion/graphset.h"

namespace graphillion {
setset SearchForbiddenInducedSubgraphs(const std::vector<edge_t> &edges, setset* ss);
}  // namespace graphillion

#endif
