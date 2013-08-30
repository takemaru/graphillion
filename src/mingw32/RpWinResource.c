/*
 * ----------------------------------------------------------------------
 *  RpWinResource.c
 *
 *  This file provides the Windows ports for Rappture of the following 
 *  resource management functions typically found on a Unix platform:
 *
 *      setrlimit(), getrlimit(), getrusage(), gettimeofday(), timeradd()
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

#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <stdio.h>
#include <limits.h>
#include <psapi.h>
#include <time.h>
#include <sys/timeb.h>
#include <errno.h>
#include "RpWinResource.h"

#define _ftime_s _ftime

static HANDLE currentJob = NULL;

void
rpWinInitJob()
{
    if (currentJob == NULL) {
        currentJob = CreateJobObject(NULL, NULL);
        AssignProcessToJobObject(currentJob, GetCurrentProcess());
    }
}

HANDLE 
rpWinGetCurrentJob()
{
    return currentJob;
}

/* add timeval values */
void
timeradd(struct timeval *a, struct timeval *b, struct timeval *result)
{
    result->tv_sec = a->tv_sec + b->tv_sec;
    result->tv_usec = a->tv_usec + b->tv_usec;
    if (result->tv_usec >= 1000000)
    {
        result->tv_sec++;
        result->tv_usec -= 1000000;
    }
}

/* add rusage values */
static void
rusageadd(struct rusage *a, struct rusage *b, struct rusage *result)
{
    timeradd(&a->ru_utime, &b->ru_utime, &result->ru_utime);
    timeradd(&a->ru_stime, &b->ru_stime, &result->ru_stime);
    result->ru_maxrss = a->ru_maxrss + b->ru_maxrss;
    result->ru_ixrss  = a->ru_ixrss  + b->ru_ixrss;
    result->ru_idrss  = a->ru_idrss  + b->ru_idrss;
    result->ru_isrss  = a->ru_isrss  + b->ru_isrss;
    result->ru_minflt = a->ru_minflt + b->ru_minflt;
    result->ru_majflt = a->ru_majflt + b->ru_majflt;
    result->ru_nswap  = a->ru_nswap  + b->ru_nswap;
}

/* convert windows FILETIME structure to unix timeval structure */
static void
filetime_to_timeval(struct timeval *dst, FILETIME *src)
{
    ULARGE_INTEGER x;

    x.LowPart = src->dwLowDateTime;
    x.HighPart = src->dwHighDateTime;

    dst->tv_sec  = x.QuadPart / 10000000;
    dst->tv_usec = (x.QuadPart % 10000000) / 10;

}

/* implement getrusage(RUSAGE_SELF) */
static void
fill_rusage_self(struct rusage *r, HANDLE h)
{
    FILETIME creationTime, exitTime, kernelTime, userTime;
    struct timeval tv;
    PROCESS_MEMORY_COUNTERS pmc;

    if (!GetProcessTimes(h, &creationTime, &exitTime, &kernelTime, &userTime))
        return;

    filetime_to_timeval(&tv, &kernelTime);
    timeradd(&r->ru_stime, &tv, &r->ru_stime);
    filetime_to_timeval(&tv, &userTime);
    timeradd(&r->ru_utime, &tv, &r->ru_utime);
  
    if (GetProcessMemoryInfo(h, &pmc, sizeof(pmc)))
    {
        r->ru_maxrss += (long) (pmc.WorkingSetSize /1024);
        r->ru_majflt += pmc.PageFaultCount;
    }
}

/* implement getrusage(RUSAGE_CHILDREN) */
static void
fill_rusage_job(struct rusage *r, HANDLE h)
{
    BOOL result;
    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION jbai;
    PROCESS_MEMORY_COUNTERS pmc;

    if (QueryInformationJobObject(rpWinGetCurrentJob(), 
               JobObjectBasicAccountingInformation,
               &jbai, sizeof(JOBOBJECT_BASIC_ACCOUNTING_INFORMATION), NULL))
    {
        r->ru_stime.tv_sec = (long)(jbai.TotalKernelTime.QuadPart/10)/1000000;
        r->ru_stime.tv_usec = (long)(jbai.TotalKernelTime.QuadPart/10)%1000000;
        r->ru_utime.tv_sec = (long)(jbai.TotalUserTime.QuadPart/10)/1000000;
        r->ru_utime.tv_usec = (long)(jbai.TotalUserTime.QuadPart/10)%1000000;

        h = GetCurrentProcess();

        memset (&pmc, 0, sizeof (pmc));

        if (GetProcessMemoryInfo (h, &pmc, sizeof (pmc)))
        {
            r->ru_maxrss += (long) (pmc.WorkingSetSize /1024);
            r->ru_majflt += pmc.PageFaultCount;
        }
    }
}

/* get system resource statistics */
int
getrusage (int intwho, struct rusage *rusage_in)
{
    int res = 0;
    struct rusage r;

    memset(&r, 0, sizeof(r));

    if (intwho == RUSAGE_SELF)
    {
        fill_rusage_self(&r, GetCurrentProcess());
        *rusage_in = r;
    }
    else if (intwho == RUSAGE_CHILDREN)
    {
        fill_rusage_job(&r, rpWinGetCurrentJob());
        *rusage_in = r;
    }
    else
    {
        errno = EINVAL;
        res = -1;
    }
  
    return res;
}

static unsigned long rlim_cpu = RLIM_INFINITY;
static unsigned long rlim_core = RLIM_INFINITY;
static unsigned long rlim_fsize = RLIM_INFINITY;

/* get system resource limits */
int
getrlimit (int resource, struct rlimit *rlp)
{
    MEMORY_BASIC_INFORMATION m;

    rlp->rlim_cur = RLIM_INFINITY;
    rlp->rlim_max = RLIM_INFINITY;

    switch (resource)
    {
        case RLIMIT_CPU:
            rlp->rlim_cur = rlim_cpu;
            break;
        case RLIMIT_FSIZE:
            rlp->rlim_cur = rlim_fsize;
            break;
        case RLIMIT_DATA:
            break;
        case RLIMIT_STACK:
            if (!VirtualQuery ((LPCVOID) &m, &m, sizeof m))
                /* debug_printf 
                 * ("couldn't get stack info, returning def.values. %E") */ ;
            else
            {
                rlp->rlim_cur = (DWORD) &m - (DWORD) m.AllocationBase;
                rlp->rlim_max = (DWORD) m.BaseAddress + m.RegionSize
                                - (DWORD) m.AllocationBase;
            }
            break;
        case RLIMIT_NOFILE:
            rlp->rlim_cur = _getmaxstdio();
            rlp->rlim_max = 2048;
            break;
        case RLIMIT_CORE:
            rlp->rlim_cur = rlim_core;
            break;
        case RLIMIT_AS:
            rlp->rlim_cur = 0x80000000UL;
            rlp->rlim_max = 0x80000000UL;
            break;
        default:
            errno = EINVAL;
            return -1;
    }

    return 0;
}

/* set system resource limits */
int
setrlimit (int resource, const struct rlimit *rlp)
{
    struct rlimit oldlimits;

    if (getrlimit (resource, &oldlimits) < 0)
        return -1;

    if (oldlimits.rlim_cur == rlp->rlim_cur &&
        oldlimits.rlim_max == rlp->rlim_max)
        return 0; // No change requested

    switch (resource)
    {
        case RLIMIT_CPU:
            rlim_cpu = rlp->rlim_cur;
            break;
        case RLIMIT_FSIZE:
            rlim_fsize = rlp->rlim_cur;
            break;
        case RLIMIT_CORE:
            rlim_core = rlp->rlim_cur;
            break;
        case RLIMIT_NOFILE:
            if (rlp->rlim_cur != RLIM_INFINITY)
                return _setmaxstdio(rlp->rlim_cur);
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    return 0;
}

/* Here are a few implementations of gettimeofday() for windows. */
/* they vary in short term (subsecond) and absolute accuracy */

#define EPOCHFILETIME (116444736000000000LL)

static int 
gettimeofday1(struct timeval *tv, void *tz)
{
    FILETIME        fileTime;
    LARGE_INTEGER   li;
    __int64         t;

    if (tv == NULL)
        return(-1);

    GetSystemTimeAsFileTime(&fileTime);
    li.LowPart  = fileTime.dwLowDateTime;
    li.HighPart = fileTime.dwHighDateTime;
    t  = (li.QuadPart - EPOCHFILETIME) / 10;       
    tv->tv_sec  = (long)(t / 1000000);
    tv->tv_usec = (long)(t % 1000000);

    return 0;
}

static unsigned __int64 base       = 0;
static unsigned __int64 frequency  = 0;
static           double ufrequency = 0.0;
static           double mfrequency = 0.0;

int
gettimeofday(struct timeval *tv, void *tz)
{
    unsigned __int64 counter = 0;

    QueryPerformanceCounter((LARGE_INTEGER *) &counter);

    if (ufrequency == 0.0)
    {
        struct _timeb now, tb;
        // mutex_lock(L_GETTIMEOFDAY);
        QueryPerformanceFrequency((LARGE_INTEGER *) &frequency);
        ufrequency = (double) frequency / 1000000.0f;
        mfrequency = (double) frequency / 1000.0f;
        _ftime_s(&tb);
        _ftime_s(&now);
        while( (now.time == tb.time) && (now.millitm == tb.millitm))
            _ftime_s(&now);
        QueryPerformanceCounter((LARGE_INTEGER *) &counter);
        base = (__int64) (now.time * frequency) + 
               (__int64) (now.millitm * mfrequency) - counter;
        // mutex_unlock(L_GETTIMEOFDAY);
        QueryPerformanceCounter((LARGE_INTEGER *) &counter);
    }

    counter += base;
    tv->tv_sec = (long) (counter / frequency);
    tv->tv_usec = (long) ((__int64)(counter / ufrequency) % 1000000);
    return 0;
}

static int
gettimeofday2(struct timeval *tv, void *tz)
{
    struct _timeb tb;

    _ftime_s(&tb);

    tv->tv_sec =  (long) tb.time;
    tv->tv_usec = tb.millitm * 1000;

    return(0);
}

