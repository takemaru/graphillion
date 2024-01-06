#ifndef GRAPHILLION_CHORDAL_H_
#define GRAPHILLION_CHORDAL_H_

#include "subsetting/util/Graph.hpp"

#include "SAPPOROBDD/ZBDD.h"
#include "graphillion/graphset.h"

namespace graphillion {
setset SearchChordals(const std::vector<edge_t> &edges, const uint32_t k = 4);
}  // namespace graphillion

#endif
