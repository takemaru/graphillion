/****************************************
 * BDD Cost Table class - Body v1.97    *
 * (C) Shin-ichi MINATO (Jan. 2, 2023)  *
 ****************************************/

#include "BDDCT.h"
using namespace std;

BDDCT::BDDCT()
{
  _n = 0;
  _cost = 0;
  _label = 0;

  _casize = 0;
  _caent = 0;
  _ca = 0;

  _ca0size = 0;
  _ca0ent = 0;
  _ca0 = 0;

  _call = 0;
}

BDDCT::~BDDCT()
{
  if(_cost) delete[] _cost; 
  if(_label)
  {
    for(int i=0; i<_n; i++) if(_label[i]) delete[] _label[i];
    delete[] _label; _label = 0;
  }
  if(_ca) delete[] _ca; 
  if(_ca0) delete[] _ca0; 
}

bddcost BDDCT::Cost(const int ix) const
{
  return (ix >= _n)? bddcost_null: (ix < 0)? 1: _cost[ix];
}

char* BDDCT::Label(const int ix) const
{
  return (ix >= _n || ix < 0)? 0: _label[ix];
}

int BDDCT::SetCost(const int ix, const bddcost cost)
{
  if(ix < 0 || ix >= _n) return 1;
  _cost[ix] = cost;
  if(_caent > 0) if(CacheClear()) return 1;
  if(_ca0ent > 0) if(Cache0Clear()) return 1;
  return 0;
}

int BDDCT::SetLabel(const int ix, const char* label)
{
  if(ix < 0 || ix >= _n) return 1;
  int j;
  for(j=0; j<CT_STRLEN; j++)
  {
    _label[ix][j] = label[j];
    if(!label[j]) break;
  }
  if(j == CT_STRLEN) _label[ix][j] = 0;
  return 0;
}

int BDDCT::Alloc(const int n, const bddcost cost)
{
  if(_cost) { delete[] _cost; _cost = 0; }
  if(_label)
  {
    for(int i=0; i<_n; i++) if(_label[i]) delete[] _label[i];
    delete[] _label; _label = 0;
  }

  _n = (n < 0)? 0: n;

  if(_n > 0)
  {
    if(!(_cost = new bddcost[_n])) { Alloc(0); return 1; }
    if(!(_label = new char*[_n])) { Alloc(0); return 1; }
    for(int i=0; i<_n; i++)
    {
      _cost[i] = cost;
      _label[i] = 0;
    }
    for(int i=0; i<_n; i++)
      if((_label[i] = new char[CT_STRLEN + 1])) _label[i][0] = 0;
      else { Alloc(0); return 1; }
  }

  if(CacheClear()) return 1;
  if(Cache0Clear()) return 1;
  return 0;
}

int BDDCT::Import(FILE *fp)
{
  char s[256];
  do if(fscanf(fp, "%s", s) == EOF) return 1;
  while(s[0] == '#'); // go next word
  int n = strtol(s, NULL, 10);
  if(Alloc(n)) return 1;

  do if(fscanf(fp, "%s", s) == EOF) return 1;
  while(s[0] == '#'); // go next word
  int e = 0;
  for(int ix=0; ix<_n; ix++)
  {
    if((e = SetCost(ix, strtol(s, NULL, 10)))) break;
    if(fscanf(fp, "%s", s) == EOF) { if(ix<_n-1) e = 1; break; }
    if(s[0] == '#') 
    {
      if((e = SetLabel(ix, s+1))) break;
      do if(fscanf(fp, "%s", s) == EOF) { if(ix<_n-1) e = 1; break; }
      while(s[0] == '#'); // go next word
    }
  }
  if(e) { Alloc(0); return 1; }
  return 0;
}

int BDDCT::AllocRand(const int n, const bddcost min, const bddcost max)
{
  Alloc(n);
  bddcost m = max - min + 1;
  if(m < 0) m = 1;
  for(int ix=0; ix<_n; ix++)
    if(SetCost(ix, ((double)rand()/((double)RAND_MAX+1)) * m + min))
    {
      Alloc(0);
      return 1;
    }
  return 0;
}

void BDDCT::Export() const
{
  cout << "#n " << _n << "\n";
  for(int i=0; i<_n; i++)
  {
    cout << _cost[i];
    if(_label[i] && _label[i][0])
      cout << " #" << _label[i];
    cout << "\n";
  }
}

int BDDCT::CacheClear()
{
  if(_ca) { delete[] _ca; _ca = 0; }
  _casize = 1 << 4;
  _caent = 0;
  if(!(_ca = new CacheEntry[_casize])) return 1;
  return 0;
}

#define Hash(id) ((id)*1234567)

int BDDCT::CacheEnlarge()
{
  bddword newsize = _casize << 2;
  //cout << "enlarge: " << newsize << "\n";
  CacheEntry* newca = 0;
  if(!(newca = new CacheEntry[newsize])) return 1;
  for(bddword i=0; i<_casize; i++)
  {
    if(_ca[i]._zmap)
    {
      bddword id = _ca[i]._id;
      bddword k = Hash(id) & (newsize - 1);
      while(1)
      {
        if(!newca[k]._zmap) break;
	k++;
	k &= newsize - 1;
      }
      newca[k]._id = id;
      newca[k]._zmap = _ca[i]._zmap;
      _ca[i]._zmap = 0;
    }
  }
  delete[] _ca;
  _ca = newca;
  _casize = newsize;
  return 0;
}

ZBDD BDDCT::CacheRef(const ZBDD& f, const bddcost bound,
                      bddcost& acc_worst, bddcost& rej_best)
{
  if(!_casize) return -1;
  bddword id = f.GetID();
  bddword k = Hash(id) & (_casize - 1);
  while(1)
  {
    if(!_ca[k]._zmap) return -1; 
    if(_ca[k]._id == id)
    {
      Zmap* zm = _ca[k]._zmap;
      Zmap::iterator itr = zm->lower_bound(-bound);
      if(itr == zm->end())
      {
        --itr;
	if(itr->second != 0) return -1;
	acc_worst = bddcost_null;
	--itr;
	rej_best = -(itr->first);
	return 0;
      }
      ZBDD h = itr->second;
      if(h == -1) return -1;
      acc_worst = -(itr->first);
      if(acc_worst == -bddcost_null) acc_worst = bddcost_null;
      if(itr == zm->begin()) rej_best = bddcost_null;
      else
      {
        --itr;
        rej_best = -(itr->first);
      }
      return h;
    }
    k++;
    k &= _casize - 1;
  }
}

int BDDCT::CacheEnt(const ZBDD& f, const ZBDD& h,
                     const bddcost acc_worst, const bddcost rej_best)
{
  if(!_casize) return 1;
  if(_caent >= (_casize >> 1) && CacheEnlarge()) return 1;
  bddword id = f.GetID();
  bddword k = Hash(id) & (_casize - 1);
  while(1)
  {
    if(!_ca[k]._zmap)
    {
      _caent++;
      if(!(_ca[k]._zmap = new Zmap)) return 1;
      _ca[k]._id = id;
      break;
    }
    if(_ca[k]._id == id) break;
    k++;
    k &= _casize - 1;
  }
  Zmap* zm = _ca[k]._zmap;
  if(acc_worst != bddcost_null) (*zm)[-acc_worst] = h;
  else if(h == 0) (*zm)[bddcost_null] = 0;
  if(rej_best != bddcost_null)
     if(zm->find(-rej_best) == zm->end()) (*zm)[-rej_best] = -1;
  return 0;
}

int BDDCT::Cache0Clear()
{
  if(_ca0) { delete[] _ca0; _ca0 = 0; }
  _ca0size = 1 << 4;
  _ca0ent = 0;
  if(!(_ca0 = new Cache0Entry[_ca0size])) return 1;
  return 0;
}

#define Hash0(op, id) ((id)*1234567+(op))

int BDDCT::Cache0Enlarge()
{
  bddword newsize = _ca0size << 2;
  //cout << "enlarge: " << newsize << "\n";
  Cache0Entry* newca0 = 0;
  if(!(newca0 = new Cache0Entry[newsize])) return 1;
  for(bddword i=0; i<_ca0size; i++)
  {
    if(_ca0[i]._b != bddcost_null)
    {
      bddword id = _ca0[i]._id;
      unsigned char op = _ca0[i]._op;
      bddword k = Hash0(op, id) & (newsize - 1);
      while(1)
      {
        if(newca0[k]._b == bddcost_null) break;
	k++;
	k &= newsize - 1;
      }
      newca0[k]._op = op;
      newca0[k]._id = id;
      newca0[k]._b = _ca0[i]._b;
    }
  }
  delete[] _ca0;
  _ca0 = newca0;
  _ca0size = newsize;
  return 0;
}

bddcost BDDCT::Cache0Ref(const unsigned char op, const bddword id) const
{
  if(!_ca0size) return bddcost_null;
  bddword k = Hash0(op, id) & (_ca0size - 1);
  while(1)
  {
    if(_ca0[k]._b == bddcost_null) return bddcost_null;
    if(_ca0[k]._op == op && _ca0[k]._id == id) return _ca0[k]._b;
    k++;
    k &= _ca0size - 1;
  }
}

int BDDCT::Cache0Ent(const unsigned char op, const bddword id, const bddcost b)
{
  if(!_ca0size) return 1;
  if(_ca0ent >= (_ca0size >> 1) && Cache0Enlarge()) return 1;
  bddword k = Hash0(op, id) & (_ca0size - 1);
  while(1)
  {
    if(_ca0[k]._b == bddcost_null) { _ca0ent++; break; }
    if(_ca0[k]._op == op && _ca0[k]._id == id) break;
    k++;
    k &= _ca0size - 1;
  }
  _ca0[k]._op = op;
  _ca0[k]._id = id;
  _ca0[k]._b = b;
  return 0;
}

static BDDCT* CT;
static ZBDD CLE(const ZBDD &, const bddcost, bddcost &, bddcost &);
ZBDD CLE(const ZBDD& f, const bddcost bound,
          bddcost& acc_worst, bddcost& rej_best)
{
  CT->_call++;
  if(f == 0)
  {
    acc_worst = bddcost_null;
    rej_best = bddcost_null;
    return 0;
  }
  if(f == 1)
  {
    if(bound >= 0)
    {
      acc_worst = 0;
      rej_best = bddcost_null;
      return 1;
    }
    else
    {
      acc_worst = bddcost_null;
      rej_best = 0;
      return 0;
    }
  }
  ZBDD h;
  h =  CT->CacheRef(f, bound, acc_worst, rej_best);
  if(h != -1) return h;
  int top = f.Top();
  bddcost cost = CT->CostOfLev(BDD_LevOfVar(top));
  bddcost aw0, aw1, rb0, rb1;
  h = CLE(f.OnSet0(top), bound - cost, aw1, rb1).Change(top)
    + CLE(f.OffSet(top), bound, aw0, rb0);
  /*
  h = CLE(f.OffSet(top), bound, aw0, rb0)
    + CLE(f.OnSet0(top), bound - cost, aw1, rb1).Change(top);
  */
  if(aw1 == bddcost_null) acc_worst = aw0;
  else
  {
    aw1 += cost;
    acc_worst = (aw0 == bddcost_null)? aw1: (aw0 > aw1)? aw0: aw1;
  }
  if(rb1 == bddcost_null) rej_best = rb0;
  else
  {
    rb1 += cost;
    rej_best = (rb0 == bddcost_null)? rb1: (rb0 < rb1)? rb0: rb1;
  }
  CT->CacheEnt(f, h, acc_worst, rej_best);
  /*
  if(h == 0) CT->CacheEnt(f, h, bddcost_null, bound+1);
  else if(h == f) CT->CacheEnt(f, h, bound, bddcost_null);
  else CT->CacheEnt(f, h, bound, bound+1);
  */
  //CT->CacheEnt(f, h, bound, bound+1);
  return h;
}

ZBDD BDDCT::ZBDD_CostLE(const ZBDD& f, const bddcost bound,
                         bddcost& acc_worst, bddcost& rej_best)
{
  CT = this;
  _call = 0;
  ZBDD h = CLE(f, bound, acc_worst, rej_best);
  return h;
}

static bddcost MinC(const ZBDD&);
bddcost MinC(const ZBDD& f)
{
  if(f == 0) return bddcost_null;
  if(f == 1) return 0;
  bddcost min = CT->Cache0Ref(4, f.GetID());
  if(min != bddcost_null) return min;
  int top = f.Top();
  min = MinC(f.OffSet(top));
  bddcost min1 = MinC(f.OnSet0(top))
               + CT->CostOfLev(BDD_LevOfVar(top));
  min = (min != bddcost_null && min < min1)? min: min1;
  CT->Cache0Ent(4, f.GetID(), min);
  return min;
}

bddcost BDDCT::MinCost(const ZBDD& f)
{
  CT = this;
  return MinC(f);
}

static bddcost MaxC(const ZBDD&);
bddcost MaxC(const ZBDD& f)
{
  if(f == 0) return bddcost_null;
  if(f == 1) return 0;
  bddcost max = CT->Cache0Ref(5, f.GetID());
  if(max != bddcost_null) return max;
  int top = f.Top();
  max = MaxC(f.OffSet(top));
  bddcost max1 = MaxC(f.OnSet0(top))
               + CT->CostOfLev(BDD_LevOfVar(top));
  max = (max != bddcost_null && max > max1)? max: max1;
  CT->Cache0Ent(5, f.GetID(), max);
  return max;
}

bddcost BDDCT::MaxCost(const ZBDD& f)
{
  CT = this;
  return MaxC(f);
}

static bddcost B;
static bddcost RetMin;
static bddcost RetMax;
static ZBDD CLE0(const ZBDD &, const bddcost);
ZBDD CLE0(const ZBDD& f, const bddcost spent)
{
  if(f == 0)
  {
    RetMin = bddcost_null; RetMax = bddcost_null;
    return 0;
  }
  if(f == 1)
  {
    RetMin = 0; RetMax = 0;
    return (B >= spent)? 1: 0;
  }
  bddcost min = CT->Cache0Ref(4, f.GetID());
  bddcost max = CT->Cache0Ref(5, f.GetID());
  RetMin = min; RetMax = max;
  if(min != bddcost_null) if(B < min + spent) return 0;
  if(max != bddcost_null) if(B >= max + spent) return f;
  int top = f.Top();
  int tlev = BDD_LevOfVar(top);
  ZBDD h = CLE0(f.OffSet(top), spent);
  int min0 = RetMin;
  int max0 = RetMax;
  bddcost cost = CT->CostOfLev(tlev);
  h += CLE0(f.OnSet0(top), spent + cost).Change(top);
  if(min == bddcost_null)
  {
    min = RetMin + cost;
    if(min0 != bddcost_null) min = (min0 <= min)? min0: min;
    CT->Cache0Ent(4, f.GetID(), min);
  }
  if(max == bddcost_null)
  {
    max = RetMax + cost;
    if(max0 != bddcost_null) max = (max0 >= max)? max0: max;
    CT->Cache0Ent(5, f.GetID(), max);
  }
  RetMin = min; RetMax = max;
  return h;
}

ZBDD BDDCT::ZBDD_CostLE0(const ZBDD& f, const bddcost bound)
{
  CT = this;
  B = bound;
  ZBDD h = CLE0(f, 0);
  return h;
}

