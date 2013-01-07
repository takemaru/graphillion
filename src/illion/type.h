#ifndef ILLION_TYPE_H_
#define ILLION_TYPE_H_

#include <cstdint>

#include "hudd/ZBDD.h"

namespace illion {

typedef ZBDD zdd_t;
typedef bddword word_t;
typedef int32_t elem_t;  // bddvar

}  // namespace illion

#endif  // ILLION_TYPE_H_
