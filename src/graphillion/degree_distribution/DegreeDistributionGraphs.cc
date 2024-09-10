#include "DegreeDistributionSpec.h"

#include "graphillion/setset.h"
#include "graphillion/graphset.h"
#include "subsetting/util/IntSubset.hpp"
#include "subsetting/eval/ToZBDD.hpp"

tdzdd::DdStructure<2>
constructDegreeDistributionGraphs(const tdzdd::Graph &g,
                                  const std::vector<int>& degRanges,
                                  const bool is_connected) {

  std::vector<tdzdd::IntSubset*> dr;

  for (size_t i = 0; i < degRanges.size(); ++i) {
    if (degRanges[i] < 0) {
      dr.push_back(new tdzdd::IntRange(0, g.vertexSize()));
    } else {
      dr.push_back(new tdzdd::IntRange(degRanges[i], degRanges[i]));
    }
  }

  DegreeDistributionSpec ddspec(g, dr, is_connected);

#ifdef _OPENMP
  bool use_mp = (omp_get_num_procs() >= 2);
#else
  bool use_mp = false;
#endif

  auto dd = tdzdd::DdStructure<2>(ddspec, use_mp);
  dd.zddReduce();

  for (size_t i = 0; i < dr.size(); ++i) {
    delete dr[i];
  }
  return dd;
}

namespace graphillion {

setset
SearchDegreeDistributionGraphs(const std::vector<edge_t> &edges,
                                const std::vector<int>& degRanges,
                                const bool is_connected) {
  tdzdd::Graph g;
  for (const auto &e : edges) {
    g.addEdge(e.first, e.second);
  }
  g.update();

  auto dd = constructDegreeDistributionGraphs(g, degRanges, is_connected);
  dd.useMultiProcessors(false);
  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

}  // namespace graphillion
