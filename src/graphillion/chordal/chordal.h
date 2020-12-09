#ifndef GRAPHILLION_CHORDAL_H_
#define GRAPHILLION_CHORDAL_H_

#include "subsetting/util/Graph.hpp"

#include "SAPPOROBDD/ZBDD.h"
#include "graphillion/graphset.h"

ZBDD constructChadalGraphs(const tdzdd::Graph &graph);

namespace graphillion {
setset SearchChordals(const std::vector<edge_t> &edges);
}  // namespace graphillion

#endif
