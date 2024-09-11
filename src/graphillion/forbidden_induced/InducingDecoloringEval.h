#ifndef GRAPHILLION_INDUCING_DECOLORING_EVAL_H_
#define GRAPHILLION_INDUCING_DECOLORING_EVAL_H_

#include "subsetting/DdEval.hpp"
#include "subsetting/DdSpec.hpp"
#include "subsetting/DdSpecOp.hpp"
#include "subsetting/DdStructure.hpp"

#include "SAPPOROBDD/ZBDD.h"

class InducingDecoloringEval
    : public tdzdd::DdEval<InducingDecoloringEval, ZBDD> {
 private:
  int offset_;

 public:
  InducingDecoloringEval(int offset = 0) : offset_(offset) {}

  void evalTerminal(ZBDD& zbdd, int id) const { zbdd = ZBDD(id); }

  void evalNode(ZBDD& zbdd, int level,
                const tdzdd::DdValues<ZBDD, 3>& values) const {
    ZBDD z0 = values.get(0) + values.get(2);
    ZBDD z1 = values.get(0) + values.get(1);
    //std::cerr << "level = " << level << ", offset_ = " << offset_ << ", VarOfLev = " << BDD_VarOfLev(level + offset_) << std::endl;
    zbdd = z0 + z1.Change(BDD_VarOfLev(level + offset_));
  }
};

#endif  // GRAPHILLION_INDUCING_DECOLORING_EVAL_H_
