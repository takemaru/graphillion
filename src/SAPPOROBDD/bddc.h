/*****************************************
*  BDD Package (SAPPORO-1.94)   - Header *
*  (C) Shin-ichi MINATO  (Apr. 19, 2022)  *
******************************************/

#ifndef bddc_h
#define bddc_h

#if (defined BDD_CPP)||(! defined B_OLDC)
#  define B_ARG(a) a       /* ANSI C style */
#else
#  define B_ARG(a) ()      /* K&R C style */
#endif

/***************** Internal macro for index *****************/
#define B_VAR_WIDTH 16U  /* Width of variable index */
#define B_VAR_MASK       ((1U << B_VAR_WIDTH) - 1U)

/***************** Internal macro for bddp *****************/
#ifdef B_64
#  define B_MSB_POS   39ULL
#  define B_LSB_MASK  1ULL
#else
#  define B_MSB_POS   31U
#  define B_LSB_MASK  1U
#endif
#define B_MSB_MASK  (B_LSB_MASK << B_MSB_POS)
#define B_INV_MASK  B_LSB_MASK /* Mask of inverter-flag */
#define B_CST_MASK  B_MSB_MASK /* Mask of constant-flag */
#define B_VAL_MASK  (B_MSB_MASK - 1U)
                      /* Mask of value-field */

/***************** External typedef *****************/
typedef unsigned int bddvar;
#ifdef B_64
  typedef unsigned long long bddp;
#else
  typedef unsigned int bddp;
#endif

/***************** External Macro *****************/
#define bddvarmax B_VAR_MASK /* Max value of variable index */
#define bddnull   B_VAL_MASK /* Special value for null pointer */
#define bddfalse  B_CST_MASK /* bddp of constant false (0) */
#define bddtrue   (bddfalse ^ B_INV_MASK)
                    /* bddp of constant true (1) */
#define bddempty  bddfalse /* bddp of empty ZBDD (0) */
#define bddsingle bddtrue  /* bddp of single unit ZBDD (1) */
#define bddconst(c) (((c) & B_VAL_MASK) | B_CST_MASK)
                    /* bddp of a constant valued node */
#define bddvalmax B_VAL_MASK  /* Max constant value */



/***************** For stack overflow limit *****************/
extern const int BDD_RecurLimit;
extern int BDD_RecurCount;
 
/***************** External operations *****************/

/***************** Init. and config. ****************/
extern int    bddinit B_ARG((bddp initsize, bddp limitsize));
extern bddvar bddnewvar B_ARG((void));
extern bddvar bddnewvaroflev B_ARG((bddvar lev));
extern bddvar bddlevofvar B_ARG((bddvar v));
extern bddvar bddvaroflev B_ARG((bddvar lev));
extern bddvar bddvarused B_ARG((void));

/************** Basic logic operations *************/
extern bddp   bddprime B_ARG((bddvar v));
extern bddvar bddtop B_ARG((bddp f));
extern bddp   bddcopy B_ARG((bddp f));
extern bddp   bddnot B_ARG((bddp f));
extern bddp   bddand B_ARG((bddp f, bddp g));
extern bddp   bddor B_ARG((bddp f, bddp g));
extern bddp   bddxor B_ARG((bddp f, bddp g));
extern bddp   bddnand B_ARG((bddp f, bddp g));
extern bddp   bddnor B_ARG((bddp f, bddp g));
extern bddp   bddxnor B_ARG((bddp f, bddp g));
extern bddp   bddat0 B_ARG((bddp f, bddvar v));
extern bddp   bddat1 B_ARG((bddp f, bddvar v));

/********** Memory management and observation ***********/
extern void   bddfree B_ARG((bddp f));
extern bddp   bddused B_ARG((void));
extern int    bddgc B_ARG((void));
extern bddp   bddsize B_ARG((bddp f));
extern bddp   bddvsize B_ARG((bddp *p, int lim));
extern void   bddexport B_ARG((FILE *strm, bddp *p, int lim));
extern int    bddimport B_ARG((FILE *strm, bddp *p, int lim));
extern void   bdddump B_ARG((bddp f));
extern void   bddvdump B_ARG((bddp *p, int lim));
extern void   bddgraph B_ARG((bddp f));
extern void   bddgraph0 B_ARG((bddp f));
extern void   bddvgraph B_ARG((bddp *p, int lim));
extern void   bddvgraph0 B_ARG((bddp *p, int lim));

/************** Advanced logic operations *************/
extern bddp   bddlshift B_ARG((bddp f, bddvar shift));
extern bddp   bddrshift B_ARG((bddp f, bddvar shift));
extern bddp   bddsupport B_ARG((bddp f));
extern bddp   bdduniv B_ARG((bddp f, bddp g));
extern bddp   bddexist B_ARG((bddp f, bddp g));
extern bddp   bddcofactor B_ARG((bddp f, bddp g));
extern int    bddimply B_ARG((bddp f, bddp g));
extern bddp   bddrcache B_ARG((unsigned char op, bddp f, bddp g));
extern void   bddwcache
              B_ARG((unsigned char op, bddp f, bddp g, bddp h));

/************** ZBDD operations *************/
extern bddp   bddoffset B_ARG((bddp f, bddvar v));
extern bddp   bddonset B_ARG((bddp f, bddvar v));
extern bddp   bddonset0 B_ARG((bddp f, bddvar v));
extern bddp   bddchange B_ARG((bddp f, bddvar v));
extern bddp   bddintersec B_ARG((bddp f, bddp g));
extern bddp   bddunion B_ARG((bddp f, bddp g));
extern bddp   bddsubtract B_ARG((bddp f, bddp g));
extern bddp   bddcard B_ARG((bddp f));
extern bddp   bddlit B_ARG((bddp f));
extern bddp   bddlen B_ARG((bddp f));
extern int    bddimportz B_ARG((FILE* strm, bddp* p, int lim));
extern char  *bddcardmp16 B_ARG((bddp f, char *s));
extern int    bddisbdd B_ARG((bddp f));
extern int    bddiszbdd B_ARG((bddp f));

/************** SeqBDD operations *************/
extern bddp   bddpush B_ARG((bddp f, bddvar v));



#endif /* bddc_h */
