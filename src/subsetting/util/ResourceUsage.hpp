/*
 * CPU Resource Usage
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: ResourceUsage.hpp 351 2012-11-15 11:20:21Z iwashita $
 */

#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/time.h>
#ifdef WIN32
#include "../../mingw32/RpWinResource.h"
#else
#include <sys/resource.h>
#endif

namespace {
static double startTime = 0;
}

struct ResourceUsage {
    double etime;
    double utime;
    double stime;
    long maxrss;

    ResourceUsage() {
        update();
    }

    ResourceUsage(double etime, double utime, double stime, long maxrss)
            : etime(etime), utime(utime), stime(stime), maxrss(maxrss) {
    }

    ResourceUsage& update() {
        struct timeval t;
        gettimeofday(&t, 0);
        etime = double(t.tv_sec) + double(t.tv_usec) / 1000000;
        if (startTime == 0) startTime = etime;
        etime -= startTime;

        struct rusage s;
        getrusage(RUSAGE_SELF, &s);
        utime = s.ru_utime.tv_sec + s.ru_utime.tv_usec * 1e-6;
        stime = s.ru_stime.tv_sec + s.ru_stime.tv_usec * 1e-6;
        maxrss = s.ru_maxrss;

#ifndef WIN32
        if (maxrss == 0) maxrss = readMemoryStatus ("VmHWM:");
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

private:
    long readMemoryStatus(std::string key) {
        std::ifstream ifs("/proc/self/status");
        std::string buf;

        while (ifs.good()) {
            getline(ifs, buf);
            if (buf.compare(0, key.length(), key) == 0) {
                std::istringstream iss(buf.substr(key.length()));
                double size;
                std::string unit;
                iss >> size >> unit;
                switch (tolower(unit[0])) {
                case 'b':
                    size *= 1.0 / 1024.0;
                    break;
                case 'm':
                    size *= 1024.0;
                    break;
                case 'g':
                    size *= 1024.0 * 1024.0;
                    break;
                case 't':
                    size *= 1024.0 * 1024.0 * 1024.0;
                    break;
                }
                return long(size);
            }
        }

        return 0;
    }
};

class ElapsedTimeCounter {
    double totalTime;
    double startTime;

public:
    ElapsedTimeCounter()
            : totalTime(0), startTime(0) {
    }

    ElapsedTimeCounter& reset() {
        totalTime = 0;
        return *this;
    }

    ElapsedTimeCounter& start() {
        timeval t;
        gettimeofday(&t, 0);
        startTime = t.tv_sec + t.tv_usec * 1e-6;
        return *this;
    }

    ElapsedTimeCounter& stop() {
        timeval t;
        gettimeofday(&t, 0);
        totalTime += t.tv_sec + t.tv_usec * 1e-6 - startTime;
        return *this;
    }

    operator double() const {
        return totalTime;
    }

    friend std::ostream& operator<<(std::ostream& os, ElapsedTimeCounter const& o) {
        std::ios_base::fmtflags backup = os.flags(std::ios::fixed);
        os.setf(std::ios::fixed);
        os << std::setprecision(2) << o.totalTime << "s";
        os.flags(backup);
        return os;
    }
};
