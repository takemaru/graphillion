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
/*********************************************
 * ZBDD+ Manipulator (SAPPORO-1.58) - Header *
 * (C) Shin-ichi MINATO  (Nov. 22, 2013)     *
 *********************************************/

class ZBDD;
class ZBDDV;

#ifndef _ZBDD_
#define _ZBDD_

#include "SAPPOROBDD/BDD.h"

class SeqBDD;

class ZBDD
{
  bddword _zbdd;

public:
  ZBDD(void) { _zbdd = bddempty; }
  ZBDD(int v) { _zbdd = (v==0)? bddempty:(v>0)? bddsingle:bddnull; }
  ZBDD(const ZBDD& f) { _zbdd = bddcopy(f._zbdd); }

  ~ZBDD(void) { bddfree(_zbdd); }

  ZBDD& operator=(const ZBDD& f) { 
    if(_zbdd != f._zbdd) { bddfree(_zbdd); _zbdd = bddcopy(f._zbdd); } 
    return *this;
  }

  ZBDD& operator&=(const ZBDD& f)
    { ZBDD h; h._zbdd = bddintersec(_zbdd, f._zbdd); return *this = h; }

  ZBDD& operator+=(const ZBDD& f)
    { ZBDD h; h._zbdd = bddunion(_zbdd, f._zbdd); return *this = h; }

  ZBDD& operator-=(const ZBDD& f)
    { ZBDD h; h._zbdd = bddsubtract(_zbdd, f._zbdd); return *this = h; }

  ZBDD& operator<<=(int s)
    { ZBDD h; h._zbdd = bddlshift(_zbdd, s); return *this = h; }

  ZBDD& operator>>=(int s)
    { ZBDD h; h._zbdd = bddrshift(_zbdd, s); return *this = h; }

  ZBDD& operator*=(const ZBDD&);
  ZBDD& operator/=(const ZBDD&);
  ZBDD& operator%=(const ZBDD&);

  ZBDD operator<<(int s) const
    { ZBDD h; h._zbdd = bddlshift(_zbdd, s); return h; }

  ZBDD operator>>(int s) const
    { ZBDD h; h._zbdd = bddrshift(_zbdd, s); return h; }

  int Top(void) const { return bddtop(_zbdd); }

  ZBDD OffSet(int v) const
    { ZBDD h; h._zbdd = bddoffset(_zbdd, v); return h; }

  ZBDD OnSet(int v) const
    { ZBDD h; h._zbdd = bddonset(_zbdd, v); return h; }

  ZBDD OnSet0(int v) const
    { ZBDD h; h._zbdd = bddonset0(_zbdd, v); return h; }

  ZBDD Change(int v) const
    { ZBDD h; h._zbdd = bddchange(_zbdd, v); return h; }

  bddword GetID(void) const { return _zbdd; }
  bddword Size(void) const { return bddsize(_zbdd); }
  bddword Card(void) const { return bddcard(_zbdd); }
  bddword Lit(void) const { return bddlit(_zbdd); }
  bddword Len(void) const { return bddlen(_zbdd); }

  void Export(FILE *strm = stdout) const;
  void XPrint(void) const;
  void Print(void) const;
  void PrintPla(void) const;

  ZBDD Swap(int, int) const;
  ZBDD Restrict(const ZBDD&) const;
  ZBDD Permit(const ZBDD&) const;
  ZBDD PermitSym(int) const;
  ZBDD Support(void) const
    { ZBDD h; h._zbdd = bddsupport(_zbdd); return h; }
  ZBDD Always(void) const;

  int SymChk(int, int) const;
  ZBDD SymGrp(void) const;
  ZBDD SymGrpNaive(void) const;

  ZBDD SymSet(int) const;
  int ImplyChk(int, int) const;
  int CoImplyChk(int, int) const;
  ZBDD ImplySet(int) const;
  ZBDD CoImplySet(int) const;

  int IsPoly(void) const;
  ZBDD Divisor(void) const;

  ZBDD ZLev(int lev, int last = 0) const;
  void SetZSkip(void) const;
  ZBDD Intersec(const ZBDD&) const;

  friend ZBDD ZBDD_ID(bddword);

  //friend class SeqBDD;
};

extern ZBDD operator*(const ZBDD&, const ZBDD&);
extern ZBDD operator/(const ZBDD&, const ZBDD&);
extern ZBDD ZBDD_Meet(const ZBDD&, const ZBDD&);
extern ZBDD ZBDD_Random(int, int density = 50);
extern ZBDD ZBDD_Import(FILE *strm = stdin);

extern ZBDD ZBDD_LCM_A(char *, int);
extern ZBDD ZBDD_LCM_C(char *, int);
extern ZBDD ZBDD_LCM_M(char *, int);

inline ZBDD ZBDD_ID(bddword zbdd)
  { ZBDD h; h._zbdd = zbdd; return h; }

inline ZBDD BDD_CacheZBDD(char op, bddword fx, bddword gx)
  { return ZBDD_ID(bddcopy(bddrcache(op, fx, gx))); }

inline ZBDD operator&(const ZBDD& f, const ZBDD& g)
  { return ZBDD_ID(bddintersec(f.GetID(), g.GetID())); }

inline ZBDD operator+(const ZBDD& f, const ZBDD& g)
  { return ZBDD_ID(bddunion(f.GetID(), g.GetID())); }

inline ZBDD operator-(const ZBDD& f, const ZBDD& g)
  { return ZBDD_ID(bddsubtract(f.GetID(), g.GetID())); }

inline ZBDD operator%(const ZBDD& f, const ZBDD& p)
  { return f - (f/p) * p; }

inline int operator==(const ZBDD& f, const ZBDD& g)
  { return f.GetID() == g.GetID(); }

inline int operator!=(const ZBDD& f, const ZBDD& g)
  { return !(f == g); }

inline ZBDD& ZBDD::operator*=(const ZBDD& f)
  { return *this = *this * f; }

inline ZBDD& ZBDD::operator/=(const ZBDD& f)
  { return *this = *this / f; }

inline ZBDD& ZBDD::operator%=(const ZBDD& f)
  { return *this = *this % f; }


class ZBDDV
{
  ZBDD _zbdd;

public:
  ZBDDV(void) { _zbdd = 0; }
  ZBDDV(const ZBDDV& fv) { _zbdd = fv._zbdd; }
  ZBDDV(const ZBDD& f, int location = 0);
  ~ZBDDV(void) { }

  ZBDDV& operator=(const ZBDDV& fv) { _zbdd = fv._zbdd; return *this; }
  ZBDDV& operator&=(const ZBDDV& fv) { _zbdd &= fv._zbdd; return *this; }
  ZBDDV& operator+=(const ZBDDV& fv) { _zbdd += fv._zbdd; return *this; }
  ZBDDV& operator-=(const ZBDDV& fv) { _zbdd -= fv._zbdd; return *this; }
  ZBDDV& operator<<=(int);
  ZBDDV& operator>>=(int);

  ZBDDV operator<<(int) const;
  ZBDDV operator>>(int) const;

  ZBDDV OffSet(int) const;
  ZBDDV OnSet(int) const;
  ZBDDV OnSet0(int) const;
  ZBDDV Change(int) const;
  ZBDDV Swap(int, int) const;
  
  int Top(void) const;
  int Last(void) const;
  ZBDDV Mask(int start, int length = 1) const;
  ZBDD GetZBDD(int) const;

  ZBDD GetMetaZBDD(void) const { return _zbdd; }
  bddword Size(void) const;
  void Print(void) const;
  void Export(FILE *strm = stdout) const;
  int PrintPla(void) const;
  void XPrint(void) const;
	
  friend ZBDDV operator&(const ZBDDV&, const ZBDDV&);
  friend ZBDDV operator+(const ZBDDV&, const ZBDDV&);
  friend ZBDDV operator-(const ZBDDV&, const ZBDDV&);
};

extern ZBDDV ZBDDV_Import(FILE *strm = stdin);

inline ZBDDV operator&(const ZBDDV& fv, const ZBDDV& gv)
  { ZBDDV hv; hv._zbdd = fv._zbdd & gv._zbdd; return hv; }
inline ZBDDV operator+(const ZBDDV& fv, const ZBDDV& gv)
  { ZBDDV hv; hv._zbdd = fv._zbdd + gv._zbdd; return hv; }
inline ZBDDV operator-(const ZBDDV& fv, const ZBDDV& gv)
  { ZBDDV hv; hv._zbdd = fv._zbdd - gv._zbdd; return hv; }
inline int operator==(const ZBDDV& fv, const ZBDDV& gv)
  {  return fv.GetMetaZBDD() == gv.GetMetaZBDD(); }
inline int operator!=(const ZBDDV& fv, const ZBDDV& gv)
  {  return !(fv == gv); }

inline ZBDDV& ZBDDV::operator<<=(int s)
  { return *this = *this << s; }

inline ZBDDV& ZBDDV::operator>>=(int s)
  { return *this = *this >> s; }

class ZBDD_Hash;
class ZBDD_Hash
{
  struct ZBDD_Entry
  {
    ZBDD _key;
    void* _ptr;
    ZBDD_Entry(void){ _key = -1; }
  };

  bddword _amount;
  bddword _hashSize;
  ZBDD_Entry* _wheel;
  
  ZBDD_Entry* GetEntry(ZBDD);
  void Enlarge(void);
public:
  ZBDD_Hash(void);
  ~ZBDD_Hash(void);
  void Clear(void);
  void Enter(ZBDD, void *);
  void* Refer(ZBDD);
  bddword Amount(void);
};

#endif // _ZBDD_
