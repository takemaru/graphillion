/*********************************************************************
Copyright 2013  JST ERATO Minato project and other contributors
http://www-erato.ist.hokudai.ac.jp/?language=en

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**********************************************************************/

#include <cassert>

#include "graphillion/reconf.h"

namespace reconf {

typedef unsigned long long int ullint;

#define BC_REMOVEE_CPP  29
#define BC_ADDE_CPP     30
#define BC_SWAPE_CPP    31

ZBDD removeSomeElement(const ZBDD& f)
{
  if (f == ZBDD(0) || f == ZBDD(1)) {
    return ZBDD(0);
  }

  bddword fx = f.GetID();

  ZBDD h = BDD_CacheZBDD(BC_REMOVEE_CPP, fx, static_cast<bddp>(bddempty));
  if(h != -1) {
    return h;
  }
  BDD_RECUR_INC;

  ZBDD f0 = f.OffSet(f.Top());
  ZBDD f1 = f.OnSet0(f.Top());

  h = removeSomeElement(f0);
  ZBDD r0 = h + f1;
  ZBDD r1 = removeSomeElement(f1);
  h = r0 + r1.Change(f.Top());

  BDD_RECUR_DEC;
  if(h != -1) {
    BDD_CacheEnt(BC_REMOVEE_CPP, fx, static_cast<bddp>(bddempty), h.GetID());
  }

  return h;
}


ZBDD addSomeElement(const ZBDD& f, int n, int lower)
{
  ZBDD f0, f1, r0, r1;
  
  if (f == ZBDD(0)) {
    return ZBDD(0);
  }

  int flev;

  if (f == ZBDD(1)) {
    flev = 0;
  } else {
    flev = BDD_LevOfVar(f.Top());
  }

  assert(flev <= n);

  if (n == 0 || n < lower) {
    assert(f == ZBDD(1));
    return ZBDD(0);
  }

  bddword fx = f.GetID();
  ZBDD h = BDD_CacheZBDD(BC_ADDE_CPP, fx, static_cast<bddword>((n << 16) + lower));
  if(h != -1) {
    return h;
  }
  BDD_RECUR_INC;

  if (flev == n) {
    f0 = f.OffSet(f.Top());
    f1 = f.OnSet0(f.Top());

    r0 = addSomeElement(f0, n - 1, lower);
    h = addSomeElement(f1, n - 1, lower);
    r1 = f0 + h;
  } else {
    assert(flev < n);
    r0 = addSomeElement(f, n - 1, lower);
    r1 = f;
  }
  h = r0 + r1.Change(BDD_VarOfLev(n));

  BDD_RECUR_DEC;
  if(h != -1) {
    BDD_CacheEnt(BC_ADDE_CPP, fx, static_cast<bddword>((n << 16) + lower), h.GetID());
  }

  return h;
}


ZBDD removeAddSomeElements(const ZBDD& f, int n, int lower)
{
  if (f == ZBDD(0) || f == ZBDD(1) || n < lower) {
    return ZBDD(0);
  }

  int flev = BDD_LevOfVar(f.Top());

  assert(flev <= n);

  bddword fx = f.GetID();
  ZBDD h = BDD_CacheZBDD(BC_SWAPE_CPP, fx, static_cast<bddword>((n << 16) + lower));
  if(h != -1) {
    return h;
  }
  BDD_RECUR_INC;

  ZBDD r0, r1;
  if (flev == n) {
    ZBDD f0 = f.OffSet(f.Top());
    ZBDD f1 = f.OnSet0(f.Top());

    h = removeAddSomeElements(f0, n - 1, lower);
    ZBDD h2 = addSomeElement(f1, n - 1, lower);
    r0 = h + h2;
    h = removeAddSomeElements(f1, n - 1, lower);
    h2 = removeSomeElement(f0);
    r1 = h + h2;
  } else {
    assert(flev < n);
    r0 = removeAddSomeElements(f, n - 1, lower);
    r1 = removeSomeElement(f);
  }
  h = r0 + r1.Change(BDD_VarOfLev(n));

  BDD_RECUR_DEC;
  if(h != -1) {
    BDD_CacheEnt(BC_SWAPE_CPP, fx, static_cast<bddword>((n << 16) + lower), h.GetID());
  }

  return h;
}

} // namespace reconf
