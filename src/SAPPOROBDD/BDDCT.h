/*****************************************
 *  BDD Cost Table class - Header v1.97  *
 *  (C) Shin-ichi MINATO (Jan. 2, 2023)  *
 *****************************************/

class BDDCT;

#ifndef _BDDCT_
#define _BDDCT_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "ZBDD.h"

typedef int bddcost;
#define bddcost_null 0x7FFFFFFF
#define CT_STRLEN 15

#include <map>
typedef std::map<bddcost, ZBDD> Zmap;

class BDDCT
{
public:
  struct CacheEntry
  {
    bddword _id;
    Zmap* _zmap;
    CacheEntry(void)
    {
      _id = BDD(-1).GetID();
      _zmap = 0;
    }
    ~CacheEntry(void)
    {
      if(!_zmap) delete _zmap;
    }
  };

  struct Cache0Entry
  {
    bddword _id;
    bddcost _b;
    unsigned char _op;
    Cache0Entry(void)
    {
      _id = BDD(-1).GetID();
      _b = bddcost_null;
      _op = 255;
    }
    ~Cache0Entry(void) { }
  };

  int _n;
  bddcost *_cost;
  char **_label;

  bddword _casize;
  bddword _caent;
  CacheEntry* _ca;

  bddword _ca0size;
  bddword _ca0ent;
  Cache0Entry* _ca0;
  
  bddword _call;

  BDDCT(void);
  ~BDDCT(void);

  inline int Size(void) const { return _n; }

  bddcost Cost(const int ix) const;
  inline bddcost CostOfLev(const int lev) const 
  { return Cost(_n-lev); }
  char* Label(const int) const;
  inline char* LabelOfLev(const int lev) const 
  { return Label(_n-lev); }

  int SetCost(const int, const bddcost);
  inline int SetCostOfLev(const int lev, const bddcost cost) 
  { return SetCost(_n-lev, cost); }
  int SetLabel(const int, const char *);
  inline int SetLabelOfLev(const int lev, const char* label)
  { return SetLabel(_n-lev, label); }

  int Alloc(const int n, const bddcost cost = 1);
  int Import(FILE* fp = stdin);
  int AllocRand(const int, const bddcost, const bddcost);
  void Export(void) const;

  int CacheClear(void);
  int CacheEnlarge(void);
  ZBDD CacheRef(const ZBDD &, const bddcost, bddcost &, bddcost &);
  int CacheEnt(const ZBDD &, const ZBDD &, const bddcost, const bddcost);

  int Cache0Clear(void);
  int Cache0Enlarge(void);
  bddcost Cache0Ref(const unsigned char, const bddword) const;
  int Cache0Ent(const unsigned char, const bddword, const bddcost);

  ZBDD ZBDD_CostLE(const ZBDD& f, const bddcost bound)
  { bddcost aw, rb; return ZBDD_CostLE(f, bound, aw, rb); }
  ZBDD ZBDD_CostLE(const ZBDD &, const bddcost, bddcost &, bddcost &);

  ZBDD ZBDD_CostLE0(const ZBDD &, const bddcost);
  bddcost MinCost(const ZBDD &);
  bddcost MaxCost(const ZBDD &);
};

#endif // _BDDCT_

