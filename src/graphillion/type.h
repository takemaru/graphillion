#ifndef GRAPHILLION_TYPE_H_
#define GRAPHILLION_TYPE_H_

#include <stdint.h>

#include "hudd/ZBDD.h"

#define nullptr (NULL)

namespace graphillion {

typedef ZBDD zdd_t;
typedef bddword word_t;
typedef int32_t elem_t;  // bddvar

}  // namespace graphillion

#endif  // GRAPHILLION_TYPE_H_
