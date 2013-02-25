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
#include <sys/resource.h>

struct ResourceUsage {
    double utime;
    double stime;
    long maxrss;

    ResourceUsage() {
        update();
    }

    ResourceUsage(double utime, double stime, long maxrss) :
        utime(utime), stime(stime), maxrss(maxrss) {
    }

    ResourceUsage& update() {
        struct rusage s;
        getrusage(RUSAGE_SELF, &s);
        utime = s.ru_utime.tv_sec + s.ru_utime.tv_usec * 1e-6;
        stime = s.ru_stime.tv_sec + s.ru_stime.tv_usec * 1e-6;
        maxrss = s.ru_maxrss;
        if (maxrss == 0) maxrss = readMemoryStatus("VmHWM:");
        return *this;
    }

    ResourceUsage operator+(ResourceUsage const& u) const {
        return ResourceUsage(utime + u.utime, stime + u.stime,
                std::max(maxrss, u.maxrss));
    }

    ResourceUsage& operator+=(ResourceUsage const& u) {
        utime += u.utime;
        stime += u.stime;
        if (maxrss < u.maxrss) maxrss = u.maxrss;
        return *this;
    }

    ResourceUsage operator-(ResourceUsage const& u) const {
        return ResourceUsage(utime - u.utime, stime - u.stime,
                std::max(maxrss, u.maxrss));
    }

    ResourceUsage& operator-=(ResourceUsage const& u) {
        utime -= u.utime;
        stime -= u.stime;
        if (maxrss < u.maxrss) maxrss = u.maxrss;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, ResourceUsage const& u) {
        std::ios_base::fmtflags backup = os.flags(std::ios::fixed);

        os << std::setprecision(2) << u.utime << "s, ";
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
                    size *= 1.0/1024.0;
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
