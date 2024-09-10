/*****************************************
*  BDD Package (SAPPORO-1.94)   - Body   *
*  (C) Shin-ichi MINATO  (Apr. 19, 2022)   *
******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bddc.h"

/* ----------------- MACRO Definitions ---------------- */
/* Operation IDs in Cache */
#define BC_NULL        0
#define BC_AND         1
#define BC_XOR         2
#define BC_AT0         3
#define BC_AT1         4
#define BC_LSHIFT      5
#define BC_RSHIFT      6
#define BC_COFACTOR    7
#define BC_UNIV        8
#define BC_SUPPORT     9
#define BC_INTERSEC   10
#define BC_UNION      11
#define BC_SUBTRACT   12
#define BC_OFFSET     13
#define BC_ONSET      14
#define BC_CHANGE     15
#define BC_CARD       16
#define BC_LIT        17
#define BC_LEN        18
#define BC_CARD2      19

/* Macros for malloc, realloc */
#define B_MALLOC(type, size) \
  (type *)malloc(sizeof(type) * size)
#define B_REALLOC(ptr, type, size) \
  (type *)realloc(ptr, sizeof(type) * size)

/* Printf format of bddp */
#ifdef B_64
#  define B_BDDP_FD "%lld"
#  define B_BDDP_FX "0x%llX"
#else
#  define B_BDDP_FD "%d"
#  define B_BDDP_FX "0x%X"
#endif

/* strtol or strtoll */
#ifdef B_64
#  define B_STRTOI strtoll
#else
#  define B_STRTOI strtol
#endif

/* Table spaces */
#define B_NODE_MAX (B_VAL_MASK>>1U) /* Max number of BDD nodes */
#define B_NODE_SPC0 256 /* Default initial node size */
#define B_VAR_SPC0   16 /* Initial var table size */
#define B_HASH_SPC0   4 /* Initial hash size */
#define B_RFCT_SPC0   4 /* Initial RFCT size */

/* Negative edge manipulation */
#define B_NEG(f)  ((f) & B_INV_MASK)
#define B_NOT(f)  ((f) ^ B_INV_MASK)
#define B_ABS(f)  ((f) & ~B_INV_MASK)

/* Constant node manipulation */
#define B_CST(f)  ((f) & B_CST_MASK)
#define B_VAL(f)  ((f) & B_VAL_MASK)

/* Conversion of bddp and node index/pointer  */
#define B_NP(f)       (Node+(B_ABS(f)>>1U))
#define B_NDX(f)      (B_ABS(f)>>1U)
#define B_BDDP_NP(p)  ((bddp)((p)-Node) << 1U)

/* Read & Write of bddp field in the tables */
#ifdef B_64
#  define B_LOW32(f) ((bddp_32)((f)&((1ULL<<32U)-1U)))
#  define B_HIGH8(f) ((bddp_h8)((f)>>32U))
#  define B_SET_NXP(p, f, i) \
    (p ## _h8 = f ## _h8 + i, p ## _32 = f ## _32 + i)
#  define B_GET_BDDP(f) \
    ((bddp) f ## _32 | ((bddp) f ## _h8 << 32U))
#  define B_SET_BDDP(f, g) \
    (f ## _h8 = B_HIGH8(g), f ## _32 = B_LOW32(g))
#  define B_CPY_BDDP(f, g) \
    (f ## _h8 = g ## _h8, f ## _32 = g ## _32)
#else
#  define B_SET_NXP(p, f, i) (p ## _32 = f ## _32 + i)
#  define B_GET_BDDP(f) (f ## _32)
#  define B_SET_BDDP(f, g) (f ## _32 = g)
#  define B_CPY_BDDP(f, g) (f ## _32 = g ## _32)
#endif /* B_64 */

/* var & rfc manipulation */
#define B_VAR_NP(p)    ((p)->varrfc & B_VAR_MASK)
#define B_RFC_MASK  (~B_VAR_MASK)
#define B_RFC_UNIT  (1U << B_VAR_WIDTH)
#define B_RFC_NP(p)    ((p)->varrfc >> B_VAR_WIDTH)
#define B_RFC_ZERO_NP(p) ((p)->varrfc < B_RFC_UNIT)
#define B_RFC_ONE_NP(p) (((p)->varrfc & B_RFC_MASK) == B_RFC_UNIT)
#define B_RFC_INC_NP(p) \
  (((p)->varrfc < B_RFC_MASK - B_RFC_UNIT)? \
   ((p)->varrfc += B_RFC_UNIT, 0) : rfc_inc_ovf(p)) 
#define B_RFC_DEC_NP(p) \
  (((p)->varrfc >= B_RFC_MASK)? rfc_dec_ovf(p): \
   (B_RFC_ZERO_NP(p))? \
    err("B_RFC_DEC_NP: rfc under flow", p-Node): \
    ((p)->varrfc -= B_RFC_UNIT, 0))

/* ----------- Stack overflow limitter ------------ */
const int BDD_RecurLimit = 8192;
int BDD_RecurCount = 0;
#define BDD_RECUR_INC \
  {if(++BDD_RecurCount >= BDD_RecurLimit) \
    err("BDD_RECUR_INC: Recursion Limit", BDD_RecurCount);}
#define BDD_RECUR_DEC BDD_RecurCount--

/* Conversion of ZBDD node flag */
#define B_Z_NP(p) ((p)->f0_32 & (bddp_32)B_INV_MASK)

/* Hash Functions */
#define B_HASHKEY(f0, f1, hashSpc) \
  (((B_CST(f0)? (f0): ((f0)+2U)) \
   ^(B_NEG(f0)? ~((f0)>>1U): ((f0)>>1U))\
   ^((B_CST(f1)? (f1): ((f1)+2U))) \
   ^((B_NEG(f1)? ~((f1)>>1U):((f1)>>1U))<<4U))\
  & (hashSpc-1U))
#define B_CACHEKEY(op, f, g) \
  ((((bddp)(op)<<4U)\
   ^((B_CST(f)? (f):((f)+2U)))\
   ^((B_NEG(f)? ~((f)>>1U): ((f)>>1U))) \
   ^((B_CST(g)? (g):((g)+2U))) \
   ^((B_NEG(g)? ~((g)>>1U):((g)>>1U))*4369U) )\
   & (CacheSpc-1U))

/* Multi-Precision Count */
#define B_MP_LWID 4U
#define B_MP_LPOS (B_MSB_POS - B_MP_LWID)
#define B_MP_LMAX (1U<<B_MP_LWID)
#define B_MP_NULL (B_CST_MASK + B_VAL_MASK)
#define B_MP_LEN(f) (B_CST(f)? (B_VAL(f)>>B_MP_LPOS)+1: 0)
#define B_MP_VAL(f) ((f) & (B_VAL_MASK>>B_MP_LWID))

/* ------- Declaration of static (internal) data ------- */
/* typedef of bddp field in the tables */
typedef unsigned int bddp_32;
#ifdef B_64
  typedef unsigned char bddp_h8;
#endif

/* Declaration of Node table */
struct B_NodeTable
{
  bddp_32      f0_32;  /* 0-edge */
  bddp_32      f1_32;  /* 1-edge */
  bddp_32      nx_32;  /* Node index */
  unsigned int varrfc; /* VarID & Reference counter */
#ifdef B_64
  bddp_h8      f0_h8;  /* Extention of 0-edge */
  bddp_h8      f1_h8;  /* Extention of 1-edge */
  bddp_h8      nx_h8;  /* Extention of node index */
#endif /* B_64 */
};
static struct B_NodeTable *Node = 0; /* Node Table */
static bddp NodeLimit = 0;    /* Final limit size */
static bddp NodeUsed = 0;     /* Number of used node */
static bddp Avail = bddnull;  /* Head of available node */
static bddp NodeSpc = 0;      /* Current Node-Table size */

/* Declaration of Hash-table per Var */
struct B_VarTable
{
  bddp    hashSpc;  /* Current hash-table size */
  bddp    hashUsed;  /* Current used entries */
  bddvar  lev;      /* Level of the variable */
  bddp_32 *hash_32; /* Hash-table */
#ifdef B_64
  bddp_h8 *hash_h8; /* Extension of hash-table */
#endif /* B_64 */
};
static struct B_VarTable *Var = 0; /* Var-tables */
static bddvar *VarID = 0;     /* VarID reverse table */
static bddvar VarUsed = 0;    /* Number of used Var */
static bddvar VarSpc = 0;     /* Current Var-table size */

/* Declaration of Operation Cache */
struct B_CacheTable
{
  bddp_32       f_32; /* an operand BDD */
  bddp_32       g_32; /* an operand BDD */
  bddp_32       h_32; /* Result BDD */
  unsigned char op;   /* Operation code */
#ifdef B_64
  bddp_h8       f_h8; /* Extention of an operand BDD */
  bddp_h8       g_h8; /* Extention of an operand BDD */
  bddp_h8       h_h8; /* Extention of result BDD */
#endif /* B_64 */
};
static struct B_CacheTable *Cache = 0; /* Opeartion cache */
static bddp CacheSpc = 0;           /* Current cache size */

/* Declaration of RFC-table */
struct B_RFC_Table
{
  bddp_32 nx_32;   /* Node index */
  bddp_32 rfc_32;  /* RFC */
#ifdef B_64
  bddp_h8 nx_h8;   /* Extension of Node index */
  bddp_h8 rfc_h8;  /* Extension of RFC */
#endif /* B_64 */
};
static struct B_RFC_Table *RFCT = 0; /* RFC-Table */
static bddp RFCT_Spc;   /* Current RFC-table size */
static bddp RFCT_Used;  /* Current RFC-table used entries */

/* Declaration of MP-Count */
struct B_MPTable
{
  bddp size;  /* Table size */
  bddp used;  /* Used entries */
  bddp* word; /* Table head */
};
static struct B_MPTable mptable[B_MP_LMAX] = {0}; /* MP-Count Table */

struct B_MP
{
  int len;
  bddp word[B_MP_LMAX];
};

/* ----- Declaration of static (internal) functions ------ */
/* Private procedure */
static int  err B_ARG((const char *msg, bddp num));
static int  rfc_inc_ovf B_ARG((struct B_NodeTable *np));
static int  rfc_dec_ovf B_ARG((struct B_NodeTable *np));
static void var_enlarge B_ARG((void));
static int  node_enlarge B_ARG((void));
static int  hash_enlarge B_ARG((bddvar v));
static bddp getnode B_ARG((bddvar v, bddp f0, bddp f1));
static bddp getbddp B_ARG((bddvar v, bddp f0, bddp f1));
static bddp getzbddp B_ARG((bddvar v, bddp f0, bddp f1));
static bddp apply B_ARG((bddp f, bddp g, unsigned char op, unsigned char skip));
static void gc1 B_ARG((struct B_NodeTable *np));
static bddp count B_ARG((bddp f));
static void dump B_ARG((bddp f));
static void reset B_ARG((bddp f));
static void export_static B_ARG((FILE *strm, bddp f));
static int import B_ARG((FILE *strm, bddp *p, int lim, int z));
static int andfalse B_ARG((bddp f, bddp g));

static int mp_add B_ARG((struct B_MP *p, bddp ix));

/* ------------------ Body of program -------------------- */
/* ----------------- External functions ------------------ */
int bddinit(bddp initsize, bddp limitsize)
/* Returns 1 if not enough memory (usually 0) */
{
  bddp   ix;
  bddvar i;

  /* Check dupulicate initialization */
  if(Node){ free(Node); Node = 0; }
  if(Var)
  {
    for(i=0; i<VarSpc; i++)
    {
      if(Var[i].hash_32) free(Var[i].hash_32);
#ifdef B_64
      if(Var[i].hash_h8) free(Var[i].hash_h8);
#endif
    }
    free(Var); Var = 0;
  }
  if(VarID){ free(VarID); VarID = 0; }
  if(Cache){ free(Cache); Cache = 0; }

  /* Set NodeLimit */
  if(limitsize < B_NODE_SPC0) NodeLimit = B_NODE_SPC0;
  else if(limitsize > B_NODE_MAX) NodeLimit = B_NODE_MAX;
  else NodeLimit = limitsize;

  /* Set NodeSpc */
  if(initsize < B_NODE_SPC0) NodeSpc = B_NODE_SPC0;
  else if(initsize > NodeLimit) NodeSpc = NodeLimit;
  else NodeSpc = initsize;

  /* Set CacheSpc */
  for(CacheSpc=B_NODE_SPC0; CacheSpc<NodeSpc>>1; CacheSpc<<=1U)
    ; /* empty */

  /* Set VarSpc */
  VarSpc = B_VAR_SPC0;

  /* Memory allocation */
  Node = B_MALLOC(struct B_NodeTable, NodeSpc);
  Var = B_MALLOC(struct B_VarTable, VarSpc);
  VarID = B_MALLOC(bddvar, VarSpc);
  Cache = B_MALLOC(struct B_CacheTable, CacheSpc);

  /* Check overflow */
  if(Node == 0 || Var == 0 || VarID == 0 || Cache == 0)
  {
    if(Cache){ free(Cache); Cache = 0; }
    if(VarID){ free(VarID); VarID = 0; }
    if(Var){ free(Var); Var = 0; }
    if(Node){ free(Node); Node = 0; }
    NodeLimit = 0;
    return 1;
  }

  /* Initialize */
  NodeUsed = 0;
  Node[NodeSpc-1U].varrfc = 0;
  B_SET_BDDP(Node[NodeSpc-1U].nx, bddnull);
  for(ix=0; ix<NodeSpc-1U; ix++)
  {
    Node[ix].varrfc = 0;
    B_SET_BDDP(Node[ix].nx, ix+1U);
  }
  Avail = 0;

  VarUsed = 0;
  for(i=0; i<VarSpc; i++)
  {
    Var[i].hashSpc = 0;
    Var[i].hashUsed = 0;
    Var[i].lev = i;
    VarID[i] = i;
    Var[i].hash_32 = 0;
#ifdef B_64
    Var[i].hash_h8 = 0;
#endif
  }

  for(ix=0; ix<CacheSpc; ix++) Cache[ix].op = BC_NULL;

  /* Init RFC Table */
  if(RFCT){ free(RFCT); RFCT = 0; }
  RFCT_Spc = 0;
  RFCT_Used = 0;

  /* Init MP-Count Table */
  for(i=0; i<B_MP_LMAX; i++)
  {
    mptable[i].size = 0;
    mptable[i].used = 0;
    if(mptable[i].word) { free(mptable[i].word); mptable[i].word = 0; }
  }

  return 0;
}

bddp bddcopy(bddp f)
{
  struct B_NodeTable *fp;

  if(f == bddnull) return bddnull;
  if(B_CST(f)) return f; /* Constant */
  fp = B_NP(f);
  if(fp >= Node+NodeSpc || fp->varrfc == 0)
    err("bddcopy: Invalid bddp", f);
  B_RFC_INC_NP(fp);
  return f;
}

void bddfree(bddp f)
{
  struct B_NodeTable *fp;

  if(f == bddnull) return;
  if(B_CST(f)) return; /* Constant */
  fp = B_NP(f);
  if(fp >= Node+NodeSpc || fp->varrfc == 0)
    err("bddfree: Invalid bddp", f);
  B_RFC_DEC_NP(fp);
}

int bddgc()
/* Returns 1 if there are no free node (usually 0) */
{
  bddp i, n, f;
  struct B_NodeTable *fp;
  struct B_CacheTable *cachep;
  struct B_NodeTable *np;
  struct B_VarTable *varp;
  bddvar v;
  bddp oldSpc, newSpc, nx, key;
  bddp_32 *newhash_32, *p_32, *p2_32;
#ifdef B_64
  bddp_h8 *newhash_h8, *p_h8, *p2_h8;
#endif

  n = NodeUsed; 
  for(fp=Node; fp<Node+NodeSpc; fp++)
    if(fp->varrfc != 0 && B_RFC_ZERO_NP(fp))
      gc1(fp);
  if(n == NodeUsed) return 1; /* No free node */

  /* Cache clear */
  for(cachep=Cache; cachep<Cache+CacheSpc; cachep++)
  {
    switch(cachep->op)
    {
    case BC_NULL:
      break;
    case BC_AND:
    case BC_XOR:
    case BC_INTERSEC:
    case BC_UNION:
    case BC_SUBTRACT:
    case BC_CHANGE:
      f = B_GET_BDDP(cachep->f);
      if(!B_CST(f) && (fp=B_NP(f))<Node+NodeSpc && fp->varrfc == 0)
      {
        cachep->op = BC_NULL;
        break;
      }
      f = B_GET_BDDP(cachep->g);
      if(!B_CST(f) && (fp=B_NP(f))<Node+NodeSpc && fp->varrfc == 0)
      {
        cachep->op = BC_NULL;
        break;
      }
      f = B_GET_BDDP(cachep->h);
      if(!B_CST(f) && (fp=B_NP(f))<Node+NodeSpc && fp->varrfc == 0)
      {
        cachep->op = BC_NULL;
        break;
      }
      break;
    case BC_AT0:
    case BC_AT1:
    case BC_OFFSET:
    case BC_ONSET:
      f = B_GET_BDDP(cachep->f);
      if(!B_CST(f) && (fp=B_NP(f))<Node+NodeSpc && fp->varrfc == 0)
      {
        cachep->op = BC_NULL;
        break;
      }
      f = B_GET_BDDP(cachep->h);
      if(!B_CST(f) && (fp=B_NP(f))<Node+NodeSpc && fp->varrfc == 0)
      {
        cachep->op = BC_NULL;
        break;
      }
      break;
    case BC_CARD:
    case BC_LIT:
    case BC_LEN:
      f = B_GET_BDDP(cachep->f);
      if(!B_CST(f) && (fp=B_NP(f))<Node+NodeSpc && fp->varrfc == 0)
      {
        cachep->op = BC_NULL;
        break;
      }
      f = B_GET_BDDP(cachep->h);
      if(f > bddnull)
      {
        cachep->op = BC_NULL;
        break;
      }
      break;
    default:
      cachep->op = BC_NULL;
      break;
    }
  }

  /* MP-Count table clear */
  for(i=0; i<B_MP_LMAX; i++)
  {
    mptable[i].size = 0;
    mptable[i].used = 0;
    free(mptable[i].word);
    mptable[i].word = 0;
  }

  /* Hash-table packing */
  for(v=1; v<=VarUsed; v++)
  {
    varp = &Var[v];

    /* Get new size */
    oldSpc = varp->hashSpc;
    newSpc = oldSpc;
    while(newSpc > B_HASH_SPC0)
    {
      if(newSpc>>2 < varp->hashUsed) break;
      newSpc >>= 1;
    }
    if(newSpc == oldSpc) continue;

    /* Reduce space */
#ifdef B_64
    newhash_32 = 0;
    newhash_h8 = 0;
    newhash_32 = B_MALLOC(bddp_32, newSpc);
    newhash_h8 = B_MALLOC(bddp_h8, newSpc);
    if(!newhash_32 || !newhash_h8)
    {
      if(newhash_32) free(newhash_32);
      if(newhash_h8) free(newhash_h8);
      break; /* Not enough memory */
    }
#else
    newhash_32 = 0;
    newhash_32 = B_MALLOC(bddp_32, newSpc);
    if(!newhash_32) break; /* Not enough memory */
#endif

    /* Initialize new hash entry */
    for(i=0; i<newSpc; i++)
    {
      B_SET_NXP(p, newhash, i);
      B_SET_BDDP(*p, bddnull);
    }

    /* restore hash entry */
    for(i=0; i<oldSpc; i++)
    {
      key = i & (newSpc-1U);
      np = 0;
      B_SET_NXP(p, newhash, key);
      nx = B_GET_BDDP(*p);
      while(nx != bddnull)
      {
        np = Node + nx;
        nx = B_GET_BDDP(np->nx);
      }
      if(np) { B_SET_NXP(p2, varp->hash, i); B_CPY_BDDP(np->nx, *p2); }
      else
      {
        B_SET_NXP(p, newhash, key);
        B_SET_NXP(p2, varp->hash, i);
        B_CPY_BDDP(*p, *p2);
      }
    }
    varp->hashSpc = newSpc;
    free(varp->hash_32);
    varp->hash_32 = newhash_32;
#ifdef B_64
    free(varp->hash_h8);
    varp->hash_h8 = newhash_h8;
#endif
  }
  return 0;
}

bddp bddused() { return NodeUsed; }

bddp bddsize(bddp f)
/* Returns 0 for bddnull */
{
  bddp num;
  struct B_NodeTable *fp;

  if(f == bddnull) return 0;
  if(B_CST(f)) return 0; /* Constant */
  if((fp=B_NP(f))>=Node+NodeSpc || fp->varrfc == 0)
    err("bddsize: Invalid bddp", f);

  num = count(f);
  reset(f);
  return num;
}

bddp bddvsize(bddp *p, int lim)
/* Returns 0 for bddnull */
{
  bddp num;
  struct B_NodeTable *fp;
  int n, i;

  /* Check operand */
  n = lim;
  for(i=0; i<n; i++)
  {
    if(p[i] == bddnull)
    {
      n = i;
      break;
    }
    if(!B_CST(p[i])&&
       ((fp=B_NP(p[i]))>=Node+NodeSpc || fp->varrfc==0))
      err("bddvsize: Invalid bddp", p[i]);
  }
  num = 0;
  for(i=0; i<n; i++) num += count(p[i]);
  for(i=0; i<n; i++) reset(p[i]);
  return num;
}

void bddexport(FILE *strm, bddp *p, int lim)
{
  struct B_NodeTable *fp;
  int n, i, lev, lev0;

  /* Check operands */
  n = lim;
  lev = 0;
  for(i=0; i<n; i++)
  {
    if(p[i] == bddnull)
    {
      n = i;
      break;
    }
    if(!B_CST(p[i])&&
       ((fp=B_NP(p[i]))>=Node+NodeSpc || fp->varrfc==0))
      err("bddvexport: Invalid bddp", p[i]);
    lev0 = bddlevofvar(bddtop(p[i]));
    if(lev0 > lev) lev = lev0;
  }

  fprintf(strm, "_i %d\n_o %d\n_n ", lev, n);
  fprintf(strm, B_BDDP_FD, bddvsize(p, n));
  fprintf(strm, "\n");

  /* Put internal nodes */
  for(i=0; i<n; i++) export_static(strm, p[i]);
  for(i=0; i<n; i++) reset(p[i]);

  /* Put external node */
  for(i=0; i<n; i++)
  {
    if(p[i] == bddfalse) fprintf(strm, "F");
    else if(p[i] == bddtrue) fprintf(strm, "T");
    else fprintf(strm, B_BDDP_FD, p[i]);
    fprintf(strm, "\n");
  }
}

void bdddump(bddp f)
{
  struct B_NodeTable *fp;

  /* Check indexes */
  if(f == bddnull) { printf("RT = NULL\n\n"); return; }
  if(!B_CST(f)&&
     ((fp=B_NP(f))>=Node+NodeSpc || fp->varrfc==0))
      err("bdddump: Invalid bddp", f);

  /* Dump nodes */
  dump(f);
  reset(f);

  /* Dump top node */
  printf("RT = ");
  if(B_NEG(f)) putchar('~');
  if(B_CST(f)) printf(B_BDDP_FD, B_ABS(B_VAL(f)));
  else { printf("N"); printf(B_BDDP_FD, B_NDX(f)); }
  printf("\n\n");
}

void bddvdump(bddp *p, int n)
{
  struct B_NodeTable *fp;
  int i;

  /* Check operands */
  for(i=0; i<n; i++)
  {
    if(p[i] == bddnull) return;
    if(!B_CST(p[i])&&
       ((fp=B_NP(p[i]))>=Node+NodeSpc || fp->varrfc==0))
      err("bddvdump: Invalid bddp", p[i]);
  }

  /* Dump nodes */
  for(i=0; i<n; i++) if(p[i] != bddnull) dump(p[i]);
  for(i=0; i<n; i++) if(p[i] != bddnull) reset(p[i]);

  /* Dump top node */
  for(i=0; i<n; i++)
  {
    printf("RT%d = ", i);
    if(p[i] == bddnull) printf("NULL");
    else
    {
      if(B_NEG(p[i])) putchar('~');
      if(B_CST(p[i])) printf(B_BDDP_FD, B_ABS(B_VAL(p[i])));
      else { printf("N"); printf(B_BDDP_FD, B_NDX(p[i])); }
    }
    putchar('\n');
  }
  printf("\n");
}

bddp bddrcache(unsigned char op, bddp f, bddp g)
{
  struct B_CacheTable *cachep;

  cachep = Cache + B_CACHEKEY(op, f, g);
  if(op == cachep->op &&
     f == B_GET_BDDP(cachep->f) &&
     g == B_GET_BDDP(cachep->g))
    return B_GET_BDDP(cachep->h); /* Hit */
  return bddnull;
}

void bddwcache(unsigned char op, bddp f, bddp g, bddp h)
{
  struct B_CacheTable *cachep;

  if(op < 20) err("bddwcache: op < 20", op);
  if(h == bddnull) return;
  cachep = Cache + B_CACHEKEY(op, f, g);
  cachep->op = op;
  B_SET_BDDP(cachep->f, f);
  B_SET_BDDP(cachep->g, g);
  B_SET_BDDP(cachep->h, h);
}

bddp bddnot(bddp f)
{
  if(f == bddnull) return bddnull;
  return B_NOT(bddcopy(f));
}

bddvar bddlevofvar(bddvar v)
{
  if(v > VarUsed)
    err("bddlevofvar: Invalid VarID", v);
  return Var[v].lev;
}

bddvar bddvaroflev(bddvar lev)
{
  if(lev > VarUsed)
    err("bddvaroflev: Invalid level", lev);
  return VarID[lev];
}

bddvar bddvarused()
{
  return VarUsed;
}

bddvar bddnewvar()
{
  if(++VarUsed == VarSpc) var_enlarge();
  return VarUsed;
}

bddvar bddnewvaroflev(bddvar lev)
{
  bddvar i;

  if(lev == 0 || lev > ++VarUsed)
    err("bddnewvaroflev: Invalid level", lev);
  if(VarUsed == VarSpc) var_enlarge();
  for(i=VarUsed; i>lev; i--) Var[ VarID[i] = VarID[i-1U] ].lev = i;
  Var[ VarID[lev] = VarUsed ].lev = lev;
  return VarUsed;
}

bddvar bddtop(bddp f)
{
  struct B_NodeTable *fp;

  if(f == bddnull) return 0;
  if(B_CST(f)) return 0; /* Constant */
  fp = B_NP(f);
  if(fp >= Node+NodeSpc || fp->varrfc == 0)
    err("bddtop: Invalid bddp", f);
  return B_VAR_NP(fp);
}

bddp    bddprime(bddvar v)
/* Returns bddnull if not enough memory */
{
        if(v == 0 || v > VarUsed)
		err("bddprime: Invalid VarID", v);
        return getbddp(v, bddfalse, bddtrue);
}


bddp bddand(bddp f, bddp g)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;
   
  /* Check operands */
  if(f == bddnull) return bddnull;
  if(g == bddnull) return bddnull;
  if(B_CST(f))
  { if(B_ABS(f) != bddfalse) err("bddand: Invalid bddp", f); }
  else
  {
    fp = B_NP(f);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddand: Invalid bddp", f);
    if(B_Z_NP(fp)) err("bddand: applying ZBDD node", f);
  }
  if(B_CST(g))
  { if(B_ABS(g) != bddfalse) err("bddand: Invalid bddp", g); }
  else
  {
    fp = B_NP(g);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddand: Invalid bddp", g);
    if(B_Z_NP(fp)) err("bddand: applying ZBDD node", g);
  }

  return apply(f, g, BC_AND, 0);
}

bddp bddor(bddp f, bddp g)
/* Returns bddnull if not enough memory */
{
  bddp h;

  if(f == bddnull) return bddnull;
  if(g == bddnull) return bddnull;
  h = bddand(B_NOT(f), B_NOT(g));
  if(h == bddnull) return bddnull;
  return B_NOT(h);
}

bddp bddxor(bddp f, bddp g)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;
   
  /* Check operands */
  if(f == bddnull) return bddnull;
  if(g == bddnull) return bddnull;
  if(B_CST(f))
  { if(B_ABS(f) != bddfalse) err("bddand: Invalid bddp", f); }
  else
  {
    fp = B_NP(f);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddxor: Invalid bddp", f);
    if(B_Z_NP(fp)) err("bddand: applying ZBDD node", f);
  }
  if(B_CST(g))
  { if(B_ABS(g) != bddfalse) err("bddand: Invalid bddp", g); }
  else
  {
    fp = B_NP(g);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddxor: Invalid bddp", g);
    if(B_Z_NP(fp)) err("bddand: applying ZBDD node", g);
  }

  return apply(f, g, BC_XOR, 0);
}

bddp bddnand(bddp f, bddp g)
/* Returns bddnull if not enough memory */
{
  bddp h;

  h = bddand(f, g);
  if(h == bddnull) return bddnull;
  return B_NOT(h);
}

bddp bddnor(bddp f, bddp g)
/* Returns bddnull if not enough memory */
{
  if(f == bddnull) return bddnull;
  if(g == bddnull) return bddnull;
  return bddand(B_NOT(f), B_NOT(g));
}

bddp bddxnor(bddp f, bddp g)
/* Returns bddnull if not enough memory */
{
  if(g == bddnull) return bddnull;
  return bddxor(f, B_NOT(g));
}

bddp bddcofactor(bddp f, bddp g)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;
   
  /* Check operands */
  if(f == bddnull) return bddnull;
  if(g == bddnull) return bddnull;
  if(B_CST(f))
  { if(B_ABS(f) != bddfalse) err("bddcofactor: Invalid bddp", f); }
  else
  {
    fp = B_NP(f);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddcofactor: Invalid bddp", f);
    if(B_Z_NP(fp)) err("bddcofactor: applying ZBDD node", f);
  }
  if(B_CST(g))
  { if(B_ABS(g) != bddfalse) err("bddcofactor: Invalid bddp", g); }
  else
  {
    fp = B_NP(g);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddcofactor: Invalid bddp", g);
    if(B_Z_NP(fp)) err("bddcofactor: applying ZBDD node", g);
  }

  return apply(f, g, BC_COFACTOR, 0);
}

bddp bdduniv(bddp f, bddp g)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;
   
  /* Check operands */
  if(f == bddnull) return bddnull;
  if(g == bddnull) return bddnull;
  if(B_CST(f))
  { if(B_ABS(f) != bddfalse) err("bdduniv: Invalid bddp", f); }
  else
  {
    fp = B_NP(f);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bdduniv: Invalid bddp", f);
    if(B_Z_NP(fp)) err("bdduniv: applying ZBDD node", f);
  }
  if(B_CST(g))
  { if(B_ABS(g) != bddfalse) err("bdduniv: Invalid bddp", g); }
  else
  {
    fp = B_NP(g);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bdduniv: Invalid bddp", g);
    if(B_Z_NP(fp)) err("bdduniv: applying ZBDD node", g);
  }

  return apply(f, g, BC_UNIV, 0);
}

bddp bddexist(bddp f, bddp g)
/* Returns bddnull if not enough memory */
{
  bddp h;

  if(f == bddnull) return bddnull;
  if(g == bddnull) return bddnull;
  h = bdduniv(B_NOT(f), g);
  if(h == bddnull) return bddnull;
  return B_NOT(h);
}

int bddimply(bddp f, bddp g)
{
  struct B_NodeTable *fp;
   
  /* Check operands */
  if(f == bddnull) return 0;
  if(g == bddnull) return 0;
  if(B_CST(f))
  { if(B_ABS(f) != bddfalse) err("bddimply: Invalid bddp", f); }
  else
  {
    fp = B_NP(f);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddimply: Invalid bddp", f);
    if(B_Z_NP(fp)) err("bddimply: applying ZBDD node", f);
  }
  if(B_CST(g))
  { if(B_ABS(g) != bddfalse) err("bddimply: Invalid bddp", g); }
  else
  {
    fp = B_NP(g);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddimply: Invalid bddp", g);
    if(B_Z_NP(fp)) err("bddimply: applying ZBDD node", g);
  }

  return ! andfalse(f, B_NOT(g));
}

bddp bddsupport(bddp f)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;

  /* Check operands */
  if(f == bddnull) return bddnull;
  if(B_CST(f)) return bddfalse;
  fp = B_NP(f);
  if(fp>=Node+NodeSpc || !fp->varrfc)
    err("bddsupport: Invalid bddp", f);

  return apply(f, bddfalse, BC_SUPPORT, 0);
}

bddp bddat0(bddp f, bddvar v)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;

  /* Check operands */
  if(v > VarUsed || v == 0) err("bddat0: Invalid VarID", v);
  if(f == bddnull) return bddnull;
  if(B_CST(f)) return f;
  fp = B_NP(f);
  if(fp>=Node+NodeSpc || !fp->varrfc)
    err("bddat0: Invalid bddp", f);

  return apply(f, (bddp)v, BC_AT0, 0);
}

bddp bddat1(bddp f, bddvar v)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;

  /* Check operands */
  if(v > VarUsed || v == 0) err("bddat1: Invalid VarID", v);
  if(f == bddnull) return bddnull;
  if(B_CST(f)) return f;
  fp = B_NP(f);
  if(fp>=Node+NodeSpc || !fp->varrfc)
    err("bddat1: Invalid bddp", f);

  return apply(f, (bddp)v, BC_AT1, 0);
}

bddp bddlshift(bddp f, bddvar shift)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;

  /* Check operands */
  if(shift >= VarUsed)
    err("bddlshift: Invalid shift", shift);
  if(f == bddnull) return bddnull;
  if(B_CST(f)) return f;
  if(shift == 0) return bddcopy(f);
  if((fp=B_NP(f))>=Node+NodeSpc || !fp->varrfc)
    err("bddlshift: Invalid bddp", f);

  return apply(f, (bddp)shift, BC_LSHIFT, 0);
}

bddp bddrshift(bddp f, bddvar shift)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;

  /* Check operands */
  if(shift >= VarUsed)
    err("bddrshift: Invalid shift", shift);
  if(f == bddnull) return bddnull;
  if(B_CST(f)) return f;
  if(shift == 0) return bddcopy(f);
  if((fp=B_NP(f))>=Node+NodeSpc || !fp->varrfc)
    err("bddrshift: Invalid bddp", f);

  return apply(f, (bddp)shift, BC_RSHIFT, 0);
}

bddp    bddoffset(bddp f, bddvar v)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;

  /* Check operands */
  if(v > VarUsed || v == 0) err("bddoffset: Invalid VarID", v);
  if(f == bddnull) return bddnull;
  if(B_CST(f)) return f;
  fp = B_NP(f);
  if(fp>=Node+NodeSpc || !fp->varrfc)
    err("bddoffset: Invalid bddp", f);
  if(!B_Z_NP(fp)) err("bddoffset: applying non-ZBDD node", f);

  return apply(f, (bddp)v, BC_OFFSET, 0);
}

bddp    bddonset0(bddp f, bddvar v)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;

  /* Check operands */
  if(v > VarUsed || v == 0) err("bddonset0: Invalid VarID", v);
  if(f == bddnull) return bddnull;
  if(B_CST(f)) return bddfalse;
  fp = B_NP(f);
  if(fp>=Node+NodeSpc || !fp->varrfc)
    err("bddonset0: Invalid bddp", f);
  if(!B_Z_NP(fp)) err("bddonset0: applying non-ZBDD node", f);

  return apply(f, (bddp)v, BC_ONSET, 0);
}

bddp    bddonset(bddp f, bddvar v)
/* Returns bddnull if not enough memory */
{
  bddp g, h;

  g = bddonset0(f, v);
  h = bddchange(g, v);
  bddfree(g);
  return h;
}

bddp    bddchange(bddp f, bddvar v)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;

  /* Check operands */
  if(v > VarUsed || v == 0) err("bddchange: Invalid VarID", v);
  if(f == bddnull) return bddnull;
  if(!B_CST(f))
  {
    fp = B_NP(f);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddchange: Invalid bddp", f);
    if(!B_Z_NP(fp)) err("bddchange: applying non-ZBDD node", f);
  }

  return apply(f, (bddp)v, BC_CHANGE, 0);
}

bddp bddintersec(bddp f, bddp g)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;
   
  /* Check operands */
  if(f == bddnull) return bddnull;
  if(g == bddnull) return bddnull;
  if(B_CST(f))
  { if(B_ABS(f) != bddfalse) err("bddintersec: Invalid bddp", f); }
  else
  {
    fp = B_NP(f);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddintersec: Invalid bddp", f);
    if(!B_Z_NP(fp)) err("bddintersec: applying non-ZBDD node", f);
  }
  if(B_CST(g))
  { if(B_ABS(g) != bddfalse) err("bddintersec: Invalid bddp", g); }
  else
  {
    fp = B_NP(g);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddintersec: Invalid bddp", g);
    if(!B_Z_NP(fp)) err("bddintersec: applying non-ZBDD node", g);
  }

  return apply(f, g, BC_INTERSEC, 0);
}

bddp bddunion(bddp f, bddp g)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;
   
  /* Check operands */
  if(f == bddnull) return bddnull;
  if(g == bddnull) return bddnull;
  if(B_CST(f))
  { if(B_ABS(f) != bddfalse) err("bddunion: Invalid bddp", f); }
  else
  {
    fp = B_NP(f);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddunion: Invalid bddp", f);
    if(!B_Z_NP(fp)) err("bddunion: applying non-ZBDD node", f);
  }
  if(B_CST(g))
  { if(B_ABS(g) != bddfalse) err("bddunion: Invalid bddp", g); }
  else
  {
    fp = B_NP(g);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddunion: Invalid bddp", g);
    if(!B_Z_NP(fp)) err("bddunion: applying non-ZBDD node", g);
  }

  return apply(f, g, BC_UNION, 0);
}

bddp bddsubtract(bddp f, bddp g)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;
   
  /* Check operands */
  if(f == bddnull) return bddnull;
  if(g == bddnull) return bddnull;
  if(B_CST(f))
  { if(B_ABS(f) != bddfalse) err("bddsubtract: Invalid bddp", f); }
  else
  {
    fp = B_NP(f);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddsubtarct: Invalid bddp", f);
    if(!B_Z_NP(fp)) err("bddsubtarct: applying non-ZBDD node", f);
  }
  if(B_CST(g))
  { if(B_ABS(g) != bddfalse) err("bddsubtarct: Invalid bddp", g); }
  else
  {
    fp = B_NP(g);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddsubtarct: Invalid bddp", g);
    if(!B_Z_NP(fp)) err("bddsubtarct: applying non-ZBDD node", g);
  }

  return apply(f, g, BC_SUBTRACT, 0);
}

bddp bddcard(bddp f)
{
  struct B_NodeTable *fp;

  if(f == bddnull) return 0;
  if(B_CST(f)) return (f == bddfalse)? 0: 1;
  fp = B_NP(f);
  if(fp>=Node+NodeSpc || !fp->varrfc)
    err("bddcard: Invalid bddp", f);
  if(!B_Z_NP(fp)) err("bddcard: applying non-ZBDD node", f);

  return apply(f, bddfalse, BC_CARD, 0);
}

bddp bddlit(bddp f)
{
  struct B_NodeTable *fp;

  if(f == bddnull) return 0;
  if(B_CST(f)) return 0;
  fp = B_NP(f);
  if(fp>=Node+NodeSpc || !fp->varrfc)
    err("bddlit: Invalid bddp", f);
  if(!B_Z_NP(fp)) err("bddlit: applying non-ZBDD node", f);

  return apply(f, bddfalse, BC_LIT, 0);
}

bddp bddlen(bddp f)
{
  struct B_NodeTable *fp;

  if(f == bddnull) return 0;
  if(B_CST(f)) return 0;
  fp = B_NP(f);
  if(fp>=Node+NodeSpc || !fp->varrfc)
    err("bddlen: Invalid bddp", f);
  if(!B_Z_NP(fp)) err("bddlen: applying non-ZBDD node", f);

  return apply(f, bddfalse, BC_LEN, 0);
}

char *bddcardmp16(bddp f, char *s)
{
  struct B_NodeTable *fp;
  int i, j, k, nz;
  struct B_MP mp;
  bddp h, d;

  mp.len = 1;
  if(f == bddnull) mp.word[0] = 0; 
  else if(B_CST(f)) mp.word[0] = (f == bddtrue)? 1: 0;
  else
  {
    fp = B_NP(f);
    if(fp>=Node+NodeSpc || !fp->varrfc)
      err("bddcardmp16: Invalid bddp", f);
    if(!B_Z_NP(fp)) err("bddcardmp16: applying non-ZBDD node", f);
    h = apply(B_ABS(f), bddfalse, BC_CARD2, 0);
    if(h == B_MP_NULL) mp.len = 0;
    else
    {
      mp.word[0] = B_NEG(f)? 1: 0;
      mp_add(&mp, h);
    }
  }
  if(!s) s = B_MALLOC(char, mp.len*sizeof(bddp)*2+1);
  if(!s) return s;
  k = 0;
  nz = 0;
  for(i=mp.len-1; i>=0; i--)
    for(j=sizeof(bddp)*2-1; j>=0; j--)
    {
      d = (mp.word[i] >> (j*4) ) & 15;
      if(d) nz = 1;
      if(nz) s[k++] = "0123456789ABCDEF"[d];
    }
  if(!nz && mp.len) s[k++] = '0';
  s[k++] = 0;

#ifdef DEBUG
  for(i=0; i<B_MP_LMAX; i++)
  {
    printf("%d: ", i);
    printf(B_BDDP_FD, mptable[i].size);
    printf("\n");
  }
#endif 

  return s;
}

int bddimport(FILE *strm, bddp *p, int lim)
{
  return import(strm, p, lim, 0);
}

int bddimportz(FILE *strm, bddp *p, int lim)
{
  return import(strm, p, lim, 1);
}

int bddisbdd(bddp f)
{
  struct B_NodeTable* fp;

  if(f == bddnull) return 0;
  if(B_CST(f)) return 1;
  if((fp=B_NP(f))>=Node+NodeSpc || !fp->varrfc)
    err("bddisbdd: Invalid bddp", f);

  return (B_NEG(B_GET_BDDP(fp->f0)) ? 0 : 1);
}

int bddiszbdd(bddp f)
{
  struct B_NodeTable* fp;

  if(f == bddnull) return 0;
  if(B_CST(f)) return 1;
  if((fp=B_NP(f))>=Node+NodeSpc || !fp->varrfc)
    err("bddiszbdd: Invalid bddp", f);

  return (B_NEG(B_GET_BDDP(fp->f0)) ? 1 : 0);
}

bddp    bddpush(bddp f, bddvar v)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;

  /* Check operands */
  if(v > VarUsed || v == 0) err("bddpush: Invalid VarID", v);
  if(f == bddnull) return bddnull;

  if(!B_CST(f)) { fp = B_NP(f); B_RFC_INC_NP(fp); }
  return getzbddp(v, bddfalse, f);
}

/* ----------------- Internal functions ------------------ */
static void var_enlarge()
{
  bddvar i, newSpc;
  struct B_VarTable *newVar;
  unsigned int *newVarID;
  
  /* Get new size */
  if(VarSpc == bddvarmax+1U)
    err("var_enlarge: var index range full", VarSpc);
  newSpc = VarSpc << 2U;
  if(newSpc > bddvarmax+1) newSpc = bddvarmax+1U;

  /* Enlarge space */
  newVar = 0;
  newVarID = 0;
  newVar = B_MALLOC(struct B_VarTable, newSpc);
  newVarID = B_MALLOC(unsigned int, newSpc);
  if(newVar && newVarID)
  {
    for(i=0; i<VarSpc; i++)
    {
      newVar[i].hashSpc = Var[i].hashSpc;
      newVar[i].hashUsed = Var[i].hashUsed;
      newVar[i].lev = Var[i].lev;
      newVar[i].hash_32 = Var[i].hash_32;
      newVarID[i] = VarID[i];
#ifdef B_64
      newVar[i].hash_h8 = Var[i].hash_h8;
#endif
    }
    free(Var);
    free(VarID);
    Var = newVar;
    VarID = newVarID;
  }
  else
  {
    if(newVar) free(newVar);
    if(newVarID) free(newVarID);
    err("var_enlarge: memory allocation failed", VarSpc);
  }

  /* Initialize new space */
  for(i=VarSpc; i<newSpc; i++)
  {
    Var[i].hashSpc = 0;
    Var[i].hashUsed = 0;
    Var[i].lev = i;
    Var[i].hash_32 = 0;
    VarID[i] = i;
#ifdef B_64
    Var[i].hash_h8 = 0;
#endif
  }
  VarSpc = newSpc;
}

static int node_enlarge()
/* Returns 1 if not enough memory */
{
  bddp i, newSpc;
  struct B_NodeTable *newNode;
  struct B_CacheTable *newCache, *cp, *cp1;
  
  /* Get new size */
  if(NodeSpc == NodeLimit) return 1; /* Cannot enlarge */
  newSpc = NodeSpc << 1U;
  if(newSpc > NodeLimit) newSpc = NodeLimit;

  /* Enlarge space */
  newNode = 0;
  newNode = B_MALLOC(struct B_NodeTable, newSpc);
  if(newNode)
  {
    for(i=0; i<NodeSpc; i++)
    {
      newNode[i].varrfc = Node[i].varrfc;
      newNode[i].f0_32 = Node[i].f0_32;
      newNode[i].f1_32 = Node[i].f1_32;
      newNode[i].nx_32 = Node[i].nx_32;
#ifdef B_64
      newNode[i].f0_h8 = Node[i].f0_h8;
      newNode[i].f1_h8 = Node[i].f1_h8;
      newNode[i].nx_h8 = Node[i].nx_h8;
#endif /* B_64 */
    }
    free(Node);
    Node = newNode;
  }
  else return 1; /* Not enough memory */

  /* Initialize new space */
  Node[newSpc-1U].varrfc = 0;
  B_SET_BDDP(Node[newSpc-1U].nx, Avail);
  for(i=NodeSpc; i<newSpc-1U; i++)
  {
    Node[i].varrfc = 0;
    B_SET_BDDP(Node[i].nx, i+1U);
  }
  Avail = NodeSpc;
  NodeSpc = newSpc;

  /* Realloc Cache */
  for(newSpc=CacheSpc; newSpc<NodeSpc>>1U; newSpc<<=1U)
    ; /* empty */
  newCache = 0;
  newCache = B_MALLOC(struct B_CacheTable, newSpc);
  if(newCache)
  {
    for(i=0; i<CacheSpc; i++)
    {
    cp = newCache + i;
    cp1 = Cache + i;
    cp->op = cp1->op;
    B_CPY_BDDP(cp->f, cp1->f);
    B_CPY_BDDP(cp->g, cp1->g);
    B_CPY_BDDP(cp->h, cp1->h);
    }
    free(Cache);
    Cache = newCache;
  }
  else return 0; /* Only NodeTable enlarged */

  /* Reconstruct Cache */
  for(i=CacheSpc; i<newSpc; i++)
  {
    cp = Cache + i;
    cp1 = Cache + (i & (CacheSpc - 1));
    cp->op = cp1->op;
    B_CPY_BDDP(cp->f, cp1->f);
    B_CPY_BDDP(cp->g, cp1->g);
    B_CPY_BDDP(cp->h, cp1->h);
  }
  CacheSpc = newSpc;

  return 0;
}

static int hash_enlarge(bddvar v)
/* Returns 1 if not enough memory */
{
  struct B_NodeTable *np, *np0;
  struct B_VarTable *varp;
  bddp i, oldSpc, newSpc, nx, key, f0, f1;
  bddp_32 *newhash_32, *p_32;
#ifdef B_64
  bddp_h8 *newhash_h8, *p_h8;
#endif
  
  varp = &Var[v];
  /* Get new size */
  oldSpc = varp->hashSpc;
  if(oldSpc == B_NODE_MAX + 1U)
    return 0; /*  Cancel enlarging */
  newSpc = oldSpc << 1U;

  /* Enlarge space */
#ifdef B_64
  newhash_32 = 0;
  newhash_h8 = 0;
  newhash_32 = B_MALLOC(bddp_32, newSpc);
  newhash_h8 = B_MALLOC(bddp_h8, newSpc);
  if(newhash_32 && newhash_h8)
  {
    for(i=0; i<varp->hashSpc; i++)
    {
      newhash_32[i] = varp->hash_32[i];
      newhash_h8[i] = varp->hash_h8[i];
    }
    free(varp->hash_32);
    free(varp->hash_h8);
    varp->hash_32 = newhash_32;
    varp->hash_h8 = newhash_h8;
  }
  else
  {
    if(newhash_32) free(newhash_32);
    if(newhash_h8) free(newhash_h8);
    return 1;
  }
#else
  newhash_32 = 0;
  newhash_32 = B_MALLOC(bddp_32, newSpc);
  if(newhash_32)
  {
    for(i=0; i<varp->hashSpc; i++) newhash_32[i] = varp->hash_32[i];
    free(varp->hash_32);
    varp->hash_32 = newhash_32;
  }
  else return 1; /* Not enough memory */
#endif
  varp->hashSpc = newSpc;

  /* Initialize new hash entry */
  for(i=oldSpc; i<newSpc; i++)
  {
    B_SET_NXP(p, varp->hash, i);
    B_SET_BDDP(*p, bddnull);
  }

  /* restore hash entry */
  for(i=0; i<oldSpc; i++)
  {
    np0 = 0;
    B_SET_NXP(p, varp->hash, i);
    nx = B_GET_BDDP(*p);
    while(nx != bddnull)
    {
      np = Node + nx;
      f0 = B_GET_BDDP(np->f0);
      f1 = B_GET_BDDP(np->f1);
      key = B_HASHKEY(f0, f1, newSpc);
      if(key == i) np0 = np;
      else
      {
        if(np0) B_CPY_BDDP(np0->nx, np->nx);
        else { B_SET_NXP(p, varp->hash, i); B_CPY_BDDP(*p, np->nx); }
        B_SET_NXP(p, varp->hash, key);
        B_CPY_BDDP(np->nx, *p);
        B_SET_BDDP(*p, nx);
      }
      if(np0) nx = B_GET_BDDP(np0->nx);
      else { B_SET_NXP(p, varp->hash, i); nx = B_GET_BDDP(*p); }
    }
  }
  return 0;
}

static bddp getnode(bddvar v, bddp f0, bddp f1)
/* Returns bddnull if not enough memory */
{
  /* After checking elimination rule & negative edge rule */
  struct B_NodeTable *np, *fp;
  struct B_VarTable *varp;
  bddp ix, nx, key;
  bddp_32 *p_32;
#ifdef B_64
  bddp_h8 *p_h8;
#endif

  varp = &Var[v];
  if(varp->hashSpc == 0)
  /* Create hash-table */
  {
    varp->hash_32 = 0;
    varp->hash_32 = B_MALLOC(bddp_32, B_HASH_SPC0);
    if(!varp->hash_32) return bddnull;
#ifdef B_64
    varp->hash_h8 = 0;
    varp->hash_h8 = B_MALLOC(bddp_h8, B_HASH_SPC0);
    if(!varp->hash_h8)
    {
      free(varp->hash_32);
      return bddnull;
    }
#endif
    for(ix=0; ix<B_HASH_SPC0; ix++)
    {
      B_SET_NXP(p, varp->hash, ix);
      B_SET_BDDP(*p, bddnull);
    }
    varp->hashSpc = B_HASH_SPC0;
    key = B_HASHKEY(f0, f1, varp->hashSpc);
  }
  else
  /* Looking for equivalent existing node */
  {
    key = B_HASHKEY(f0, f1, varp->hashSpc);
    B_SET_NXP(p, varp->hash, key);
    nx = B_GET_BDDP(*p);
    while(nx != bddnull)
    {
      np = Node + nx;
      if(f0 == B_GET_BDDP(np->f0) &&
         f1 == B_GET_BDDP(np->f1) )
      {
        /* Sharing equivalent node */
        if(!B_CST(f0)) { fp = B_NP(f0); B_RFC_DEC_NP(fp); }
        if(!B_CST(f1)) { fp = B_NP(f1); B_RFC_DEC_NP(fp); }
        B_RFC_INC_NP(np);
        return B_BDDP_NP(np);
      }
      nx = B_GET_BDDP(np->nx);
    }
  }

  /* Check hash-table overflow */
  if(++ varp->hashUsed >= varp->hashSpc)
  {
    if(hash_enlarge(v)) return bddnull; /* Hash-table overflow */
    key = B_HASHKEY(f0, f1, varp->hashSpc); /* Enlarge success */
  }

  /* Check node-table overflow */
  if(NodeUsed >= NodeSpc-1U)
  {
    if(node_enlarge())
    {
      if(bddgc()) return bddnull; /* Node-table overflow */
      key = B_HASHKEY(f0, f1, varp->hashSpc);
    }
    /* Node-table enlarged or GC succeeded */
  }
  NodeUsed++;

  /* Creating a new node */
  nx = Avail;
  np = Node + nx;
  Avail = B_GET_BDDP(np->nx);
  B_SET_NXP(p, varp->hash, key);
  B_CPY_BDDP(np->nx, *p);
  B_SET_BDDP(*p, nx);
  B_SET_BDDP(np->f0, f0);
  B_SET_BDDP(np->f1, f1);
  np->varrfc = v;
  B_RFC_INC_NP(np);
  return B_BDDP_NP(np);
}

static bddp getbddp(bddvar v, bddp f0, bddp f1)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp;

  /* Check elimination rule */
  if(f0 == f1)
  {
    if(!B_CST(f0)) { fp = B_NP(f0); B_RFC_DEC_NP(fp); }
    return f0;
  }

  /* Negative edge constraint */
  if(B_NEG(f0))
  {
    bddp h;

    h = getnode(v, B_NOT(f0), B_NOT(f1));
    if(h == bddnull) return bddnull;
    return B_NOT(h);
  }
  return getnode(v, f0, f1);
}

static bddp apply(bddp f, bddp g, unsigned char op, unsigned char skip)
/* Returns bddnull if not enough memory */
{
  struct B_NodeTable *fp, *gp;
  struct B_CacheTable *cachep;
  bddp key, f0, f1, g0, g1, h0, h1, h;
  bddvar v, flev, glev;
  char z; /* flag to check ZBDD node */

  /* Check terminal case */
  if(!skip) switch(op)
  {
  case BC_AND: 
    /* Check trivial cases */
    if(f == bddfalse || g == bddfalse || f == B_NOT(g))
      return bddfalse;
    if(f == g)
    {
      if(f != bddtrue) { fp = B_NP(f); B_RFC_INC_NP(fp); }
      return f;
    }
    if(f == bddtrue) { fp = B_NP(g); B_RFC_INC_NP(fp); return g; }
    if(g == bddtrue) { fp = B_NP(f); B_RFC_INC_NP(fp); return f; }
    /* Check operand swap */
    if(f < g) { h = f; f = g; g = h; } /* swap (f, g) */
    break;

  case BC_XOR:
    /* Check trivial cases */
    if(f == g) return bddfalse;
    if(f == B_NOT(g)) return bddtrue;
    if(f == bddfalse) { fp = B_NP(g); B_RFC_INC_NP(fp); return g; }
    if(g == bddfalse) { fp = B_NP(f); B_RFC_INC_NP(fp); return f; }
    if(f == bddtrue) {fp=B_NP(g); B_RFC_INC_NP(fp); return B_NOT(g);}
    if(g == bddtrue) {fp=B_NP(f); B_RFC_INC_NP(fp); return B_NOT(f);}
    /* Check negation */
    if(B_NEG(f) && B_NEG(g)) { f = B_NOT(f); g = B_NOT(g); }
    else if(B_NEG(f) || B_NEG(g))
    {
      f = B_ABS(f); g = B_ABS(g);
      /* Check operand swap */
      h = (f < g)? apply(g, f, op, 1): apply(f, g, op, 1);
      if(h == bddnull) return bddnull;
      return B_NOT(h);
    }
    /* Check operand swap */
    if(f < g) { h = f; f = g; g = h; } /* swap (f, g) */
    break;

  case BC_COFACTOR: 
    /* Check trivial cases */
    if(B_CST(f)) return f;
    if(g == bddfalse || f == B_NOT(g)) return bddfalse;
    if(f == g) return bddtrue;
    if(g == bddtrue) { fp = B_NP(f); B_RFC_INC_NP(fp); return f; }
    break;
  
  case BC_UNIV: 
    /* Check trivial cases */
    if(B_CST(f)) return f;
    if(B_CST(g)) { fp = B_NP(f); B_RFC_INC_NP(fp); return f; }
    if(B_NEG(g)) g = B_NOT(g);
    break;
  
  case BC_SUPPORT:
    if(B_CST(f)) return bddfalse;
    if(B_NEG(f)) f = B_NOT(f);
    break;

  case BC_INTERSEC: 
    /* Check trivial cases */
    if(f == bddfalse || g == bddfalse) return bddfalse;
    if(f == bddtrue) return B_NEG(g)? bddtrue: bddfalse;
    if(g == bddtrue) return B_NEG(f)? bddtrue: bddfalse;
    if(f == g) { fp = B_NP(f); B_RFC_INC_NP(fp); return f; }
    if(f == B_NOT(g)) {fp=B_NP(f); B_RFC_INC_NP(fp); return B_ABS(f); }
    /* Check operand swap */
    if(f < g) { h = f; f = g; g = h; } /* swap (f, g) */
    break;

  case BC_UNION: 
    /* Check trivial cases */
    if(f == bddfalse)
    {
      if(!B_CST(g)) {fp=B_NP(g); B_RFC_INC_NP(fp); }
      return g;
    }
    if(f == bddtrue)
    {
      if(!B_CST(g)) {fp=B_NP(g); B_RFC_INC_NP(fp); }
      return B_NEG(g)? g: B_NOT(g);
    }
    if(g == bddfalse || f == g)
      { fp=B_NP(f); B_RFC_INC_NP(fp); return f; }
    if(g == bddtrue || f == B_NOT(g))
    {
      fp=B_NP(f); B_RFC_INC_NP(fp);
      return B_NEG(f)? f: B_NOT(f);
    }
    /* Check operand swap */
    if(f < g) { h = f; f = g; g = h; } /* swap (f, g) */
    break;

  case BC_SUBTRACT: 
    /* Check trivial cases */
    if(f == bddfalse || f == g) return bddfalse;
    if(f == bddtrue || f == B_NOT(g))
      return B_NEG(g)? bddfalse: bddtrue;
    if(g == bddfalse) { fp=B_NP(f); B_RFC_INC_NP(fp); return f; }
    if(g == bddtrue) { fp=B_NP(f); B_RFC_INC_NP(fp); return B_ABS(f); }
    break;
  
  case BC_AT0: 
  case BC_AT1: 
  case BC_OFFSET: 
    /* Check trivial cases */
    if(B_CST(f)) return f;
    /* special cases */
    fp = B_NP(f); flev = Var[B_VAR_NP(fp)].lev;
    glev = Var[(bddvar)g].lev;
    if(flev < glev) { B_RFC_INC_NP(fp); return f; }
    if(flev == glev)
    {
      if(op != BC_AT1)
      {
        h = B_GET_BDDP(fp->f0);
        if(B_NEG(f)^B_NEG(h)) h = B_NOT(h);
      }
      else
      {
        h = B_GET_BDDP(fp->f1);
        if(B_NEG(f)) h = B_NOT(h);
      }
      if(!B_CST(h)) { fp = B_NP(h); B_RFC_INC_NP(fp); }
      return h;
    }
    /* Check negation */
    if(B_NEG(f))
    {
      h = apply(B_NOT(f), g, op, 1);
      if(h == bddnull) return bddnull;
      return B_NOT(h);
    }
    break;

  case BC_ONSET: 
    /* Check trivial cases */
    if(B_CST(f)) return bddfalse;
    /* special cases */
    fp = B_NP(f); flev = Var[B_VAR_NP(fp)].lev;
    glev = Var[(bddvar)g].lev;
    if(flev < glev)  return bddfalse;
    if(flev == glev)
    {
      h = B_GET_BDDP(fp->f1);
      if(!B_CST(h)) { fp = B_NP(h); B_RFC_INC_NP(fp); }
      return h;
    }
    /* Check negation */
    if(B_NEG(f)) f = B_NOT(f);
    break;

  case BC_CHANGE: 
    /* Check trivial cases */
    if(f == bddfalse) return f;
    if(B_CST(f)) return getzbddp((bddvar)g, bddfalse, f);
    /* special cases */
    fp = B_NP(f); flev = Var[B_VAR_NP(fp)].lev;
    glev = Var[(bddvar)g].lev;
    if(flev < glev)
    {
      B_RFC_INC_NP(fp);
      h = getzbddp((bddvar)g, bddfalse, f);
      if(h == bddnull) bddfree(f);
      return h;
    }
    if(flev == glev)
    {
      h0 = B_GET_BDDP(fp->f1);
      h1 = B_GET_BDDP(fp->f0);
      if(B_NEG(f)^B_NEG(h1)) h1 = B_NOT(h1);
      if(!B_CST(h0)) { fp = B_NP(h0); B_RFC_INC_NP(fp); }
      if(!B_CST(h1)) { fp = B_NP(h1); B_RFC_INC_NP(fp); }
      h = getzbddp((bddvar)g, h0, h1);
      if(h == bddnull) { bddfree(h0); bddfree(h1); }
      return h;
    }
    break;

  case BC_LSHIFT: 
  case BC_RSHIFT: 
    /* Check trivial cases */
    if(B_CST(f)) return f;

    /* Check negation */
    if(B_NEG(f))
    {
      h = apply(B_NOT(f), g, op, 1);
      if(h == bddnull) return bddnull;
      return B_NOT(h);
    }
    break;

  case BC_CARD:
    if(B_CST(f)) return (f == bddfalse)? 0: 1;
    if(B_NEG(f)) 
    {
      h = apply(B_NOT(f), bddfalse, op, 1);
      return (h >= bddnull)? bddnull: h+1;
    }
    break;

  case BC_CARD2:
    if(B_CST(f)) return (f == bddfalse)? 0: 1;
    break;

  case BC_LIT:
    if(B_CST(f)) return 0;
    if(B_NEG(f)) f = B_NOT(f);
    break;

  case BC_LEN:
    if(B_CST(f)) return 0;
    if(B_NEG(f)) f = B_NOT(f);
    break;

  default:
    err("apply: unknown opcode", op);
    break;
  }

  /* Non-trivial operations */
  switch(op)
  {
  /* binary operation */
  case BC_AND:
  case BC_XOR:
  case BC_COFACTOR:
  case BC_UNIV:
  case BC_INTERSEC:
  case BC_UNION:
  case BC_SUBTRACT:
    /* Try cache? */
    if((B_CST(f) || B_RFC_ONE_NP(B_NP(f))) &&
       (B_CST(g) || B_RFC_ONE_NP(B_NP(g)))) key = bddnull;
    else
    {
      /* Checking Cache */
      key = B_CACHEKEY(op, f, g);
      cachep = Cache + key;
      if(cachep->op == op &&
         f == B_GET_BDDP(cachep->f) &&
         g == B_GET_BDDP(cachep->g))
      {
        /* Hit */
        h = B_GET_BDDP(cachep->h);
        if(!B_CST(h) && h != bddnull) { fp = B_NP(h); B_RFC_INC_NP(fp); }
        return h;
      }
    }
    /* Get (f0, f1) and (g0, g1)*/
    z = 0;
    fp = B_NP(f);
    flev = B_CST(f)? 0: Var[B_VAR_NP(fp)].lev;
    gp = B_NP(g);
    glev = B_CST(g)? 0: Var[B_VAR_NP(gp)].lev;
    f0 = f; f1 = f;
    g0 = g; g1 = g;

    if(flev <= glev)
    {
      v = B_VAR_NP(gp);
      if(B_Z_NP(gp))
      {
        z = 1;
        if(flev < glev) f1 = bddfalse;
      }
      g0 = B_GET_BDDP(gp->f0);
      g1 = B_GET_BDDP(gp->f1);
      if(B_NEG(g)^B_NEG(g0)) g0 = B_NOT(g0);
      if(B_NEG(g) && !z) g1 = B_NOT(g1);
    }

    if(flev >= glev)
    {
      v = B_VAR_NP(fp);
      if(B_Z_NP(fp))
      {
        z = 1;
        if(flev > glev) g1 = bddfalse;
      }
      f0 = B_GET_BDDP(fp->f0);
      f1 = B_GET_BDDP(fp->f1);
      if(B_NEG(f)^B_NEG(f0)) f0 = B_NOT(f0);
      if(B_NEG(f) && !z) f1 = B_NOT(f1);
    }
    break;

  /* unary operation */
  case BC_AT0:
  case BC_AT1:
  case BC_LSHIFT:
  case BC_RSHIFT:
  case BC_SUPPORT:
  case BC_OFFSET:
  case BC_ONSET:
  case BC_CHANGE:
    fp = B_NP(f);
    if(B_RFC_ONE_NP(fp)) key = bddnull;
    else
    {
      /* Checking Cache */
      key = B_CACHEKEY(op, f, g);
      cachep = Cache + key;
      if(cachep->op == op &&
         f == B_GET_BDDP(cachep->f) &&
         g == B_GET_BDDP(cachep->g))
      {
        /* Hit */
        h = B_GET_BDDP(cachep->h);
        if(!B_CST(h) && h != bddnull) { fp = B_NP(h); B_RFC_INC_NP(fp); }
        return h;
      }
    }
    /* Get (f0, f1)*/
    v = B_VAR_NP(fp);
    z = B_Z_NP(fp)? 1: 0;
    f0 = B_GET_BDDP(fp->f0);
    f1 = B_GET_BDDP(fp->f1);
    if(B_NEG(f)^B_NEG(f0)) f0 = B_NOT(f0);
    if(B_NEG(f) && !z) f1 = B_NOT(f1);
    break;

  case BC_CARD:
  case BC_LIT:
  case BC_LEN:
    fp = B_NP(f);
    if(B_RFC_ONE_NP(fp)) key = bddnull;
    else
    {
      /* Checking Cache */
      key = B_CACHEKEY(op, f, bddfalse);
      cachep = Cache + key;
      if(cachep->op == op &&
         f == B_GET_BDDP(cachep->f) &&
         bddfalse == B_GET_BDDP(cachep->g))
      {
        /* Hit */
        return B_GET_BDDP(cachep->h);
      }
    }
    /* Get (f0, f1)*/
    f0 = B_GET_BDDP(fp->f0);
    f1 = B_GET_BDDP(fp->f1);
    if(B_NEG(f)^B_NEG(f0)) f0 = B_NOT(f0);
    break;

  case BC_CARD2:
    fp = B_NP(f);
    if(B_RFC_ONE_NP(fp)) key = bddnull;
    else
    {
      /* Checking Cache */
      key = B_CACHEKEY(BC_CARD, f, bddfalse);
      cachep = Cache + key;
      if(cachep->op == BC_CARD &&
         f == B_GET_BDDP(cachep->f) &&
         bddfalse == B_GET_BDDP(cachep->g))
      {
        /* Hit */
        h = B_GET_BDDP(cachep->h);
	if(h != bddnull) return h;
      }
    }
    /* Get (f0, f1)*/
    f0 = B_GET_BDDP(fp->f0);
    f1 = B_GET_BDDP(fp->f1);
    if(B_NEG(f)^B_NEG(f0)) f0 = B_NOT(f0);
    break;

  default:
    err("apply: unknown opcode", op);
  }

  /* Stack overflow limitter */
  BDD_RECUR_INC;

  /* Get result node */
  switch(op)
  {
  case BC_AND:
  case BC_XOR:
  case BC_INTERSEC:
  case BC_UNION:
  case BC_SUBTRACT:
    h0 = apply(f0, g0, op, 0);
    if(h0 == bddnull) { h = h0; break; } /* Overflow */
    h1 = apply(f1, g1, op, 0);
    if(h1 == bddnull) { bddfree(h0); h = h1; break; } /* Overflow */
    h = z? getzbddp(v, h0, h1): getbddp(v, h0, h1);
    if(h == bddnull) { bddfree(h0); bddfree(h1); } /* Overflow */
    break;
  
  case BC_COFACTOR:
    if(g0 == bddfalse && g1 != bddfalse)
    {
      h = apply(f1, g1, op, 0);
    }
    else if(g1 == bddfalse && g0 != bddfalse)
    {
      h = apply(f0, g0, op, 0);
    }
    else
    {
      h0 = apply(f0, g0, op, 0);
      if(h0 == bddnull) { h = h0; break; } /* Overflow */
      h1 = apply(f1, g1, op, 0);
      if(h1 == bddnull) { bddfree(h0); h = h1; break; } /* Overflow */
      h = getbddp(v, h0, h1);
      if(h == bddnull) { bddfree(h0); bddfree(h1); } /* Overflow */
    }
    break;

  case BC_UNIV:
    if(g0 != g1)
    {
      h0 = apply(f0, g0, op, 0);
      if(h0 == bddnull) { h = h0; break; } /* Overflow */
      h1 = apply(f1, g0, op, 0);
      if(h1 == bddnull) { bddfree(h0); h = h1; break; } /* Overflow */
      h = apply(h0, h1, BC_AND, 0);
      bddfree(h0); bddfree(h1);
    }
    else
    {
      h0 = apply(f0, g0, op, 0);
      if(h0 == bddnull) { h = h0; break; } /* Overflow */
      h1 = apply(f1, g0, op, 0);
      if(h1 == bddnull) { bddfree(h0); h = h1; break; } /* Overflow */
      h = getbddp(v, h0, h1);
      if(h == bddnull) { bddfree(h0); bddfree(h1); } /* Overflow */
    }
    break;

  case BC_AT0:
  case BC_AT1:
  case BC_OFFSET:
  case BC_ONSET:
  case BC_CHANGE:
    h0 = apply(f0, g, op, 0);
    if(h0 == bddnull) { h = h0; break; } /* Overflow */
    h1 = apply(f1, g, op, 0);
    if(h1 == bddnull) { bddfree(h0); h = h1; break; } /* Overflow */
    h = z? getzbddp(v, h0, h1): getbddp(v, h0, h1);
    if(h == bddnull) { bddfree(h0); bddfree(h1); } /* Overflow */
    break;

  case BC_SUPPORT:
    h0 = apply(f0, bddfalse, op, 0);
    if(h0 == bddnull) { h = h0; break; } /* Overflow */
    h1 = apply(f1, bddfalse, op, 0);
    if(h1 == bddnull) { bddfree(h0); h = h1; break; } /* Overflow */
    h = z? apply(h0, h1, BC_UNION, 0):
           apply(B_NOT(h0), B_NOT(h1), BC_AND, 0);
    bddfree(h0); bddfree(h1);
    if(h == bddnull) break; /* Overflow */
    h0 = h;
    h = z? getzbddp(v, h0, bddtrue):
           getbddp(v, B_NOT(h0), bddtrue);
    if(h == bddnull) bddfree(h0); /* Overflow */
    break;

  case BC_LSHIFT:
  case BC_RSHIFT:
    /* Get VarID of new level */
    {
      bddvar newlev; 
  
      flev = bddlevofvar(v);
      if(op == BC_LSHIFT)
      {
        newlev = flev + (bddvar)g;
        if(newlev > VarUsed || newlev < flev)
          err("apply: Invald shift", newlev);
      }
      else
      {
        newlev = flev - (bddvar)g;
        if(newlev == 0 || newlev > flev)
          err("apply: Invald shift", newlev);
      }
      v = bddvaroflev(newlev);
    }
    h0 = apply(f0, g, op, 0);
    if(h0 == bddnull) { h = h0; break; } /* Overflow */
    h1 = apply(f1, g, op, 0);
    if(h1 == bddnull) { bddfree(h0); h = h1; break; } /* Overflow */
    h = z? getzbddp(v, h0, h1): getbddp(v, h0, h1);
    if(h == bddnull) { bddfree(h0); bddfree(h1); } /* Overflow */
    break;
    
  case BC_CARD:
    h0 = apply(f0, bddfalse, op, 0);
    if(h0 == bddnull) { h = h0; break; }
    h1 = apply(f1, bddfalse, op, 0);
    if(h1 == bddnull) { h = h1; break; }
    h = h0 + h1;
    if(h >= bddnull) h = bddnull;
    break;

  case BC_CARD2:
    h0 = apply(B_ABS(f0), bddfalse, op, 0);
    if(h0 == B_MP_NULL) { h = h0; break; }
    h1 = apply(B_ABS(f1), bddfalse, op, 0);
    if(h1 == B_MP_NULL) { h = h1; break; }
    {
      struct B_MP mp;
      struct B_MPTable *mpt;
      bddp i, size2;
      bddp *wp;

      mp.len = 1;
      mp.word[0] = 0;
      if(B_NEG(f0)) mp.word[0]++;
      if(B_NEG(f1)) mp.word[0]++;
      mp_add(&mp, h0);
      mp_add(&mp, h1);
      if(mp.len == 1 && mp.word[0] <= bddnull)
        { h = mp.word[0]; break; }
      mpt = mptable + mp.len-1;
      if(mpt->word == 0)
      {
        mpt->size = 16;
        mpt->used = 0;
        mpt->word = B_MALLOC(bddp, mp.len * mpt->size);
      }
      if(mpt->size == mpt->used)
      {
        size2 = mpt->size << 1;
	if(size2 > (B_CST_MASK>>B_MP_LWID)) { h = B_MP_NULL; break; }
	wp = 0;
        wp = B_MALLOC(bddp, mp.len * size2);
	if(!wp) { h = B_MP_NULL; break; }
	for(i=0; i<mp.len*(mpt->size); i++) wp[i] = mpt->word[i];
        mpt->size = size2;
	free(mpt->word);
	mpt->word = wp;
      }
      wp = mpt->word;
        
      for(i=0; i<(bddp)mp.len; i++) wp[mp.len*(mpt->used)+i] = mp.word[i];
      h = (((bddp)mp.len-1)<<B_MP_LPOS) + B_CST_MASK + (mpt->used++);
      break;
    }
  case BC_LIT:
    h = apply(f0, bddfalse, op, 0)
      + apply(f1, bddfalse, op, 0);
    if(h >= bddnull) h = bddnull;
    h += apply(f1, bddfalse, BC_CARD, 0);
    if(h >= bddnull) h = bddnull;
    break;

  case BC_LEN:
    h0 = apply(f0, bddfalse, op, 0);
    h1 = apply(f1, bddfalse, op, 0) + 1;
    h = (h0 < h1)? h1: h0;
    break;

  default:
    err("apply: unknown opcode", op);
    break;
  }

  /* Stack overflow limitter */
  BDD_RECUR_DEC;

  /* Saving to Cache */
  if(key != bddnull)
  {
    cachep = Cache + key;
    cachep->op = op;
    if(op == BC_CARD2) cachep->op = BC_CARD;
    B_SET_BDDP(cachep->f, f);
    B_SET_BDDP(cachep->g, g);
    B_SET_BDDP(cachep->h, h);
    if(h == f) switch(op)
    {
    case BC_AT0:
      key = B_CACHEKEY(BC_AT1, f, g);
      cachep = Cache + key;
      cachep->op = BC_AT1;
      B_SET_BDDP(cachep->f, f);
      B_SET_BDDP(cachep->g, g);
      B_SET_BDDP(cachep->h, h);
      break;
    case BC_AT1:
      key = B_CACHEKEY(BC_AT0, f, g);
      cachep = Cache + key;
      cachep->op = BC_AT0;
      B_SET_BDDP(cachep->f, f);
      B_SET_BDDP(cachep->g, g);
      B_SET_BDDP(cachep->h, h);
      break;
    case BC_OFFSET:
      key = B_CACHEKEY(BC_ONSET, f, g);
      cachep = Cache + key;
      cachep->op = BC_ONSET;
      B_SET_BDDP(cachep->f, f);
      B_SET_BDDP(cachep->g, g);
      B_SET_BDDP(cachep->h, bddfalse);
      break;
    default:
      break;
    }
    if(h == bddfalse && op == BC_ONSET)
    {
      key = B_CACHEKEY(BC_OFFSET, f, g);
      cachep = Cache + key;
      cachep->op = BC_OFFSET;
      B_SET_BDDP(cachep->f, f);
      B_SET_BDDP(cachep->g, g);
      B_SET_BDDP(cachep->h, f);
    }
  }
  return h;
}

static void gc1(struct B_NodeTable *np)
{
  /* np is a node ptr to be collected. (refc == 0) */
  bddp key, nx1, f0, f1;
  struct B_VarTable *varp;
  struct B_NodeTable *np1, *np2;
  bddp_32 *p_32;
#ifdef B_64
  bddp_h8 *p_h8;
#endif

  /* remove the node from hash list */
  varp = Var + B_VAR_NP(np);
  f0 = B_GET_BDDP(np->f0);
  f1 = B_GET_BDDP(np->f1);
  key = B_HASHKEY(f0, f1, varp->hashSpc);
  B_SET_NXP(p, varp->hash, key);
  nx1 = B_GET_BDDP(*p);
  np1 = Node + nx1;

  if(np1 == np) B_CPY_BDDP(*p, np->nx);
  else
  {
    while(np1 != np)
    {
      if(nx1 == bddnull)
        err("gc1: Fail to find the node to be deleted", np-Node);
      np2 = np1;
      nx1 = B_GET_BDDP(np2->nx);
      np1 = Node + nx1;
    }
    B_CPY_BDDP(np2->nx, np->nx);
  }
  varp->hashUsed--;

  /* append the node to avail list */
  B_SET_BDDP(np->nx, Avail);
  Avail = np - Node;

  NodeUsed--;
  np->varrfc = 0;

  /* Check sub-graphs recursively */
  if(!B_CST(f0))
  {
    np1 = B_NP(f0);
    B_RFC_DEC_NP(np1);
    if(B_RFC_ZERO_NP(np1))
    {  BDD_RECUR_INC; gc1(np1); BDD_RECUR_DEC; }
  }
  if(!B_CST(f1))
  {
    np1 = B_NP(f1);
    B_RFC_DEC_NP(np1);
    if(B_RFC_ZERO_NP(np1))
    {  BDD_RECUR_INC; gc1(np1); BDD_RECUR_DEC; }
  }
}

static bddp count(bddp f)
{
  bddp nx;
  bddp c;
  struct B_NodeTable *fp;

  /* Check consistensy
  if(f == bddnull)
    err("count: bddnull found", bddnull);
  */

  if(B_CST(f)) return 0; /* Constant */
  fp = B_NP(f);

  /* Check visit flag */
  nx = B_GET_BDDP(fp->nx);
  if(nx & B_CST_MASK) return 0;

  /* Check consistensy
  flev = Var[B_VAR_NP(fp)].lev;
  g = B_GET_BDDP(fp->f0);
  if(!B_CST(g))
  {
    gp = B_NP(g); glev = Var[B_VAR_NP(gp)].lev;
    if(flev <= glev) 
        err("count: inconsistensy found at f0", fp-Node);
  }
  g = B_GET_BDDP(fp->f1);
  if(!B_CST(g))
  {
    gp = B_NP(g); glev = Var[B_VAR_NP(gp)].lev;
    if(flev <= glev) 
        err("count: inconsistensy found at f1", fp-Node);
  }
  */

  BDD_RECUR_INC;
  c = count(B_GET_BDDP(fp->f0)) + count(B_GET_BDDP(fp->f1)) + 1U ;
  BDD_RECUR_DEC;

  /* Set visit flag */
  B_SET_BDDP(fp->nx, nx | B_CST_MASK);

  return c;
}

static void export_static(FILE *strm, bddp f)
{
  bddp nx, f0, f1;
  bddvar v;
  struct B_NodeTable *fp;

  if(B_CST(f)) return; /* Constant */
  fp = B_NP(f);

  /* Check visit flag */
  nx = B_GET_BDDP(fp->nx);
  if(nx & B_CST_MASK) return;

  /* Set visit flag */
  B_SET_BDDP(fp->nx, nx | B_CST_MASK);

  /* Dump its subgraphs recursively */
  v = B_VAR_NP(fp);
  f0 = B_GET_BDDP(fp->f0);
  f0 = B_ABS(f0);
  f1 = B_GET_BDDP(fp->f1);
  BDD_RECUR_INC;
  export_static(strm, f0);
  export_static(strm, f1);
  BDD_RECUR_DEC;

  /* Dump this node */
  fprintf(strm, B_BDDP_FD, B_ABS(f));
  fprintf(strm, " %d ", Var[v].lev);
  if(f0 == bddfalse) fprintf(strm, "F");
  else if(f0 == bddtrue) fprintf(strm, "T");
  else fprintf(strm, B_BDDP_FD, f0); 
  fprintf(strm, " ");
  if(f1 == bddfalse) fprintf(strm, "F");
  else if(f1 == bddtrue) fprintf(strm, "T");
  else fprintf(strm, B_BDDP_FD, f1);
  fprintf(strm, "\n");
}

static void dump(bddp f)
{
  bddp nx, f0, f1;
  bddvar v;
  struct B_NodeTable *fp;

  if(B_CST(f)) return; /* Constant */
  fp = B_NP(f);

  /* Check visit flag */
  nx = B_GET_BDDP(fp->nx);
  if(nx & B_CST_MASK) return;

  /* Set visit flag */
  B_SET_BDDP(fp->nx, nx | B_CST_MASK);

  /* Dump its subgraphs recursively */
  v = B_VAR_NP(fp);
  f0 = B_GET_BDDP(fp->f0);
  f0 = B_ABS(f0);
  f1 = B_GET_BDDP(fp->f1);
  BDD_RECUR_INC;
  dump(f0);
  dump(f1);
  BDD_RECUR_DEC;

  /* Dump this node */
  printf("N");
  printf(B_BDDP_FD, B_NDX(f));
  printf(" = [V%d(%d), ", v, Var[v].lev);
  if(B_CST(f0)) printf(B_BDDP_FD, B_VAL(f0));
  else { printf("N"); printf(B_BDDP_FD, B_NDX(f0)); }
  printf(", ");
  if(B_NEG(f1)) putchar('~'); 
  if(B_CST(f1)) printf(B_BDDP_FD, B_ABS(B_VAL(f1)));
  else { printf("N"); printf(B_BDDP_FD, B_NDX(f1)); }
  printf("]");
  if(B_Z_NP(fp)) printf(" #Z");
  printf("\n");
}

static void reset(bddp f)
{
  bddp nx;
  struct B_NodeTable *fp;

  if(B_CST(f)) return; /* Constant */
  fp = B_NP(f);

  /* Check visit flag */
  nx = B_GET_BDDP(fp->nx);
  if(nx & B_CST_MASK)
  {
    /* Reset visit flag */
    B_SET_BDDP(fp->nx, nx & ~B_CST_MASK);
    BDD_RECUR_INC;
    reset(B_GET_BDDP(fp->f0));
    reset(B_GET_BDDP(fp->f1));
    BDD_RECUR_DEC;
  }
}

static bddp getzbddp(bddvar v, bddp f0, bddp f1)
/* Returns bddnull if not enough memory */
{
  /* Check elimination rule */
  if(f1 == bddfalse) return f0;

  /* Negative edge constraint */
  if(B_NEG(f0))
  {
    bddp h;

    h = getnode(v, f0, f1);
    if(h == bddnull) return bddnull;
    return B_NOT(h);
  }
  return getnode(v, B_NOT(f0), f1);
}

static int andfalse(bddp f, bddp g)
{
  struct B_NodeTable *fp, *gp;
  struct B_CacheTable *cachep;
  bddp key, f0, f1, g0, g1, h;
  bddvar flev, glev;

  /* Check trivial cases */
  if(f == bddfalse || g == bddfalse || f == B_NOT(g)) return 0;
  if(f == bddtrue || g == bddtrue || f == g) return 1;
  /* Check operand swap */
  if(f > g) { h = f; f = g; g = h; } /* swap (f, g) */

  /* Non-trivial operations */
  /* Try cache? */
  if((B_CST(f) || B_RFC_ONE_NP(B_NP(f))) &&
     (B_CST(g) || B_RFC_ONE_NP(B_NP(g)))) key = bddnull;
  else
  {
    /* Checking Cache */
    key = B_CACHEKEY(BC_AND, f, g);
    cachep = Cache + key;
    if(cachep->op == BC_AND &&
       f == B_GET_BDDP(cachep->f) &&
       g == B_GET_BDDP(cachep->g))
    {
      /* Hit */
      h = B_GET_BDDP(cachep->h);
      return (h==bddfalse)? 0: 1;
    }
  }
  /* Get (f0, f1) and (g0, g1)*/
  fp = B_NP(f);
  flev = B_CST(f)? 0: Var[B_VAR_NP(fp)].lev;
  gp = B_NP(g);
  glev = B_CST(g)? 0: Var[B_VAR_NP(gp)].lev;
  f0 = f; f1 = f;
  g0 = g; g1 = g;

  if(flev <= glev)
  {
    g0 = B_GET_BDDP(gp->f0);
    g1 = B_GET_BDDP(gp->f1);
    if(B_NEG(g)) { g0 = B_NOT(g0); g1 = B_NOT(g1); }
  }

  if(flev >= glev)
  {
    f0 = B_GET_BDDP(fp->f0);
    f1 = B_GET_BDDP(fp->f1);
    if(B_NEG(f)) { f0 = B_NOT(f0); f1 = B_NOT(f1); }
  }

  /* Get result */
  if(andfalse(f0, g0) == 1) return 1;
  if(andfalse(f1, g1) == 1) return 1;

  /* Saving to Cache */
  if(key != bddnull)
  {
    cachep = Cache + key;
    cachep->op = BC_AND;
    B_SET_BDDP(cachep->f, f);
    B_SET_BDDP(cachep->g, g);
    B_SET_BDDP(cachep->h, bddfalse);
  }
  return 0;
}

static int err(const char *msg, bddp num)
{
  fprintf(stderr,"***** ERROR  %s ( ", msg);
  fprintf(stderr, B_BDDP_FX, num);
  fprintf(stderr," ) *****\n");
  fprintf(stderr," NodeLimit : ");
  fprintf(stderr, B_BDDP_FD, NodeLimit);
  fprintf(stderr,"\t NodeSpc : ");
  fprintf(stderr, B_BDDP_FD, NodeSpc);
  fprintf(stderr,"\t VarSpc : %d",VarSpc);
  fprintf(stderr,"\n CacheSpc : ");
  fprintf(stderr, B_BDDP_FD, CacheSpc);
  fprintf(stderr,"\t NodeUsed : ");
  fprintf(stderr, B_BDDP_FD, NodeUsed);
  fprintf(stderr,"\t VarUsed : %d\n",VarUsed);
  exit(1);
  return 1;
}

static int rfc_inc_ovf(struct B_NodeTable *np)
{
  bddp ix, nx, nx2, key, rfc, oldSpc;
  struct B_RFC_Table *oldRFCT;

/* printf("rfc_inc %d (u:%d)\n", np-Node, RFCT_Used); */
  if(RFCT_Spc == 0)
  {
    /* Create RFC-table */
    RFCT = 0;
    RFCT = B_MALLOC(struct B_RFC_Table, B_RFCT_SPC0);
    if(!RFCT)
    {
      err("B_RFC_INC_NP: rfc memory over flow", np-Node);
      return 1;
    }
    for(ix=0; ix<B_RFCT_SPC0; ix++)
    {
      B_SET_BDDP((RFCT+ix)->nx, bddnull);
      B_SET_BDDP((RFCT+ix)->rfc, (bddp)0);
    }
    RFCT_Spc = B_RFCT_SPC0;
  }

  nx = np - Node;
  key = nx & (RFCT_Spc-1);
  nx2 = B_GET_BDDP((RFCT+key)->nx);
  while(nx2 != bddnull)
  {
    if(nx == nx2)
    {
      if(np->varrfc < B_RFC_MASK)
      {
        rfc = 0;
	np->varrfc += B_RFC_UNIT;
      }
      else rfc = B_GET_BDDP((RFCT+key)->rfc) + 1;
      B_SET_BDDP((RFCT+key)->rfc, rfc);
      return 0;
    }
    key = (key+1) & (RFCT_Spc-1);
    nx2 = B_GET_BDDP((RFCT+key)->nx);
  }

  /* new rfc entry */
  B_SET_BDDP((RFCT+key)->nx, nx);
  B_SET_BDDP((RFCT+key)->rfc, (bddp)0);
  np->varrfc += B_RFC_UNIT;
  RFCT_Used++;

  if((RFCT_Used << 1) >= RFCT_Spc)
  {
    /* Enlarge RFC-table */
    oldSpc = RFCT_Spc;
    RFCT_Spc <<= 2;

    oldRFCT = RFCT;
    RFCT = 0;
    RFCT = B_MALLOC(struct B_RFC_Table, RFCT_Spc);
    if(!RFCT)
    {
      err("B_RFC_INC_NP: rfc memory over flow", np-Node);
      return 1;
    }
    for(ix=0; ix<RFCT_Spc; ix++)
    {
      B_SET_BDDP((RFCT+ix)->nx, bddnull);
      B_SET_BDDP((RFCT+ix)->rfc, (bddp)0);
    }
    for(ix=0; ix<oldSpc; ix++)
    {
      nx = B_GET_BDDP((oldRFCT+ix)->nx);
      if(nx == bddnull) continue;
      key = nx & (RFCT_Spc-1);
      nx2 = B_GET_BDDP((RFCT+key)->nx);
      while(nx2 != bddnull)
      {
        key = (key+1) & (RFCT_Spc-1);
        nx2 = B_GET_BDDP((RFCT+key)->nx);
      }
      B_SET_BDDP((RFCT+key)->nx, nx);
      rfc = B_GET_BDDP((oldRFCT+ix)->rfc);
      B_SET_BDDP((RFCT+key)->rfc, rfc);
    }
    free(oldRFCT);
  }

  return 0;
}

static int rfc_dec_ovf(struct B_NodeTable *np)
{
  bddp nx, key, nx2, rfc;

/* printf("rfc_dec %d (u:%d)\n", np-Node, RFCT_Used); */
  nx = np - Node;
  key = nx & (RFCT_Spc-1);
  nx2 = B_GET_BDDP((RFCT+key)->nx);
  while(nx2 != bddnull)
  {
    if(nx == nx2)
    {
      rfc = B_GET_BDDP((RFCT+key)->rfc);
      if(rfc == 0)
      {
        np->varrfc -= B_RFC_UNIT;
        return 0;
      }
      B_SET_BDDP((RFCT+key)->rfc, rfc-1);
      return 0;
    }
    key = (key+1) & (RFCT_Spc-1);
    nx2 = B_GET_BDDP((RFCT+key)->nx);
  }
  return 0;
}

#define IMPORTHASH(x) ((((x)>>1)^((x)<<8)^((x)<<16)) & (hashsize-1))

int import(FILE *strm, bddp *p, int lim, int z)
{
  int n, m, v, i, lev, var, inv, e;
  bddp n_nd, ix, f, f0, f1, nd, nd0, nd1, hashsize, ixx;
  char s[256];
  bddp *hash1;
  bddp *hash2;

  v = fscanf(strm, "%s", s);
  if(v == EOF) return 1;
  if(strcmp(s, "_i") != 0) return 1;
  v = fscanf(strm, "%s", s);
  if(v == EOF) return 1;
  n = strtol(s, 0, 10);
  while(n > (int)bddvarused()) bddnewvar();

  v = fscanf(strm, "%s", s);
  if(v == EOF) return 1;
  if(strcmp(s, "_o") != 0) return 1;
  v = fscanf(strm, "%s", s);
  if(v == EOF) return 1;
  m = strtol(s, 0, 10);

  v = fscanf(strm, "%s", s);
  if(v == EOF) return 1;
  if(strcmp(s, "_n") != 0) return 1;
  v = fscanf(strm, "%s", s);
  if(v == EOF) return 1;
  n_nd = B_STRTOI(s, 0, 10);

  for(hashsize = 1; hashsize < (n_nd<<1); hashsize <<= 1)
    ; /* empty */
  hash1 = 0;
  hash1 = B_MALLOC(bddp, hashsize);
  if(hash1 == 0) return 1;
  hash2 = 0;
  hash2 = B_MALLOC(bddp, hashsize);
  if(hash2 == 0)
  {
    free(hash1);
    return 1;
  }
  for(ix=0; ix<hashsize; ix++) hash1[ix] = bddnull;

  e = 0;
  for(ix=0; ix<n_nd; ix++)
  {
    v = fscanf(strm, "%s", s);
    if(v == EOF) { e = 1; break; }
    nd = B_STRTOI(s, 0, 10);
    
    v = fscanf(strm, "%s", s);
    if(v == EOF) { e = 1; break; }
    lev = strtol(s, 0, 10);
    var = bddvaroflev(lev);

    v = fscanf(strm, "%s", s);
    if(v == EOF) { e = 1; break; }
    if(strcmp(s, "F") == 0) f0 = bddfalse;
    else if(strcmp(s, "T") == 0) f0 = bddtrue;
    else
    {
      nd0 = B_STRTOI(s, 0, 10);

      ixx = IMPORTHASH(nd0);
      while(hash1[ixx] != nd0)
      {
        if(hash1[ixx] == bddnull)
        {
          err("bddimport: internal error", ixx);
	  return 1;
        }
        ixx++;
        ixx &= (hashsize-1);
      }
      f0 = bddcopy(hash2[ixx]);
    }

    v = fscanf(strm, "%s", s);
    if(v == EOF) { e = 1; bddfree(f0); break; }
    if(strcmp(s, "F") == 0) f1 = bddfalse;
    else if(strcmp(s, "T") == 0) f1 = bddtrue;
    else
    {
      nd1 = B_STRTOI(s, 0, 10);
      if(nd1 & 1) { inv = 1; nd1 ^= 1; }
      else inv = 0;
  
      ixx = IMPORTHASH(nd1);
      while(hash1[ixx] != nd1)
      {
        if(hash1[ixx] == bddnull)
        {
          err("bddimport: internal error", ixx);
	  return 1;
        }
        ixx++;
        ixx &= (hashsize-1);
      }
      f1 = (inv)? bddnot(hash2[ixx]): bddcopy(hash2[ixx]);
    }

    f = (z)? getzbddp(var, f0, f1): getbddp(var, f0, f1);
    if(f == bddnull)
    {
      e = 1;
      bddfree(f1);
      bddfree(f0);
      break;
    }

    ixx = IMPORTHASH(nd);
    while(hash1[ixx] != bddnull)
    {
      if(hash1[ixx] == nd)
      {
        err("bddimport: internal error", ixx);
        return 1;
      }
      ixx++;
      ixx &= (hashsize-1);
    }
    hash1[ixx] = nd;
    hash2[ixx] = f;
  }

  if(e)
  {
    for(ix=0; ix<hashsize; ix++)
      if(hash1[ix] != bddnull) bddfree(hash2[ix]);
    free(hash2);
    free(hash1);
    return 1;
  }

  for(i=0; i<m; i++)
  {
    if(i >= lim) break;
    v = fscanf(strm, "%s", s);
    if(v == EOF)
    {
      for(i--; i>=0; i--) bddfree(p[i]);
      for(ix=0; ix<hashsize; ix++)
        if(hash1[ix] != bddnull) bddfree(hash2[ix]);
      free(hash2);
      free(hash1);
      return 1;
    }
    nd = B_STRTOI(s, 0, 10);
    if(strcmp(s, "F") == 0) p[i] = bddfalse;
    else if(strcmp(s, "T") == 0) p[i] = bddtrue;
    else
    {
      if(nd & 1) { inv = 1; nd ^= 1; }
      else inv = 0;
  
      ixx = IMPORTHASH(nd);
      while(hash1[ixx] != nd)
      {
        if(hash1[ixx] == bddnull)
        {
          err("bddimport: internal error", ixx);
          return 1;
        }
        ixx++;
        ixx &= (hashsize-1);
      }
      p[i] = (inv)? bddnot(hash2[ixx]): bddcopy(hash2[ixx]);
    }
  }
  if(i < lim) p[i] = bddnull;

  /* clear hash table */
  for(ix=0; ix<hashsize; ix++)
    if(hash1[ix] != bddnull) bddfree(hash2[ix]);
  free(hash2);
  free(hash1);

  return 0;
}

int mp_add(struct B_MP *p, bddp ix)
{
  int len, i;
  bddp c, *wp;

  if(ix == B_MP_NULL) return 1;
  len = B_MP_LEN(ix);
  if(len) wp = mptable[len-1].word+(B_MP_VAL(ix)*len);
  else { wp = &ix; len = 1; }
  while(p->len < len) p->word[p->len++] = 0;

  c = 0;
  for(i=0; i<p->len; i++)
  {
    p->word[i] += c;
    c = (p->word[i] >= c)? 0: 1;
    if(i < len)
    {
      p->word[i] += wp[i];
      c = (p->word[i] >= wp[i])? c: 1;
    }
  }
  if(c)
  {
    if(p->len == B_MP_LMAX)
    {
      for(i=0; i<p->len; i++) p->word[i] = ~((bddp)0);
      return 1;
    }
    p->word[p->len++] = c;
  }
  return 0;
}

