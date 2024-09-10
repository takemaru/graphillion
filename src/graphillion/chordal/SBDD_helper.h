#ifndef GRAPHILLION_SBDD_HELPER_H_
#define GRAPHILLION_SBDD_HELPER_H_

#include <cassert>
#include <vector>

#include "SAPPOROBDD/ZBDD.h"

namespace sbddh {
// inline function qualifier for gcc
// if a compile error occurs, change the qualifier
#define sbddextended_INLINE_FUNC static inline

sbddextended_INLINE_FUNC void sbddextended_sort_array(bddvar* arr, int n) {
  int i, j;
  bddvar temp;

  for (i = n - 1; i >= 1; --i) {
    for (j = 0; j < i; ++j) {
      if (arr[j] > arr[j + 1]) {
        temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}

// must free the returned pointer
sbddextended_INLINE_FUNC bddvar* sbddextended_getsortedarraybylevel_inner(
    const bddvar* vararr, int n) {
  int i;
  bddvar* ar;

  ar = (bddvar*)malloc((size_t)n * sizeof(bddvar));
  if (ar == NULL) {
    fprintf(stderr, "out of memory\n");
    return NULL;
  }

  // translate varIDs to levels
  for (i = 0; i < n; ++i) {
    ar[i] = bddlevofvar(vararr[i]);
  }

  sbddextended_sort_array(ar, n);

  return ar;
}

sbddextended_INLINE_FUNC bddp bddgetpowerset(const bddvar* vararr, int n) {
  int i;
  bddp f, g, h;
  bddvar* ar;

  ar = sbddextended_getsortedarraybylevel_inner(vararr, n);
  if (ar == NULL) {
    return bddnull;
  }

  f = bddsingle;
  for (i = 0; i < n; ++i) {
    assert(1 <= ar[i] && ar[i] <= bddvarused());
    g = bddchange(f, ar[i]);
    h = bddunion(f, g);
    bddfree(g);
    bddfree(f);
    f = h;
  }
  free(ar);
  return f;
}

sbddextended_INLINE_FUNC ZBDD getPowerSet(const std::vector<bddvar>& vararr) {
  if (vararr.empty()) {
    return ZBDD(1);
  }

  int n = static_cast<int>(vararr.size());

  bddvar* ar = new bddvar[n];
  if (ar == NULL) {
    fprintf(stderr, "out of memory\n");
    return false;
  }

  for (int i = 0; i < n; ++i) {
    ar[i] = vararr[i];
  }

  bddp f = bddgetpowerset(ar, n);

  delete[] ar;

  return ZBDD_ID(f);
}
}  // end of namespace sbddh

#endif  // GRAPHILLION_SBDD_HELPER_H_
