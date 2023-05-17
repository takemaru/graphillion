#ifndef GRAPHILLION_GRAPHILLION_BDDCT_H_
#define GRAPHILLION_GRAPHILLION_BDDCT_H_

#include "graphillion/setset.h"
#include "graphillion/type.h"
#include "SAPPOROBDD/BDDCT.h"

namespace graphillion {

setset CostLE(const std::vector<bddcost> costs, const ZBDD &zbdd);

} // namespace graphillion

#endif // GRAPHILLION_GRAPHILLION_BDDCT_H_