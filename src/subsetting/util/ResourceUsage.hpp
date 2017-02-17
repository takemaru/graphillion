/*
 * TdZdd: a Top-down/Breadth-first Decision Diagram Manipulation Framework
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2014 ERATO MINATO Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#ifndef _MSC_VER
#include <sys/time.h>
#endif
#ifndef _WIN32
#include <sys/resource.h>
#else
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#endif

namespace tdzdd {

#ifndef _MSC_VER
inline double getWallClockTime() {
    struct timeval t;
    gettimeofday(&t, 0);
    return double(t.tv_sec) + double(t.tv_usec) / 1000000;
}
#else
inline double getWallClockTime() {
    FILETIME t;
    GetSystemTimeAsFileTime(&t);
    return double(t.dwHighDateTime) / 10 + double(t.dwLowDateTime) / 10000000;
}
#endif

struct ResourceUsage {
    double etime;
    double utime;
    double stime;
    long maxrss;

    ResourceUsage() {
        update();
    }

    ResourceUsage(double etime, double utime, double stime, long maxrss) :
            etime(etime), utime(utime), stime(stime), maxrss(maxrss) {
    }

    ResourceUsage& update() {
        etime = getWallClockTime();

#ifdef _WIN32
        HANDLE h = GetCurrentProcess();
        FILETIME ft_creat, ft_exit, ft_kernel, ft_user;
        if (GetProcessTimes(h, &ft_creat, &ft_exit, &ft_kernel, &ft_user)) {
            ULARGE_INTEGER ul_kernel, ul_user;
            ul_kernel.LowPart = ft_kernel.dwLowDateTime;
            ul_kernel.HighPart = ft_kernel.dwHighDateTime;
            ul_user.LowPart = ft_user.dwLowDateTime;
            ul_user.HighPart = ft_user.dwHighDateTime;
            stime = ul_kernel.QuadPart * 1e-7;
            utime = ul_user.QuadPart * 1e-7;
        }

        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(h, &pmc, sizeof(pmc))) {
            maxrss = pmc.WorkingSetSize / 1024;
        }
#else
        struct rusage s;
        getrusage(RUSAGE_SELF, &s);
        utime = s.ru_utime.tv_sec + s.ru_utime.tv_usec * 1e-6;
        stime = s.ru_stime.tv_sec + s.ru_stime.tv_usec * 1e-6;
        maxrss = s.ru_maxrss;
#endif
        return *this;
    }

    ResourceUsage operator+(ResourceUsage const& u) const {
        return ResourceUsage(etime + u.etime, utime + u.utime, stime + u.stime,
                std::max(maxrss, u.maxrss));
    }

    ResourceUsage& operator+=(ResourceUsage const& u) {
        etime += u.etime;
        utime += u.utime;
        stime += u.stime;
        if (maxrss < u.maxrss) maxrss = u.maxrss;
        return *this;
    }

    ResourceUsage operator-(ResourceUsage const& u) const {
        return ResourceUsage(etime - u.etime, utime - u.utime, stime - u.stime,
                std::max(maxrss, u.maxrss));
    }

    ResourceUsage& operator-=(ResourceUsage const& u) {
        etime -= u.etime;
        utime -= u.utime;
        stime -= u.stime;
        if (maxrss < u.maxrss) maxrss = u.maxrss;
        return *this;
    }

    std::string elapsedTime() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << etime << "s";
        return ss.str();
    }

    std::string userTime() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << utime << "s";
        return ss.str();
    }

    std::string memory() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(0) << maxrss / 1024.0 << "MB";
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, ResourceUsage const& u) {
        std::ios_base::fmtflags backup = os.flags(std::ios::fixed);
        os.setf(std::ios::fixed);

        os << std::setprecision(2) << u.etime << "s elapsed, ";
        os << std::setprecision(2) << u.utime << "s user, ";
        os << std::setprecision(0) << u.maxrss / 1024.0 << "MB";

        os.flags(backup);
        return os;
    }
};

class ElapsedTimeCounter {
    double totalTime;
    double startTime;

public:
    ElapsedTimeCounter() :
            totalTime(0), startTime(0) {
    }

    ElapsedTimeCounter& reset() {
        totalTime = 0;
        return *this;
    }

    ElapsedTimeCounter& start() {
        startTime = getWallClockTime();
        return *this;
    }

    ElapsedTimeCounter& stop() {
        totalTime += getWallClockTime() - startTime;
        return *this;
    }

    operator double() const {
        return totalTime;
    }

    friend std::ostream& operator<<(std::ostream& os,
                                    ElapsedTimeCounter const& o) {
        std::ios_base::fmtflags backup = os.flags(std::ios::fixed);
        os.setf(std::ios::fixed);
        os << std::setprecision(2) << o.totalTime << "s";
        os.flags(backup);
        return os;
    }
};

} // namespace tdzdd
