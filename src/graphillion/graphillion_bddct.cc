#include "graphillion_bddct.h"

namespace graphillion {

setset CostLE(const std::vector<bddcost> costs, const zdd_t &zbdd) {
  return setset(zbdd);
}

} // namespace graphillion