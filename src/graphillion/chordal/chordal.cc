#include "chordal.h"

#include "FrontierSingleCycle.h"
#include "../forbidden_induced/InducingColoringSpec.h"
#include "../forbidden_induced/InducingDecoloringEval.h"
#include "SBDD_helper.h"
#include "subsetting/DdStructure.hpp"
#include "subsetting/spec/SizeConstraint.hpp"
#include "subsetting/util/IntSubset.hpp"

#include "graphillion/setset.h"

ZBDD constructChordalGraphs(const tdzdd::Graph &graph, const uint32_t k) {
  const int m = graph.edgeSize();

#ifdef _OPENMP
  bool use_mp = (omp_get_num_procs() >= 2);
#else
  bool use_mp = false;
#endif

  // construct 2-DD reprsenting the set of all the cycles
  FrontierSingleCycleSpec cycleSpec(graph);
  auto cycleDD = tdzdd::DdStructure<2>(cycleSpec, use_mp);

  tdzdd::IntRange r(k);  // at least 4 edges
  tdzdd::SizeConstraint sc(graph.edgeSize(), &r);
  // construct 2-DD reprsenting the set of all the cycles
  // with length at least four
  cycleDD.zddSubset(sc);

  // construct 3-DD
  InducingColoringSpec coloringSpec(graph, cycleDD);
  auto DD3 = tdzdd::DdStructure<3>(coloringSpec, use_mp);

  // construct 2-DD representing the set of subgraphs
  // each of which contains a cycle with length at least
  // four as an induced subgraph
  DD3.useMultiProcessors(false);
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
setset SearchChordals(const std::vector<edge_t> &edges, const uint32_t k) {
  tdzdd::Graph g;
  for (const auto &e : edges) {
    g.addEdge(e.first, e.second);
  }
  g.update();

  auto dd = constructChordalGraphs(g, k);
  return setset(dd);
}
}  // namespace graphillion
