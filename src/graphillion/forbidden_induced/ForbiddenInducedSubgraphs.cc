#include "ForbiddenInducedSubgraphs.h"

#include "InducingColoringSpec.h"
#include "InducingDecoloringEval.h"
//#include "SBDD_helper.h"
#include "subsetting/DdStructure.hpp"
#include "subsetting/spec/SizeConstraint.hpp"
#include "subsetting/util/IntSubset.hpp"
#include "subsetting/spec/SapporoZdd.hpp"

#include "graphillion/setset.h"

ZBDD constructForbiddenInducedSubgraphs(const tdzdd::Graph &graph, const tdzdd::DdStructure<2>& dd) {
  const int m = graph.edgeSize();

#ifdef _OPENMP
  bool use_mp = (omp_get_num_procs() >= 2);
#else
  bool use_mp = false;
#endif

  // construct 3-DD
  InducingColoringSpec coloringSpec(graph, dd);
  auto DD3 = tdzdd::DdStructure<3>(coloringSpec, use_mp);

  // construct 2-DD representing the set of subgraphs
  // each of which contains a subgraph as an induced subgraph
  DD3.useMultiProcessors(false);
  int offset =
      graphillion::setset::max_elem() - graphillion::setset::num_elems();
  ZBDD new_dd = DD3.evaluate(InducingDecoloringEval(offset));

  ZBDD powerSetDD = ZBDD(1);
  for (bddvar v = 1; v <= static_cast<bddvar>(m); ++v) {
    powerSetDD += powerSetDD.Change(v);
  }
  //std::vector<bddvar> vararr;
  //for (bddvar v = offset + 1; v <= static_cast<bddvar>(m + offset); ++v) {
  //  vararr.push_back(v);
  //}
  //ZBDD powerSetDD2 = sbddh::getPowerSet(vararr);
  ZBDD resultDD = powerSetDD - new_dd;

  return resultDD;
};

namespace graphillion {
setset SearchForbiddenInducedSubgraphs(const std::vector<edge_t> &edges, setset* ss) {
  tdzdd::Graph g;
  for (const auto &e : edges) {
    g.addEdge(e.first, e.second);
  }
  g.update();

  int offset =
      graphillion::setset::max_elem() - graphillion::setset::num_elems();
  SapporoZdd szdd(ss->zdd_, offset);
  auto new_dd = constructForbiddenInducedSubgraphs(g,
    tdzdd::DdStructure<2>(szdd));
  return setset(new_dd);
}
}  // namespace graphillion
