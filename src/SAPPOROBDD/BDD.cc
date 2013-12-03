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
 /***************************************
 * BDD+ Manipulator (SAPPORO-1.58)      *
 * (Basic methods)                      *
 * (C) Shin-ichi MINATO (Nov. 22, 2013) *
 ****************************************/

#include "SAPPOROBDD/BDD.h"

using std::cout;
using std::cerr;

static const char BC_Smooth = 60;
static const char BC_Spread = 61;

extern "C"
{
  int rand();
};


//--- SBDD class for default initialization ----

int BDDV_Active = 0;

class SBDD
{
public:
  SBDD(bddword init, bddword limit) { bddinit(init, limit); }
};
static SBDD BDD_Manager((bddword)256, (bddword)1024);


//----- External constant data for BDD -------

const bddword BDD_MaxNode = B_VAL_MASK >> 1U;
const int BDD_MaxVar = bddvarmax;

//-------------- class BDD --------------------

bddword BDD::Size() const { return bddsize(_bdd); }

void BDD::Export(FILE *strm) const 
{
  bddword p = _bdd;
  bddexport(strm, &p, 1);
}

void BDD::Print() const
{
  cout << "[ " << GetID();
  cout << " Var:" << Top() << "(" << BDD_LevOfVar(Top()) << ")";
  cout << " Size:" << Size() << " ]\n";
  cout.flush();
}

BDD BDD::Swap(const int& v1, const int& v2) const
{
  if(v1 == v2) return *this;
  BDD x = BDDvar(v1);
  BDD y = BDDvar(v2);
  BDD fx0 = At0(v1);
  BDD fx1 = At1(v1);
  return x & ( ~y & fx0.At1(v2) | y & fx1.At1(v2) ) |
        ~x & ( ~y & fx0.At0(v2) | y & fx1.At0(v2) );
}

#define BDD_CACHE_CHK_RETURN(op, fx, gx) \
  { BDD h = BDD_CacheBDD(op, fx, gx); \
    if(h != -1) return h; \
    BDD_RECUR_INC; }

#define BDD_CACHE_ENT_RETURN(op, fx, gx, h) \
  { BDD_RECUR_DEC; \
    if(h != -1) BDD_CacheEnt(op, fx, gx, h.GetID()); \
    return h; }

BDD BDD::Smooth(const int& v) const
{
  int t = Top();
  if(t == 0) return *this;
  if(BDD_LevOfVar(t) <= BDD_LevOfVar(v)) return 1;
  bddword fx = GetID();
  bddword gx = BDDvar(v).GetID();
  BDD_CACHE_CHK_RETURN(BC_Smooth, fx, gx);
  BDD x = BDDvar(t);
  BDD h = (~x & At0(t).Smooth(v))|(x & At1(t).Smooth(v));
  BDD_CACHE_ENT_RETURN(BC_Smooth, fx, gx, h);
}

BDD BDD::Spread(const int& k) const
{
  int t = Top();
  if(t == 0) return *this;
  if(k == 0) return *this;
  if(k < 0) BDDerr("BDD::Spread: k < 0.",k);
  bddword fx = GetID();
  bddword gx = BDDvar(k).GetID();
  BDD_CACHE_CHK_RETURN(BC_Spread, fx, gx);
  BDD x = BDDvar(t);
  BDD f0 = At0(t);
  BDD f1 = At1(t);
  BDD h = (~x & f0.Spread(k)) | (x & f1.Spread(k))
    | (~x & f1.Spread(k-1)) | (x & f0.Spread(k-1));
  BDD_CACHE_ENT_RETURN(BC_Spread, fx, gx, h);
}

//----- External functions for BDD -------

void BDD_Init(bddword init, bddword limit)
{
  bddinit(init, limit);
  BDDV_Active = 0;
}
	
int BDD_NewVarOfLev(int lev)
{
  if(lev > BDD_TopLev() + 1)
    BDDerr("BDD_NewVarOfLev:Invald lev ", (bddword)lev);
  return bddnewvaroflev(lev);
}

int BDD_VarUsed(void) { return bddvarused(); }

bddword BDD_Used(void) { return bddused(); }

void BDD_GC() { bddgc(); }

BDD BDD_Import(FILE *strm)
{
	bddword bdd;
	if(bddimport(strm, &bdd, 1)) return -1;
	return BDD_ID(bdd);
}

BDD BDD_Random(int level, int density)
{
  if(level < 0)
    BDDerr("BDD_Random: level < 0.",level);
  if(level == 0) return ((rand()%100) < density)? 1: 0;
  return (BDDvar(BDD_VarOfLev(level))
        & BDD_Random(level-1, density)) |
         (~BDDvar(BDD_VarOfLev(level))
        & BDD_Random(level-1, density));
}

void BDDerr(const char* msg)
{
  cerr << "<ERROR> " << msg << " \n";
  exit(1);
}

void BDDerr(const char* msg, bddword key)
{
  cerr << "<ERROR> " << msg << " (" << key << ")\n";
  exit(1);
}

void BDDerr(const char* msg, const char* name)
{
  cerr << "<ERROR> " << msg << " (" << name << ")\n";
  exit(1);
}


//----- External constant data for BDDV -------

const int BDDV_SysVarTop = 20;
const int BDDV_MaxLen = 1 << BDDV_SysVarTop;
const int BDDV_MaxLenImport = 1000;


//--------------- class BDDV ------------------------

BDDV::BDDV(const BDD& f, int len)
{
  if(len < 0) BDDerr("BDDV::BDDV: len < 0.", len);
  if(len > BDDV_MaxLen) BDDerr("BDDV::BDDV: Too large len.", len);
  int t = f.Top();
  if(t > 0 && BDD_LevOfVar(t) > BDD_TopLev())
    BDDerr("BDDV::BDDV: Invalid Top Var.", t);
  _bdd = (len == 0)? 0: f;
  _len = (f == -1)? 1: len;
  _lev = GetLev(len);
}

BDDV BDDV::operator<<(int shift) const
{
  if(!Uniform()) return (Former() << shift) || (Latter() << shift);
  BDDV hv;
  if((hv._bdd = _bdd << shift) == -1) return BDDV(-1);
  hv._len = _len;
  hv._lev = _lev;
  return hv;
}

BDDV BDDV::operator>>(int shift) const
{
  if(!Uniform()) return (Former() >> shift) || (Latter() >> shift);
  BDDV hv;
  if((hv._bdd = _bdd >> shift) == -1) return BDDV(-1);
  hv._len = _len;
  hv._lev = _lev;
  return hv;
}

BDDV BDDV::Cofact(const BDDV& fv) const
{
  if(_lev > 0)
    return Former().Cofact(fv.Former()) || Latter().Cofact(fv.Latter());
  BDDV hv;
  if((hv._bdd = _bdd.Cofact(fv._bdd)) == -1) return BDDV(-1);
  if(_len != fv._len) BDDerr("BDDV::Cofact: Length mismatch.");
  hv._len = _len;
  // hv._lev = _lev; (always zero)
  return hv;
}

BDDV BDDV::Swap(int v1, int v2) const
{
  if(BDD_LevOfVar(v1) > BDD_TopLev())
    BDDerr("BDDV::Swap: Invalid VarID.", v1);
  if(BDD_LevOfVar(v2) > BDD_TopLev())
    BDDerr("BDDV::Swap: Invalid VarID.", v2);
  BDDV hv;
  if((hv._bdd = _bdd.Swap(v1, v2)) == -1) return BDDV(-1);
  hv._len = _len;
  hv._lev = _lev;
  return hv;
}

int BDDV::Top() const
{
  if(Uniform()) return _bdd.Top();
  int t0 = Former().Top();
  int t1 = Latter().Top();
  if(BDD_LevOfVar(t0) > BDD_LevOfVar(t1)) return t0;
  else return t1;
}

bddword BDDV::Size() const
{
  bddword* bddv = new bddword[_len];
  for(int i=0; i<_len; i++) bddv[i] = GetBDD(i).GetID(); 
  bddword s = bddvsize(bddv, _len);
  delete[] bddv;
  return s;
}

void BDDV::Export(FILE *strm) const
{
  bddword* bddv = new bddword[_len];
  for(int i=0; i<_len; i++) bddv[i] = GetBDD(i).GetID(); 
  bddexport(strm, bddv, _len);
  delete[] bddv;
}

BDDV BDDV::Spread(int k) const
{
  if(Uniform()) return _bdd.Spread(k);
  return Former().Spread(k) || Latter().Spread(k);
}

BDDV BDDV::Part(int start, int len) const
{
  if(_bdd == -1) return *this;
  if(len == 0) return BDDV();

  if(start < 0 || start + len  > _len)
    BDDerr("BDDV::Part: Illegal index.");
  
  if(start == 0 && len == _len) return *this;
  
  int half = 1 << (_lev-1);
  
  if(start + len <= half) return Former().Part(start, len);
  if(start >= half) return Latter().Part(start - half, len);
  return Former().Part(start, half - start)
      || Latter().Part(0, start + len - half);
}

BDD BDDV::GetBDD(int index) const
{
  if(index < 0 || index >= _len)
    BDDerr("BDDV::GetBDD: Illegal index.",index);
  if(_len == 1) return _bdd;
  BDD f = _bdd;
  for(int i=_lev-1; i>=0; i--)
    if((index & (1<<i)) == 0) f = f.At0(i + 1);
    else f = f.At1(i + 1);
  return f;
}

void BDDV::Print() const
{
  for(int i=0; i<_len; i++)
  {
    cout << "f" << i << ": ";
    GetBDD(i).Print();
  }
  cout << "Size= " << Size() << "\n\n";
  cout.flush();
}

//----- External functions for BDD Vector -------

void BDDV_Init(bddword init, bddword limit)
{
  bddinit(init, limit);
  for(int i=0; i<BDDV_SysVarTop; i++) bddnewvar();
  BDDV_Active = 1;
}
	
BDDV operator||(const BDDV& fv, const BDDV& gv)
{
  if(fv._len == 0) return gv;
  if(gv._len == 0) return fv;
  if(fv._len != (1 << fv._lev))
    return BDDV(fv).Former() || (BDDV(fv).Latter() || gv);
  if(fv._len < gv._len)
    return (fv || BDDV(gv).Former()) || BDDV(gv).Latter();
  BDDV hv;
  BDD x = BDDvar(fv._lev + 1);
  if((hv._bdd = (~x & fv._bdd)|(x & gv._bdd)) == -1) return BDDV(-1);
  if((hv._len = fv._len + gv._len) > BDDV_MaxLen)
    BDDerr("BDDV::operatop||: Too large len.", hv._len);
  hv._lev = fv._lev + 1;
  return hv;
}

BDDV BDDV_Mask1(int index, int len)
{
  if(len < 0) BDDerr("BDDV_Mask1: len < 0.", len);
  if(index < 0 || index >= len)
    BDDerr("BDDV_Mask1: Illegal index.", index);
  return BDDV(0,index)||BDDV(1,1)||BDDV(0,len-index-1);
}

BDDV BDDV_Mask2(int index, int len)
{
  if(len < 0) BDDerr("BDDV_Mask2: len < 0.", len);
  if(index < 0 || index > len)
    BDDerr("BDDV_Mask2: Illegal index.", index);
  return BDDV(0,index)||BDDV(1,len-index);
}

#define IMPORTHASH(x) (((x >> 1) ^ (x >> 16)) & (hashsize - 1))

#ifdef B_64
#  define B_STRTOI strtoll
#else
#  define B_STRTOI strtol
#endif

BDDV BDDV_Import(FILE *strm)
{
  int inv, e;
  bddword hashsize;
  BDD f, f0, f1;
  char s[256];
  bddword *hash1;
  BDD *hash2;

  if(fscanf(strm, "%s", &s) == EOF) return BDDV(-1);
  if(strcmp(s, "_i") != 0) return BDDV(-1);
  if(fscanf(strm, "%s", &s) == EOF) return BDDV(-1);
  int n = strtol(s, NULL, 10);
  while(n > BDD_TopLev()) BDD_NewVar();

  if(fscanf(strm, "%s", &s) == EOF) return BDDV(-1);
  if(strcmp(s, "_o") != 0) return BDDV(-1);
  if(fscanf(strm, "%s", &s) == EOF) return BDDV(-1);
  int m = strtol(s, NULL, 10);

  if(fscanf(strm, "%s", &s) == EOF) return BDDV(-1);
  if(strcmp(s, "_n") != 0) return BDDV(-1);
  if(fscanf(strm, "%s", &s) == EOF) return BDDV(-1);
  bddword n_nd = B_STRTOI(s, NULL, 10);

  for(hashsize = 1; hashsize < (n_nd<<1); hashsize <<= 1)
    ; /* empty */
  hash1 = new bddword[hashsize];
  if(hash1 == 0) return BDDV(-1);
  hash2 = new BDD[hashsize];
  if(hash2 == 0) { delete[] hash1; return BDDV(-1); }
  for(bddword ix=0; ix<hashsize; ix++)
  {
    hash1[ix] = B_VAL_MASK;
    hash2[ix] = 0;
  }

  e = 0;
  for(bddword ix=0; ix<n_nd; ix++)
  {
    if(fscanf(strm, "%s", &s) == EOF) { e = 1; break; }
    bddword nd = B_STRTOI(s, NULL, 10);
    
    if(fscanf(strm, "%s", &s) == EOF) { e = 1; break; }
    int lev = strtol(s, NULL, 10);
    int var = bddvaroflev(lev);

    if(fscanf(strm, "%s", &s) == EOF) { e = 1; break; }
    if(strcmp(s, "F") == 0) f0 = 0;
    else if(strcmp(s, "T") == 0) f0 = 1;
    else
    {
      bddword nd0 = B_STRTOI(s, NULL, 10);

      bddword ixx = IMPORTHASH(nd0);
      while(hash1[ixx] != nd0)
      {
        if(hash1[ixx] == B_VAL_MASK)
          BDDerr("BDDV_Import(): internal error", ixx);
        ixx++;
        ixx &= (hashsize-1);
      }
      f0 = hash2[ixx];
    }

    if(fscanf(strm, "%s", &s) == EOF) { e = 1; break; }
    if(strcmp(s, "F") == 0) f1 = 0;
    else if(strcmp(s, "T") == 0) f1 = 1;
    else
    {
      bddword nd1 = B_STRTOI(s, NULL, 10);
      if(nd1 & 1) { inv = 1; nd1 ^= 1; }
      else inv = 0;
  
      bddword ixx = IMPORTHASH(nd1);
      while(hash1[ixx] != nd1)
      {
        if(hash1[ixx] == B_VAL_MASK)
          BDDerr("BDDV_Import(): internal error", ixx);
        ixx++;
        ixx &= (hashsize-1);
      }
      f1 = (inv)? ~hash2[ixx]: hash2[ixx];
    }

    BDD x = BDDvar(var);
    f = (x & f1) | (~x & f0);
    if(f == -1) { e = 1; break; }

    bddword ixx = IMPORTHASH(nd);
    while(hash1[ixx] != B_VAL_MASK)
    {
      if(hash1[ixx] == nd)
        BDDerr("BDDV_Import(): internal error", ixx);
      ixx++;
      ixx &= (hashsize-1);
    }
    hash1[ixx] = nd;
    hash2[ixx] = f;
  }

  if(e)
  {
    delete[] hash2;
    delete[] hash1;
    return BDDV(-1);
  }

  BDDV v = BDDV();
  for(int i=0; i<m; i++)
  {
    if(fscanf(strm, "%s", &s) == EOF)
    {
      delete[] hash2;
      delete[] hash1;
      return BDDV(-1);
    }
    bddword nd = B_STRTOI(s, NULL, 10);
    if(strcmp(s, "F") == 0) v = v || BDD(0);
    else if(strcmp(s, "T") == 0) v = v || BDD(1);
    else
    {
      if(nd & 1) { inv = 1; nd ^= 1; }
      else inv = 0;
  
      bddword ixx = IMPORTHASH(nd);
      while(hash1[ixx] != nd)
      {
        if(hash1[ixx] == B_VAL_MASK)
          BDDerr("BDDV_Import(): internal error", ixx);
        ixx++;
        ixx &= (hashsize-1);
      }
      v = v || (inv? ~hash2[ixx]: hash2[ixx]);
    }
  }

  delete[] hash2;
  delete[] hash1;
  return v;
}

