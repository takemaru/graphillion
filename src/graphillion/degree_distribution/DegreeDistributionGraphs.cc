#include "DegreeDistributionSpec.h"

#include "graphillion/setset.h"
#include "graphillion/graphset.h"
#include "subsetting/util/IntSubset.hpp"
#include "subsetting/eval/ToZBDD.hpp"
#include "subsetting/spec/SapporoZdd.hpp"

tdzdd::DdStructure<2>
constructDegreeDistributionGraphs(const tdzdd::Graph &g,
                                  const std::vector<int>& degRanges,
                                  const bool is_connected,
                                  const graphillion::zdd_t* search_space,
                                  const int offset) {

#ifdef _OPENMP
  bool use_mp = (omp_get_num_procs() >= 2);
#else
  bool use_mp = false;
#endif

  tdzdd::DdStructure<2> dd;
  if (search_space != NULL) {
    SapporoZdd f(*search_space, offset);
    dd = tdzdd::DdStructure<2>(f, use_mp);
  } else {
    dd = tdzdd::DdStructure<2>(g.edgeSize(), use_mp);
  }

  std::vector<tdzdd::IntSubset*> dr;

  for (size_t i = 0; i < degRanges.size(); ++i) {
    if (degRanges[i] < 0) {
      dr.push_back(new tdzdd::IntRange(0, g.vertexSize()));
    } else {
      dr.push_back(new tdzdd::IntRange(degRanges[i], degRanges[i]));
    }
  }

  DegreeDistributionSpec ddspec(g, dr, is_connected);

  dd.zddSubset(ddspec);
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
                                const bool is_connected,
                                const setset* search_space) {
  tdzdd::Graph g;
  for (const auto &e : edges) {
    g.addEdge(e.first, e.second);
  }
  g.update();

  const zdd_t* search_space_z =
  ((search_space == NULL) ? NULL : &search_space->zdd_);

  auto dd = constructDegreeDistributionGraphs(g, degRanges, is_connected,
    search_space_z, setset::max_elem() - setset::num_elems());
  dd.useMultiProcessors(false);
  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

}  // namespace graphillion
