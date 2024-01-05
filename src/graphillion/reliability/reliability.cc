/*
 * Reliability: Computing the network reliability with TdZdd
 */

#include "reliability.h"

#include "subsetting/DdSpec.hpp"
#include "subsetting/DdStructure.hpp"
#include "subsetting/spec/FrontierBasedSearch.hpp"
#include "subsetting/util/Graph.hpp"

class ProbEval : public tdzdd::DdEval<ProbEval, double> {
 private:
  std::vector<double> prob_list_;

 public:
  ProbEval(const std::vector<double>& prob_list) : prob_list_(prob_list) {}

  void evalTerminal(double& p, bool one) const { p = one ? 1.0 : 0.0; }

  void evalNode(double& p, int level,
                tdzdd::DdValues<double, 2> const& values) const {
    double pc = prob_list_[prob_list_.size() - level];
    p = values.get(0) * (1 - pc) + values.get(1) * pc;
  }
};

namespace graphillion {

double reliability(const std::vector<edge_t>& edges,
                   const std::vector<double>& prob_list,
                   const std::vector<std::string>& terminals) {
  tdzdd::Graph g;
  for (const auto& e : edges) {
    g.addEdge(e.first, e.second);
  }
  g.update();

  for (const auto& v : terminals) {
    g.setColor(v, 1);
  }
  g.update();

  // look ahead cannot be used for BDDs,
  // so set the 4th argment to false
  tdzdd::FrontierBasedSearch fbs(g, -1, false, false);

#ifdef _OPENMP
  bool use_mp = (omp_get_num_procs() >= 2);
#else
  bool use_mp = false;
#endif

  tdzdd::DdStructure<2> dd(fbs, use_mp);
  dd.bddReduce();
  dd.useMultiProcessors(false);
  return dd.evaluate(ProbEval(prob_list));
}

}  // namespace graphillion
