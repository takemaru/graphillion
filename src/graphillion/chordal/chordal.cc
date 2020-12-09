#include "chordal.h"

#include "FrontierSingleCycle.h"
#include "InducingColoringSpec.h"
#include "InducingDecoloringEval.h"
#include "sbdd_helper.h"
#include "subsetting/DdStructure.hpp"
#include "subsetting/spec/SizeConstraint.hpp"
#include "subsetting/util/IntSubset.hpp"

#include "graphillion/setset.h"

ZBDD constructChadalGraphs(const tdzdd::Graph &graph) {
  const int m = graph.edgeSize();

  // construct 2-DD reprsenting the set of all the cycles
  FrontierSingleCycleSpec cycleSpec(graph);
  tdzdd::DdStructure<2> cycleDD = tdzdd::DdStructure<2>(cycleSpec);

  tdzdd::IntRange r(4);  // at least 4 edges
  tdzdd::SizeConstraint sc(graph.edgeSize(), &r);
  // construct 2-DD reprsenting the set of all the cycles
  // with length at least four
  cycleDD.zddSubset(sc);

  // construct 3-DD
  InducingColoringSpec coloringSpec(graph, cycleDD);
  tdzdd::DdStructure<3> DD3 = tdzdd::DdStructure<3>(coloringSpec);

  // construct 2-DD representing the set of subgraphs
  // each of which contains a cycle with length at least
  // four as an induced subgraph
  int offset =
      graphillion::setset::max_elem() - graphillion::setset::num_elems();
  ZBDD dd = DD3.evaluate(InducingDecoloringEval(offset));

  std::vector<bddvar> vararr;
  for (bddvar v = offset + 1; v <= static_cast<bddvar>(m + offset); ++v) {
    vararr.push_back(v);
  }
  ZBDD powerSetDD = sbddh::getPowerSet(vararr);
  ZBDD chordalDD = powerSetDD - dd;

  return chordalDD;
};

namespace graphillion {
setset SearchChordals(const std::vector<edge_t> &edges) {
  tdzdd::Graph g;
  for (const auto &e : edges) {
    g.addEdge(e.first, e.second);
  }
  g.update();

  auto dd = constructChadalGraphs(g);
  return setset(dd);
}
}  // namespace graphillion