/****************************************
 * ZBDD+ Manipulator (SAPPORO-1.87)     *
 * (Main part)                          *
 * (C) Shin-ichi MINATO (May 14, 2021)  *
 ****************************************/

#include "ZBDD.h"

#define BDD_CPP
#include "bddc.h"

using std::cout;

static const char BC_ZBDD_MULT = 20;
static const char BC_ZBDD_DIV = 21;
static const char BC_ZBDD_RSTR = 22;
static const char BC_ZBDD_PERMIT = 23;
static const char BC_ZBDD_PERMITSYM = 24;
static const char BC_ZBDD_SYMCHK = 25;
static const char BC_ZBDD_ALWAYS = 26;
static const char BC_ZBDD_SYMSET = 27;
static const char BC_ZBDD_COIMPSET = 28;
static const char BC_ZBDD_MEET = 29;

static const char BC_ZBDD_ZSkip = 65;
static const char BC_ZBDD_INTERSEC = 66;

extern "C"
{
	int rand();
};

// class ZBDD ---------------------------------------------

void ZBDD::Export(FILE *strm) const
{
  bddword p = _zbdd;
  bddexport(strm, &p, 1);
}

void ZBDD::Print() const
{
  cout << "[ " << GetID();
  cout << " Var:" << Top() << "(" << BDD_LevOfVar(Top()) << ")";
  cout << " Size:" << Size() << " Card:";
  cout << Card() << " Lit:" << Lit() << " Len:" << Len() << " ]\n";
  cout.flush();
}

void ZBDD::PrintPla() const { ZBDDV(*this).PrintPla(); }

#define ZBDD_CACHE_CHK_RETURN(op, fx, gx) \
  { ZBDD h = BDD_CacheZBDD(op, fx, gx); \
    if(h != -1) return h; \
    BDD_RECUR_INC; }

#define ZBDD_CACHE_ENT_RETURN(op, fx, gx, h) \
  { BDD_RECUR_DEC; \
    if(h != -1) BDD_CacheEnt(op, fx, gx, h.GetID()); \
    return h; }

ZBDD ZBDD::Swap(int v1, int v2) const
{
  if(v1 == v2) return *this;
  ZBDD f00 = this->OffSet(v1).OffSet(v2);
  ZBDD f11 = this->OnSet(v1).OnSet(v2);
  ZBDD h = *this - f00 - f11;
  return h.Change(v1).Change(v2) + f00 + f11;
}

ZBDD ZBDD::Restrict(const ZBDD& g) const
{
  if(*this == -1) return -1;
  if(g == -1) return -1;
  if(*this == 0) return 0;
  if(g == 0) return 0;
  if(*this == g) return g;
  if((g & 1) == 1) return *this;
  ZBDD f = *this - 1;

  int top = f.Top();
  if(BDD_LevOfVar(top) < BDD_LevOfVar(g.Top())) top = g.Top();

  bddword fx = f.GetID();
  bddword gx = g.GetID();
  ZBDD_CACHE_CHK_RETURN(BC_ZBDD_RSTR, fx, gx);

  ZBDD f1 = f.OnSet0(top);
  ZBDD f0 = f.OffSet(top);
  ZBDD g1 = g.OnSet0(top);
  ZBDD g0 = g.OffSet(top);
  ZBDD h = f1.Restrict(g1 + g0).Change(top) + f0.Restrict(g0);

  ZBDD_CACHE_ENT_RETURN(BC_ZBDD_RSTR, fx, gx, h);
}

ZBDD ZBDD::Permit(const ZBDD& g) const
{
  if(*this == -1) return -1;
  if(g == -1) return -1;
  if(*this == 0) return 0;
  if(g == 0) return 0;
  if(*this == g) return *this;
  if(g == 1) return *this & 1;
  if(*this == 1) return 1;

  int top = Top();
  if(BDD_LevOfVar(top) < BDD_LevOfVar(g.Top())) top = g.Top();

  bddword fx = GetID();
  bddword gx = g.GetID();
  ZBDD_CACHE_CHK_RETURN(BC_ZBDD_PERMIT, fx, gx);

  ZBDD f1 = OnSet0(top);
  ZBDD f0 = OffSet(top);
  ZBDD g1 = g.OnSet0(top);
  ZBDD g0 = g.OffSet(top);
  ZBDD h = f1.Permit(g1).Change(top) + f0.Permit(g0 + g1);

  ZBDD_CACHE_ENT_RETURN(BC_ZBDD_PERMIT, fx, gx, h);
}

ZBDD ZBDD::PermitSym(int n) const
{
  if(*this == -1) return -1;
  if(*this == 0) return 0;
  if(*this == 1) return 1;
  if(n < 1) return *this & 1;

  int top = Top();

  bddword fx = GetID();
  ZBDD_CACHE_CHK_RETURN(BC_ZBDD_PERMITSYM, fx, n);

  ZBDD f1 = OnSet0(top);
  ZBDD f0 = OffSet(top);
  ZBDD h = f1.PermitSym(n - 1).Change(top) + f0.PermitSym(n);

  ZBDD_CACHE_ENT_RETURN(BC_ZBDD_PERMITSYM, fx, n, h);
}

ZBDD ZBDD::Always() const
{
  if(*this == -1) return -1;
  if(*this == 0 || *this == 1) return 0;

  bddword fx = GetID();
  ZBDD_CACHE_CHK_RETURN(BC_ZBDD_ALWAYS, fx, 0);

  int t = Top();
  ZBDD f1 = OnSet0(t);
  ZBDD f0 = OffSet(t);
  ZBDD h = f1.Always();
  if(f0 == 0) h += ZBDD(1).Change(t);
  else if(h != 0) h &= f0.Always();

  ZBDD_CACHE_ENT_RETURN(BC_ZBDD_ALWAYS, fx, 0, h);
}

int ZBDD::SymChk(int v1, int v2) const
{
  if(*this == -1) return -1;
  if(v1 <= 0) BDDerr("ZBDD::SymChk(): invalid v1.", v1);
  if(v2 <= 0) BDDerr("ZBDD::SymChk(): invalid v2.", v2);
  if(*this == 0 || *this == 1) return 1;
  if(v1 == v2) return 1;
  if(v1 < v2) { int tmp = v1; v1 = v2; v2 = tmp; }

  ZBDD S = ZBDD(1).Change(v1) + ZBDD(1).Change(v2);
  bddword fx = GetID();
  bddword gx = S.GetID();
  int Y = BDD_CacheInt(BC_ZBDD_SYMCHK, fx, gx);
  if(Y != -1) return Y;
  BDD_RECUR_INC;

  int t = Top();
  if(BDD_LevOfVar(t) > BDD_LevOfVar(v1))
  {
    Y = OnSet0(t).SymChk(v1, v2);
    if(Y == 1) Y = OffSet(t).SymChk(v1, v2);
  }
  else
  {
    ZBDD f0 = OffSet(v1);
    ZBDD f1 = OnSet0(v1);
    int t0 = f0.Top();
    int t1 = f1.Top();
    int t2 = (BDD_LevOfVar(t0) > BDD_LevOfVar(t1))? t0: t1;
    if(BDD_LevOfVar(t2) <= BDD_LevOfVar(v2))
      Y = (f0.OnSet0(v2) == f1.OffSet(v2));
    else
    {
      ZBDD g0 = f0.OffSet(t2) + f1.OffSet(t2).Change(t2);
      ZBDD g1 = f0.OnSet0(t2) + f1.OnSet0(t2).Change(t2);
      Y = g1.SymChk(t2, v2);
      if(Y == 1) Y = g0.SymChk(t2, v2);
    }
  }

  BDD_RECUR_DEC;
  if(Y != -1) BDD_CacheEnt(BC_ZBDD_SYMCHK, fx, gx, Y);
  return Y;
}

ZBDD ZBDD::SymGrp() const
{
  ZBDD h = 0;
  ZBDD g = Support();
  while(g != 0)
  {
    int t = g.Top();
    ZBDD hh = ZBDD(1).Change(t);
    g = g.OffSet(t);

    ZBDD g2 = g;
    while(g2 != 0)
    {
      int t2 = g2.Top();
      g2 = g2.OffSet(t2);
      int y = SymChk(t, t2);
      if(y == -1) return -1;
      if(y)
      {
	hh = hh.Change(t2);
	g = g.OffSet(t2);
      }
    }
    if(hh.OnSet0(t) != 1) h += hh;
  }
  return h;
}

ZBDD ZBDD::SymGrpNaive() const
{
  ZBDD h = 0;
  ZBDD g = Support();
  while(g != 0)
  {
    int t = g.Top();
    ZBDD hh = ZBDD(1).Change(t);
    g = g.OffSet(t);
    ZBDD f0 = OffSet(t);
    ZBDD f1 = OnSet0(t);

    ZBDD g2 = g;
    while(g2 != 0)
    {
      int t2 = g2.Top();
      g2 = g2.OffSet(t2);
      if(f0.OnSet0(t2) == f1.OffSet(t2))
      {
	hh = hh.Change(t2);
	g = g.OffSet(t2);
      }
    }
    h += hh;
  }
  return h;
}

static ZBDD ZBDD_SymSet(const ZBDD&, const ZBDD&);
static ZBDD ZBDD_SymSet(const ZBDD& f0, const ZBDD& f1)
{
  if(f0 == -1) return -1;
  if(f1 == -1) return -1;
  if(f1 == 0) return 0;
  if(f1 == 1 && (f0 == 0 || f0 == 1)) return 0;

  bddword fx = f0.GetID();
  bddword gx = f1.GetID();
  ZBDD_CACHE_CHK_RETURN(BC_ZBDD_SYMSET, fx, gx);

  int t0 = f0.Top();
  int t1 = f1.Top();
  int t = (BDD_LevOfVar(t0) > BDD_LevOfVar(t1))? t0: t1;

  ZBDD f00 = f0.OffSet(t);
  ZBDD f01 = f0.OnSet0(t);
  ZBDD f10 = f1.OffSet(t);
  ZBDD f11 = f1.OnSet0(t);
  
  ZBDD h;
  if(f11 == 0) h = ZBDD_SymSet(f00, f10) - f01.Support();
  else if(f10 == 0) h = ZBDD_SymSet(f01, f11) - f00.Support();
  else
  {
    h = ZBDD_SymSet(f01, f11);
    if(h != 0) h &= ZBDD_SymSet(f00, f10);
  }
  if(f10 == f01) h += ZBDD(1).Change(t);

  ZBDD_CACHE_ENT_RETURN(BC_ZBDD_SYMSET, fx, gx, h);
}

ZBDD ZBDD::SymSet(int v) const
{
  if(*this == -1) return -1;
  if(v <= 0) BDDerr("ZBDD::SymSet(): invalid v.", v);
  ZBDD f0 = OffSet(v);
  ZBDD f1 = OnSet0(v);
  return ZBDD_SymSet(f0, f1);
}

int ZBDD::ImplyChk(int v1, int v2) const
{
  if(*this == -1) return -1;
  if(v1 <= 0) BDDerr("ZBDD::IndImplyChk(): invalid v1.", v1);
  if(v2 <= 0) BDDerr("ZBDD::IndImplyChk(): invalid v2.", v2);
  if(v1 == v2) return 1;
  if(*this == 0 || *this == 1) return 1;

  ZBDD f10 = OnSet0(v1).OffSet(v2);
  if(f10 == -1) return -1;
  return (f10 == 0);
}

ZBDD ZBDD::ImplySet(int v) const
{
  if(*this == -1) return -1;
  if(v <= 0) BDDerr("ZBDD::ImplySet(): invalid v.", v);
  ZBDD f1 = OnSet0(v);
  if(f1 == 0) return Support();
  return f1.Always();
}

int ZBDD::CoImplyChk(int v1, int v2) const
{
  if(*this == -1) return -1;
  if(v1 <= 0) BDDerr("ZBDD::IndImplyChk(): invalid v1.", v1);
  if(v2 <= 0) BDDerr("ZBDD::IndImplyChk(): invalid v2.", v2);
  if(v1 == v2) return 1;
  if(*this == 0 || *this == 1) return 1;

  ZBDD f10 = OnSet0(v1).OffSet(v2);
  if(f10 == 0) return 1;

  ZBDD f01 = OffSet(v1).OnSet0(v2);
  ZBDD chk = f10 - f01;
  if(chk == -1) return -1;
  return (chk == 0) ;
}

static ZBDD ZBDD_CoImplySet(const ZBDD&, const ZBDD&);
static ZBDD ZBDD_CoImplySet(const ZBDD& f0, const ZBDD& f1)
{
  if(f0 == -1) return -1;
  if(f1 == -1) return -1;
  if(f1 == 0) return 0;
  if(f1 == 1 && (f0 == 0 || f0 == 1)) return 0;

  bddword fx = f0.GetID();
  bddword gx = f1.GetID();
  ZBDD_CACHE_CHK_RETURN(BC_ZBDD_COIMPSET, fx, gx);

  int t0 = f0.Top();
  int t1 = f1.Top();
  int t = (BDD_LevOfVar(t0) > BDD_LevOfVar(t1))? t0: t1;

  ZBDD f00 = f0.OffSet(t);
  ZBDD f01 = f0.OnSet0(t);
  ZBDD f10 = f1.OffSet(t);
  ZBDD f11 = f1.OnSet0(t);
  
  ZBDD h;
  if(f11 == 0) h = ZBDD_CoImplySet(f00, f10);
  else if(f10 == 0) h = ZBDD_CoImplySet(f01, f11);
  else
  {
    h = ZBDD_CoImplySet(f01, f11);
    if(h != 0) h &= ZBDD_CoImplySet(f00, f10);
  }
  if(f10 - f01 == 0) h += ZBDD(1).Change(t);

  ZBDD_CACHE_ENT_RETURN(BC_ZBDD_COIMPSET, fx, gx, h);
}

ZBDD ZBDD::CoImplySet(int v) const
{
  if(*this == -1) return -1;
  if(v <= 0) BDDerr("ZBDD::CoImplySet(): invalid v.", v);
  ZBDD f0 = OffSet(v);
  ZBDD f1 = OnSet0(v);
  if(f1 == 0) return Support();
  return ZBDD_CoImplySet(f0, f1);
}

int ZBDD::IsPoly() const
{
  int top = Top();
  if(top == 0) return 0;
  ZBDD f1 = OnSet0(top);
  ZBDD f0 = OffSet(top);
  if(f0 != 0) return 1;
  return f1.IsPoly();
}

ZBDD ZBDD::Divisor() const
{
  if(*this == -1) return -1;
  if(*this == 0) return 0;
  if(! IsPoly()) return 1;
  ZBDD f = *this;
  ZBDD g = Support();
  int t;
  while(g != 0)
  {
    t = g.Top();
    g = g.OffSet(t);
    ZBDD f1 = f.OnSet0(t);
    if(f1.IsPoly()) f = f1;
  }
  return f;
}


//--------- External functions for ZBDD ------------

ZBDD operator*(const ZBDD& fc, const ZBDD& gc)
{
  if(fc == -1) return -1;
  if(gc == -1) return -1;
  if(fc == 0) return 0;
  if(gc == 0) return 0;
  if(fc == 1) return gc;
  if(gc == 1) return fc;

  ZBDD f = fc; ZBDD g = gc;
  int ftop = f.Top(); int gtop = g.Top();
  if(BDD_LevOfVar(ftop) < BDD_LevOfVar(gtop))
  {
    f = gc; g = fc;
    ftop = f.Top(); gtop = g.Top();
  }

  bddword fx = f.GetID();
  bddword gx = g.GetID();
  if(ftop == gtop && fx < gx)
  {
    f = gc; g = fc;
    fx = f.GetID(); gx = g.GetID();
  }

  ZBDD_CACHE_CHK_RETURN(BC_ZBDD_MULT, fx, gx);

  ZBDD f1 = f.OnSet0(ftop);
  ZBDD f0 = f.OffSet(ftop);
  ZBDD h;
  if(ftop != gtop)
  {
    h = f1 * g;
    h = h.Change(ftop) + (f0 * g);
  }
  else
  {
    ZBDD g1 = g.OnSet0(ftop);
    ZBDD g0 = g.OffSet(ftop);
    h = (f1 * g1)+(f1 * g0)+(f0 * g1);
    h = h.Change(ftop) + (f0 * g0);
  }

  ZBDD_CACHE_ENT_RETURN(BC_ZBDD_MULT, fx, gx, h);
}

ZBDD operator/(const ZBDD& f, const ZBDD& p)
{
  if(f == -1) return -1;
  if(p == -1) return -1;
  if(p == 1) return f;
  if(f == p) return 1;
  if(p == 0) BDDerr("operator /(): Divided by zero.");
  int top = p.Top();
  if(BDD_LevOfVar(f.Top()) < BDD_LevOfVar(top)) return 0;

  bddword fx = f.GetID();
  bddword px = p.GetID();
  ZBDD_CACHE_CHK_RETURN(BC_ZBDD_DIV, fx, px);
  
  ZBDD q = f.OnSet0(top) / p.OnSet0(top);
  if(q != 0)
  {
    ZBDD p0 = p.OffSet(top);
    if(p0 != 0) q &= f.OffSet(top) / p0;
  }

  ZBDD_CACHE_ENT_RETURN(BC_ZBDD_DIV, fx, px, q);
}

ZBDD ZBDD_Meet(const ZBDD& fc, const ZBDD& gc)
{
  if(fc == -1) return -1;
  if(gc == -1) return -1;
  if(fc == 0) return 0;
  if(gc == 0) return 0;
  if(fc == 1) return 1;
  if(gc == 1) return 1;

  ZBDD f = fc; ZBDD g = gc;
  int ftop = f.Top();
  int gtop = g.Top();
  if(BDD_LevOfVar(ftop) < BDD_LevOfVar(gtop))
  {
    f = gc; g = fc;
    ftop = f.Top(); gtop = g.Top();
  }

  bddword fx = f.GetID();
  bddword gx = g.GetID();
  if(ftop == gtop && fx < gx)
  {
    f = gc; g = fc;
    fx = f.GetID(); gx = g.GetID();
  }

  ZBDD_CACHE_CHK_RETURN(BC_ZBDD_MEET, fx, gx);

  ZBDD f1 = f.OnSet0(ftop);
  ZBDD f0 = f.OffSet(ftop);
  ZBDD h;
  if(ftop != gtop)
  {
    h = ZBDD_Meet(f0, g) + ZBDD_Meet(f1, g);
  }
  else
  {
    ZBDD g1 = g.OnSet0(ftop);
    ZBDD g0 = g.OffSet(ftop);
    h = ZBDD_Meet(f1, g1);
    h = h.Change(ftop) + ZBDD_Meet(f0, g0)
      + ZBDD_Meet(f1, g0) + ZBDD_Meet(f0, g1);
  }

  ZBDD_CACHE_ENT_RETURN(BC_ZBDD_MEET, fx, gx, h);
}

ZBDD ZBDD_Random(int lev, int density)
{
  if(lev < 0) BDDerr("ZBDD_Random(): lev < 0.", lev);
  if(lev == 0) return ((rand()%100) < density)? 1: 0;
  return ZBDD_Random(lev-1, density) +
         ZBDD_Random(lev-1, density).Change(BDD_VarOfLev(lev));
}

ZBDD ZBDD_Import(FILE *strm)
{
  bddword zbdd;
  if(bddimportz(strm, &zbdd, 1)) return -1;
  return ZBDD_ID(zbdd);
}


// class ZBDDV ---------------------------------------------

ZBDDV::ZBDDV(const ZBDD& f, int location)
{
  if(location < 0) BDDerr("ZBDDV::ZBDDV(): location < 0.", location);
  if(location >= BDDV_MaxLen)
    BDDerr("ZBDDV::ZBDDV(): Too large location.", location);
  if(BDD_LevOfVar(f.Top()) > BDD_TopLev())
    BDDerr("ZBDDV::ZBDDV(): Invalid top var.", f.Top());
  _zbdd = f;
  int var = 1;
  for(int i=location; i>0; i>>=1)
  {
    if((i & 1)!= 0) _zbdd = _zbdd.Change(var);
    var++;
  }
}

ZBDDV ZBDDV::operator<<(int shift) const
{
  ZBDDV fv1 = *this;
  ZBDDV fv2;
  while(fv1 != ZBDDV())
  {
    if(fv1 == ZBDDV(-1)) return fv1;
    int last = fv1.Last();
    fv2 += ZBDDV(fv1.GetZBDD(last) << shift, last);
    fv1 -= fv1.Mask(last);
  }
  return fv2;
}

ZBDDV ZBDDV::operator>>(int shift) const
{
  ZBDDV fv1 = *this;
  ZBDDV fv2;
  while(fv1 != ZBDDV())
  {
    if(fv1 == ZBDDV(-1)) return fv1;
    int last = fv1.Last();
    fv2 += ZBDDV(fv1.GetZBDD(last) >> shift, last);
    fv1 -= fv1.Mask(last);
  }
  return fv2;
}

ZBDDV ZBDDV::OffSet(int v) const
{
  if(BDD_LevOfVar(v) > BDD_TopLev())
    BDDerr("ZBDDV::OffSet(): Invalid VarID.", v);
  ZBDDV tmp;
  tmp._zbdd = _zbdd.OffSet(v);
  return tmp;
}

ZBDDV ZBDDV::OnSet(int v) const
{
  if(BDD_LevOfVar(v) > BDD_TopLev())
    BDDerr("ZBDDV::OnSet(): Invalid VarID.", v);
  ZBDDV tmp;
  tmp._zbdd = _zbdd.OnSet(v);
  return tmp;
}

ZBDDV ZBDDV::OnSet0(int v) const
{
  if(BDD_LevOfVar(v) > BDD_TopLev())
    BDDerr("ZBDDV::OnSet0(): Invalid VarID.", v);
  ZBDDV tmp;
  tmp._zbdd = _zbdd.OnSet0(v);
  return tmp;
}

ZBDDV ZBDDV::Change(int v) const
{
  if(BDD_LevOfVar(v) > BDD_TopLev())
    BDDerr("ZBDDV::Change(): Invalid VarID.", v);
  ZBDDV tmp;
  tmp._zbdd = _zbdd.Change(v);
  return tmp;
}

ZBDDV ZBDDV::Swap(int v1, int v2) const
{
  if(BDD_LevOfVar(v1) > BDD_TopLev())
    BDDerr("ZBDDV::Swap(): Invalid VarID.", v1);
  if(BDD_LevOfVar(v1) > BDD_TopLev())
    BDDerr("ZBDDV::Swap(): Invalid VarID.", v2);
  ZBDDV tmp;
  tmp._zbdd = _zbdd.Swap(v1, v2);
  return tmp;
}

int ZBDDV::Top() const
{
  ZBDDV fv1 = *this;
  if(fv1 == ZBDDV(-1)) return 0;
  int top = 0;
  while(fv1 != ZBDDV())
  {
    int last = fv1.Last();
    int t = fv1.GetZBDD(last).Top();
    if(BDD_LevOfVar(t) > BDD_LevOfVar(top)) top = t;
    fv1 -= fv1.Mask(last);
  }
  return top;
}

int ZBDDV::Last() const
{
  int last = 0;
  ZBDD f = _zbdd;
  while(BDD_LevOfVar(f.Top()) > BDD_TopLev())
  {
    int t = f.Top();
    last += 1 << (t - 1);
    f = f.OnSet0(t);
  }
  return last;
}

ZBDDV ZBDDV::Mask(int start, int len) const
{
  if(start < 0 || start >= BDDV_MaxLen)
    BDDerr("ZBDDV::Mask(): Illegal start index.", start);
  if(len <= 0 || start+len > BDDV_MaxLen)
    BDDerr("ZBDDV::Mask(): Illegal len.", len);
  ZBDDV tmp;
  for(int i=start; i<start+len; i++)
  	tmp += ZBDDV(this -> GetZBDD(i), i);
  return tmp;
}

ZBDD ZBDDV::GetZBDD(int index) const
{
  if(index < 0 || index >= BDDV_MaxLen)
    BDDerr("ZBDDV::GetZBDD(): Illegal index.",index);
  int level = 0;
  for(int i=1; i<=index; i<<=1) level++;

  ZBDD f = _zbdd;
  while(BDD_LevOfVar(f.Top()) > BDD_TopLev() + level)
    f = f.OffSet(f.Top());
  while(level > 0)
  {
    if(f == 0) return f;
    if((index & (1<<(level-1))) != 0) f = f.OnSet0(level);
    else f = f.OffSet(level);
    level--;
  }
  return f;
}

bddword ZBDDV::Size() const
{
  int len = this -> Last() + 1;
  bddword* bddv = new bddword[len];
  for(int i=0; i<len; i++) bddv[i] = GetZBDD(i).GetID(); 
  bddword s = bddvsize(bddv, len);
  delete[] bddv;
  return s;
}

void ZBDDV::Print() const
{
  int len = this -> Last() + 1;
  for(int i=0; i<len; i++)
  {
    cout << "f" << i << ": ";
    GetZBDD(i).Print();
  }
  cout << "Size= " << Size() << "\n\n";
  cout.flush();
}

void ZBDDV::Export(FILE *strm) const
{
  int len = this -> Last() + 1;
  bddword* bddv = new bddword[len];
  for(int i=0; i<len; i++) bddv[i] = GetZBDD(i).GetID(); 
  bddexport(strm, bddv, len);
  delete[] bddv;
}

static int Len;
static char* Cube;
static int ZBDDV_PLA(const ZBDDV&, int);
static int ZBDDV_PLA(const ZBDDV& fv, int tlev)
{
  if(fv == ZBDDV(-1)) return 1;
  if(fv == ZBDDV()) return 0;
  if(tlev == 0)
  {
    cout << Cube << " ";
    for(int i=0; i<Len; i++)
      if(fv.GetZBDD(i) == 0) cout << "~";
      else cout << "1";
    cout << "\n";
    cout.flush();
    return 0;
  }
  Cube[tlev-1] = '1';
  if(ZBDDV_PLA(fv.OnSet0(BDD_VarOfLev(tlev)), tlev-1) == 1)
    return 1;
  Cube[tlev-1] = '0';
  return ZBDDV_PLA(fv.OffSet(BDD_VarOfLev(tlev)), tlev-1);
}

int ZBDDV::PrintPla() const
{
  if(*this == ZBDDV(-1)) return 1;
  int tlev = BDD_LevOfVar(Top());
  Len = Last() + 1;
  cout << ".i " << tlev << "\n";
  cout << ".o " << Len << "\n";
  if(tlev == 0)
  {
    for(int i=0; i<Len; i++)
    if(GetZBDD(i) == 0) cout << "0";
    else cout << "1";
    cout << "\n";
  }
  else
  {
    Cube = new char[tlev + 1];
    Cube[tlev] = 0;
    int err = ZBDDV_PLA(*this, tlev);
    delete[] Cube;
    if(err == 1) return 1;
  }
  cout << ".e\n";
  cout.flush();
  return 0;
}

#define IMPORTHASH(x) ((((x)>>1)^((x)<<8)^((x)<<16)) & (hashsize-1))

#ifdef B_64
#  define B_STRTOI strtoll
#else
#  define B_STRTOI strtol
#endif

ZBDDV ZBDDV_Import(FILE *strm)
{
  int inv, e;
  bddword hashsize;
  ZBDD f, f0, f1;
  char s[256];
  bddword *hash1 = 0;
  ZBDD *hash2 = 0;

  if(fscanf(strm, "%s", s) == EOF) return ZBDDV(-1);
  if(strcmp(s, "_i") != 0) return ZBDDV(-1);
  if(fscanf(strm, "%s", s) == EOF) return ZBDDV(-1);
  int n = strtol(s, NULL, 10);
  while(n > BDD_TopLev()) BDD_NewVar();

  if(fscanf(strm, "%s", s) == EOF) return ZBDDV(-1);
  if(strcmp(s, "_o") != 0) return ZBDDV(-1);
  if(fscanf(strm, "%s", s) == EOF) return ZBDDV(-1);
  int m = strtol(s, NULL, 10);

  if(fscanf(strm, "%s", s) == EOF) return ZBDDV(-1);
  if(strcmp(s, "_n") != 0) return ZBDDV(-1);
  if(fscanf(strm, "%s", s) == EOF) return ZBDDV(-1);
  bddword n_nd = B_STRTOI(s, NULL, 10);

  for(hashsize = 1; hashsize < (n_nd<<1); hashsize <<= 1)
    ; /* empty */
  hash1 = new bddword[hashsize];
  if(hash1 == 0) return ZBDDV(-1);
  hash2 = new ZBDD[hashsize];
  if(hash2 == 0) { delete[] hash1; return ZBDDV(-1); }
  for(bddword ix=0; ix<hashsize; ix++)
  {
    hash1[ix] = B_VAL_MASK;
    hash2[ix] = 0;
  }

  e = 0;
  for(bddword ix=0; ix<n_nd; ix++)
  {
    if(fscanf(strm, "%s", s) == EOF) { e = 1; break; }
    bddword nd = B_STRTOI(s, NULL, 10);
    
    if(fscanf(strm, "%s", s) == EOF) { e = 1; break; }
    int lev = strtol(s, NULL, 10);
    int var = bddvaroflev(lev);

    if(fscanf(strm, "%s", s) == EOF) { e = 1; break; }
    if(strcmp(s, "F") == 0) f0 = 0;
    else if(strcmp(s, "T") == 0) f0 = 1;
    else
    {
      bddword nd0 = B_STRTOI(s, NULL, 10);

      bddword ixx = IMPORTHASH(nd0);
      while(hash1[ixx] != nd0)
      {
        if(hash1[ixx] == B_VAL_MASK)
          BDDerr("ZBDDV_Import(): internal error", ixx);
        ixx++;
        ixx &= (hashsize-1);
      }
      f0 = hash2[ixx];
    }

    if(fscanf(strm, "%s", s) == EOF) { e = 1; break; }
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
          BDDerr("ZBDDV_Import(): internal error", ixx);
        ixx++;
        ixx &= (hashsize-1);
      }
      f1 = (inv)? (hash2[ixx] + 1): hash2[ixx];
    }

    f = f1.Change(var) + f0;
    if(f == -1) { e = 1; break; }

    bddword ixx = IMPORTHASH(nd);
    while(hash1[ixx] != B_VAL_MASK)
    {
      if(hash1[ixx] == nd)
        BDDerr("ZBDDV_Import(): internal error", ixx);
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
    return ZBDDV(-1);
  }

  ZBDDV v = ZBDDV();
  for(int i=0; i<m; i++)
  {
    if(fscanf(strm, "%s", s) == EOF)
    {
      delete[] hash2;
      delete[] hash1;
      return ZBDDV(-1);
    }
    bddword nd = B_STRTOI(s, NULL, 10);
    if(strcmp(s, "F") == 0) v += ZBDDV(0, i);
    else if(strcmp(s, "T") == 0) v += ZBDDV(1, i);
    else
    {
      if(nd & 1) { inv = 1; nd ^= 1; }
      else inv = 0;
  
      bddword ixx = IMPORTHASH(nd);
      while(hash1[ixx] != nd)
      {
        if(hash1[ixx] == B_VAL_MASK)
          BDDerr("ZBDDV_Import(): internal error", ixx);
        ixx++;
        ixx &= (hashsize-1);
      }
      v += ZBDDV((inv? (hash2[ixx] + 1): hash2[ixx]), i);
    }
  }

  delete[] hash2;
  delete[] hash1;
  return v;
}

#define ZLevNum(n) \
  (n-((n&2)?(n&1)? (n<512)?(n<64)?(n<16)?4:8:(n<128)?32:(n<256)?64:128:(n<4096)?(n<1024)?256:(n<2048)?512:1024:(n<8192)?2048:(n<32768)?4096:8192 \
  : (n<512)?(n<64)?4:(n<256)?16:32:(n<4096)?(n<1024)?64:128:(n<32768)?512:1024 \
  :(n&1)? (n<512)?(n<16)?4:8:(n<2048)?(n<1024)?16:32:(n<32768)?64:128 \
  : (n<1024)?4:(n<32768)?8:16 \
  ))

ZBDD ZBDD::ZLev(int lev, int last) const
{
  if(lev <= 0) return *this & 1;
  ZBDD f = *this;
  ZBDD u = *this & 1;
  int ftop = Top();
  int flev = BDD_LevOfVar(ftop);
  while(flev > lev)
  {
    if(flev - lev >= 5)
    {
      int n = ZLevNum(flev);
      if(flev >= 66)
      {
        if(n < lev || ((flev & 3) < 3 && ZLevNum((flev-3)) >= lev))
	  n = flev - 1;
      }
      else if(flev >= 18)
      {
        if(n < lev || ((flev & 1) < 1 && ZLevNum((flev-1)) >= lev))
	  n = flev - 1;
      }
      else if(n < lev) n = flev - 1;

      if(n < flev - 1)
      {
        bddword fx = f.GetID();
        ZBDD g = BDD_CacheZBDD(BC_ZBDD_ZSkip, fx, fx);
        if(g != -1)
        {
          int gtop = g.Top();
          int glev = BDD_LevOfVar(gtop);
	  if(glev >= lev)
	  {
            f = g;
	    ftop = gtop;
	    flev = glev;
	    continue;
	  }
        }
      }
    }
    u = f;
    f = f.OffSet(ftop);
    ftop = f.Top();
    flev = BDD_LevOfVar(ftop);
  }
  return (last == 0 || lev == flev)? f: u;
}

void ZBDD::SetZSkip() const
{
  int t = Top();
  int lev = BDD_LevOfVar(t);
  if(lev <= 4) return;
  bddword fx = GetID();
  ZBDD g = BDD_CacheZBDD(BC_ZBDD_ZSkip, fx, fx);
  if(g != -1) return;
  ZBDD f0 = OffSet(t);
  f0.SetZSkip();
  g = ZLev(ZLevNum(lev), 1);
  if(g == *this) g = f0;
  bddword gx = g.GetID();
  BDD_CacheEnt(BC_ZBDD_ZSkip, fx, fx, gx);
  OnSet0(t).SetZSkip();
}

ZBDD ZBDD::Intersec(const ZBDD& g) const
{
  if(g == 0) return 0;
  if(g == 1) return *this & 1;
  int ftop = Top();
  if(ftop == 0) return *this & g;
  int gtop = g.Top();

  bddword fx = GetID();
  bddword gx = g.GetID();
  if(fx < gx) { fx = g.GetID(); gx = GetID(); }
  ZBDD_CACHE_CHK_RETURN(BC_ZBDD_INTERSEC, fx, gx);

  int flev = BDD_LevOfVar(ftop);
  int glev = BDD_LevOfVar(gtop);
  ZBDD h;
  if(flev > glev) h = ZLev(glev).Intersec(g);
  else if(flev < glev) h = Intersec(g.OffSet(gtop));
  else
  {
    h = OnSet0(ftop).Intersec(g.OnSet0(ftop)).Change(ftop)
      + OffSet(ftop).Intersec(g.OffSet(ftop));
  }

  ZBDD_CACHE_ENT_RETURN(BC_ZBDD_INTERSEC, fx, gx, h);
}

