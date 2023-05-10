/********************************************
 * BDD+ Manipulator (SAPPORO-1.93) - Header *
 * (C) Shin-ichi MINATO  (Dec. 6, 2021)     *
 ********************************************/

class BDD;
class BDDV;

#ifndef _BDD_
#define _BDD_

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <iostream>

#define BDD_CPP
#include "bddc.h"

//--------- Definition of "bddword" type --------
#ifdef B_64
  typedef unsigned long long bddword;
#else
  typedef unsigned int bddword;
#endif

//--------- External data for BDD ---------
extern const bddword BDD_MaxNode;
extern const int BDD_MaxVar;

//----- External constant data for BDDV ---------
extern int BDDV_Active;
extern const int BDDV_SysVarTop;
extern const int BDDV_MaxLen;
extern const int BDDV_MaxLenImport;

//--------- Stack overflow limitter ---------
#define BDD_RECUR_INC \
  {if(++BDD_RecurCount >= BDD_RecurLimit) \
  BDDerr("BDD_RECUR_INC:Stack overflow ", (bddword) BDD_RecurCount);}
#define BDD_RECUR_DEC BDD_RecurCount--

class BDD
{
  bddword _bdd;

public:
  BDD(void) { _bdd = bddfalse; }
  BDD(int a) { _bdd = (a==0)? bddfalse:(a>0)? bddtrue:bddnull; }
  BDD(const BDD& f) { _bdd = bddcopy(f._bdd); }

  ~BDD(void) { bddfree(_bdd); }

  BDD& operator=(const BDD& f) { 
    if(_bdd != f._bdd) { bddfree(_bdd); _bdd = bddcopy(f._bdd); }
    return *this; 
  }

  BDD& operator&=(const BDD& f)
    { BDD h; h._bdd = bddand(_bdd, f._bdd); return *this = h; }
  BDD& operator|=(const BDD& f)
    { BDD h; h._bdd = bddor(_bdd, f._bdd); return *this = h; }
  BDD& operator^=(const BDD& f)
    { BDD h; h._bdd = bddxor(_bdd, f._bdd); return *this = h; }
  BDD& operator<<=(const int s)
    { BDD h; h._bdd = bddlshift(_bdd, s); return *this = h; }
  BDD& operator>>=(const int s)
    { BDD h; h._bdd = bddrshift(_bdd, s); return *this = h; }

  BDD operator~(void) const { BDD h; h._bdd = bddnot(_bdd); return h; }
  BDD operator<<(int s) const
    { BDD h; h._bdd = bddlshift(_bdd, s); return h; }
  BDD operator>>(int s) const
    { BDD h; h._bdd = bddrshift(_bdd, s); return h; }

  int Top(void) const { return bddtop(_bdd); }
  BDD At0(int v) const { BDD h; h._bdd = bddat0(_bdd, v); return h; }
  BDD At1(int v) const { BDD h; h._bdd = bddat1(_bdd, v); return h; }
  BDD Cofact(const BDD& f) const
    { BDD h; h._bdd = bddcofactor(_bdd, f._bdd); return h; }
  BDD Univ(const BDD& f) const
    { BDD h; h._bdd = bdduniv(_bdd, f._bdd); return h; }
  BDD Exist(const BDD& f) const
    { BDD h; h._bdd = bddexist(_bdd, f._bdd); return h; }
  BDD Support(void) const
    { BDD h; h._bdd = bddsupport(_bdd); return h; }

  bddword GetID(void) const {return _bdd; }
   
  bddword Size(void) const;
  void Export(FILE *strm = stdout) const;
  void Print(void) const;
  void XPrint0(void) const;
  void XPrint(void) const;

  BDD Swap(const int&, const int&) const;
  BDD Smooth(const int&) const;
  BDD Spread(const int&) const;

  friend BDD BDD_ID(bddword);
};

//--------- External functions for BDD ---------
extern int     BDD_Init(bddword init=256, bddword limit=BDD_MaxNode);
extern int     BDD_NewVarOfLev(int);
extern int     BDD_VarUsed(void);
extern bddword BDD_Used(void);
extern void    BDD_GC(void);
extern BDD BDD_Import(FILE *strm = stdin);
extern BDD BDD_Random(int, int density = 50);
extern void BDDerr(const char *);
extern void BDDerr(const char *, bddword);
extern void BDDerr(const char *, const char *);

//--------- Inline functions for BDD ---------
inline int BDD_TopLev(void)
  { return BDDV_Active? bddvarused() - BDDV_SysVarTop: bddvarused(); }

inline int BDD_NewVar(void)
  { return bddnewvaroflev(BDD_TopLev() + 1); }

inline int BDD_LevOfVar(int v) { return bddlevofvar(v); }
inline int BDD_VarOfLev(int lev) { return bddvaroflev(lev); }

inline BDD BDD_ID(bddword bdd)
  { BDD h; h._bdd = bdd; return h; }

inline bddword BDD_CacheInt(unsigned char op, bddword fx, bddword gx)
  { return bddrcache(op, fx, gx); }

inline BDD BDD_CacheBDD(unsigned char op, bddword fx, bddword gx)
  { return BDD_ID(bddcopy(bddrcache(op, fx, gx))); }

inline void BDD_CacheEnt(unsigned char op, bddword fx, bddword gx, bddword hx)
  { bddwcache(op, fx, gx, hx); }

inline BDD BDDvar(int v) { return BDD_ID(bddprime(v)); }

inline BDD operator&(const BDD& f, const BDD& g) 
  { return BDD_ID(bddand(f.GetID(), g.GetID())); }

inline BDD operator|(const BDD& f, const BDD& g) 
  { return BDD_ID(bddor(f.GetID(), g.GetID())); }

inline BDD operator^(const BDD& f, const BDD& g) 
  { return BDD_ID(bddxor(f.GetID(), g.GetID())); }

inline int operator==(const BDD& f, const BDD& g) 
  { return f.GetID() == g.GetID(); }

inline int operator!=(const BDD& f, const BDD& g) 
  { return f.GetID() != g.GetID(); }

inline int BDD_Imply(const BDD& f, const BDD& g) 
  { return bddimply(f.GetID(), g.GetID()); }

class BDDV
{
  BDD _bdd;
  int _len;
  int _lev;

  int GetLev(int len) const {
    int lev = 0;
    for(len--; len>0; len>>=1) lev++;
    return lev;
  }

public:
  BDDV(void) { _bdd = 0; _len = 0; _lev = 0; }

  BDDV(const BDDV& fv)
    { _bdd = fv._bdd; _len = fv._len; _lev = fv._lev; } 

  BDDV(const BDD& f) {
    int t = f.Top();
    if(t > 0 && BDD_LevOfVar(t) > BDD_TopLev())
      BDDerr("BDDV::BDDV: Invalid top var.", t);
    _bdd = f;
    _len = 1;
    _lev = 0;
  }

  BDDV(const BDD&, int len);

  ~BDDV(void) { }

  BDDV& operator=(const BDDV& fv)
    { _bdd = fv._bdd; _len = fv._len; _lev = fv._lev; return *this; } 

  BDDV& operator&=(const BDDV&);
  BDDV& operator|=(const BDDV&);
  BDDV& operator^=(const BDDV&);
  BDDV& operator<<=(int);
  BDDV& operator>>=(int);

  BDDV operator~(void) const
    { BDDV h; h._bdd = ~_bdd; h._len = _len; h._lev = _lev; return h; } 
  BDDV operator<<(int) const;
  BDDV operator>>(int) const;

  BDDV At0(int v) const {
    if(v > 0 && BDD_LevOfVar(v) > BDD_TopLev())
      BDDerr("BDDV::At0: Invalid var.", v);
    BDDV hv;
    if((hv._bdd = _bdd.At0(v)) == -1) return BDDV(-1);
    hv._len = _len;
    hv._lev = _lev;
    return hv;
  }

  BDDV At1(int v) const {
    if(v > 0 && BDD_LevOfVar(v) > BDD_TopLev())
      BDDerr("BDDV::At1: Invalid var.", v);
    BDDV hv;
    if((hv._bdd = _bdd.At1(v)) == -1) return BDDV(-1);
    hv._len = _len;
    hv._lev = _lev;
    return hv;
  }

  BDDV Cofact(const BDDV&) const;
  BDDV Swap(int, int) const;
  BDDV Spread(int) const;

  int Top(void) const;

  bddword Size() const;
  void Export(FILE *strm = stdout) const;

  BDDV Former(void) const {
    BDDV hv;
    if(_len <= 1) return hv;
    if((hv._bdd = _bdd.At0(_lev)) == -1) return BDDV(-1);
    hv._len = 1 << (_lev - 1);
    hv._lev = _lev - 1;
    return hv;
  }

  BDDV Latter(void) const {
    BDDV hv;
    if(_len == 0) return hv;
    if(_len == 1) return *this;
    if((hv._bdd = _bdd.At1(_lev)) == -1) return BDDV(-1);
    hv._len = _len - (1 << (_lev - 1));
    hv._lev = GetLev(hv._len);
    return hv;
  }

  BDDV Part(int, int) const;
  BDD GetBDD(int) const;

  BDD GetMetaBDD(void) const { return _bdd; }
  int Uniform(void) const
    { return BDD_LevOfVar(_bdd.Top()) <= BDD_TopLev(); }
  int Len(void) const { return _len; }

  void Print() const;
  void XPrint0() const;
  void XPrint() const;

  friend BDDV operator&(const BDDV&, const BDDV&);
  friend BDDV operator|(const BDDV&, const BDDV&);
  friend BDDV operator^(const BDDV&, const BDDV&);
  friend BDDV operator||(const BDDV&, const BDDV&);
};

//----- External functions for BDDV ---------
extern int     BDDV_Init(bddword init=256, bddword limit=BDD_MaxNode);
extern int     BDDV_NewVarOfLev(int);
extern BDDV operator||(const BDDV&, const BDDV&);
extern BDDV BDDV_Mask1(int, int);
extern BDDV BDDV_Mask2(int, int);
extern BDDV BDDV_Import(FILE *strm = stdin);
extern BDDV BDDV_ImportPla(FILE *strm = stdin, int sopf = 0);

//----- Inline functions for BDDV ---------
inline int BDDV_UserTopLev(void) { return BDD_TopLev(); }
inline int BDDV_NewVar(void) { return BDD_NewVar(); }
inline int BDDV_NewVarOfLev(int lev) {return BDD_NewVarOfLev(lev); }

inline BDDV operator&(const BDDV& fv, const BDDV& gv) {
  BDDV hv;
  if((hv._bdd = fv._bdd & gv._bdd) == -1) return BDDV(-1);
  if(fv._len != gv._len) BDDerr("BDDV::operator&: Length mismatch");
  hv._len = fv._len;
  hv._lev = fv._lev;
  return hv;
}

inline BDDV operator|(const BDDV& fv, const BDDV& gv) {
  BDDV hv;
  if((hv._bdd = fv._bdd | gv._bdd) == -1) return BDDV(-1);
  if(fv._len != gv._len) BDDerr("BDDV::operator|: Length mismatch");
  hv._len = fv._len;
  hv._lev = fv._lev;
  return hv;
}

inline BDDV operator^(const BDDV& fv, const BDDV& gv) {
  BDDV hv;
  if((hv._bdd = fv._bdd ^ gv._bdd) == -1) return BDDV(-1);
  if(fv._len != gv._len) BDDerr("BDDV::operator^: Length mismatch");
  hv._len = fv._len;
  hv._lev = fv._lev;
  return hv;
}

extern BDDV operator|(const BDDV&, const BDDV&);
extern BDDV operator^(const BDDV&, const BDDV&);
inline int operator==(const BDDV& fv, const BDDV& gv)

  { return fv.GetMetaBDD() == gv.GetMetaBDD() && fv.Len() == gv.Len(); }

inline int operator!=(const BDDV& fv, const BDDV& gv)
  { return !(fv == gv); }

inline int BDDV_Imply(const BDDV& fv, const BDDV& gv) {
  return fv.Len() == gv.Len() 
    && BDD_Imply(fv.GetMetaBDD(), gv.GetMetaBDD());
}

inline BDDV& BDDV::operator&=(const BDDV& fv) { return *this = *this & fv; }
inline BDDV& BDDV::operator|=(const BDDV& fv) { return *this = *this | fv; }
inline BDDV& BDDV::operator^=(const BDDV& fv) { return *this = *this ^ fv; }
inline BDDV& BDDV::operator<<=(int s) { return *this = *this << s; }
inline BDDV& BDDV::operator>>=(int s) { return *this = *this >> s; }


class BDD_Hash
{
  struct BDD_Entry
  {
    BDD _key;
    void* _ptr;
    BDD_Entry(void){ _key = -1; }
  };

  bddword _amount;
  bddword _hashSize;
  BDD_Entry* _wheel;

  BDD_Entry* GetEntry(BDD);
  void Enlarge(void);
public:
  BDD_Hash(void);
  ~BDD_Hash(void);
  void Clear(void);
  void Enter(BDD, void *);
  void* Refer(BDD);
  bddword Amount(void);
};

#endif // _BDD_ 
