/*
 * ----------------------------------------------------------------------
 *  RpWinResource.h
 *
 *  This file provides the neccessary structures and function prototypes
 *  for the Windows ports for Rappture of the following 
 *  resource management functions typically found on a Unix platform:
 *
 *      setrlimit(), getrlimit(), getrusage(), gettimeofday()
 *
 *  None of the ports are exactly perfect, but they are close enough
 *  for Rappture's use.
 * ======================================================================
 *  AUTHOR:  Nicholas J. Kisseberth, Purdue University
 *  Copyright (c) 2004-2012  HUBzero Foundation, LLC
 *
 *  See the file "license.terms" for information on usage and
 *  redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 * ======================================================================
 */
#ifndef RpWinResource_h
#define RpWinResource_h

#include <windows.h>

#define OPEN_MAX        512             /* Windows Hard Limit on open files */
#define RLIMIT_CPU      0               /* CPU time in seconds */
#define RLIMIT_FSIZE    1               /* Maximum filesize */
#define RLIMIT_DATA     2               /* max data size */
#define RLIMIT_STACK    3               /* max stack size */
#define RLIMIT_CORE     4               /* max core file size */
#define RLIMIT_NOFILE   5               /* max number of open files */
#define RLIMIT_AS       6               /* address space (virt. memory) limit */
#define RLIM_INFINITY   (0xffffffffUL)

typedef unsigned long rlim_t;

struct rlimit {
        rlim_t  rlim_cur;
        rlim_t  rlim_max;
};

#define RUSAGE_SELF     0               /* calling process */
#define RUSAGE_CHILDREN -1              /* terminated child processes */

struct rusage {
    struct timeval ru_utime; /* user time used */
    struct timeval ru_stime; /* system time used */
    long ru_maxrss;          /* max resident set size */
    long ru_ixrss;           /* integral  shared  text memory size */
    long ru_idrss;           /* integral unshared data size */
    long ru_isrss;           /* integral unshared  stack size */
    long ru_minflt;          /* page reclaims */
    long ru_majflt;          /* page faults */
    long ru_nswap;           /* swaps */
};

#ifdef __cplusplus
extern "C" {
#endif

HANDLE rpWinGetCurrentJob(); 
int getrusage(int intwho, struct rusage *rusage_in);
int getrlimit(int resource, struct rlimit *rlp);
int setrlimit(int resource, const struct rlimit *rlp);
int gettimeofday(struct timeval *tv, void *tz);

#ifdef __cplusplus
}
#endif

#endif
