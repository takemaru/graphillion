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

#include <cstdio>
#include <ctime>
#include <iostream>
#include <streambuf>
#include <string>

#include "ResourceUsage.hpp"

namespace tdzdd {

inline std::string capitalize(std::string const& s) {
    std::string t = s;
    if (t.size() >= 1) {
        t[0] = toupper(s[0]);
    }
    return t;
}

template<typename T>
inline std::string to_string(T const& o) {
    std::ostringstream oss;
    oss << o;
    return oss.str();
}

template<std::ostream& os>
class MessageHandler_: public std::ostream {
    class Buf: public std::streambuf {
        MessageHandler_& mh;

    public:
        Buf(MessageHandler_& mh)
                : mh(mh) {
        }

    protected:
        virtual void imbue(std::locale const& loc) {
            os.imbue(loc);
        }

        virtual int overflow(int c) {
            if (!enabled) return c;

            if (lastUser != this) {
                if (column != 0) {
                    os.put('\n');
                    ++lineno;
                    column = 0;
                }
                lastUser = this;
            }

            if (c == EOF) return EOF;

            if (column == 0) {
                if (isspace(c)) return c;
                for (int i = mh.indent; i > 0; --i) {
                    os.put(' ');
                    ++column;
                }
            }

            os.put(c);

            if (c == '\n') {
                ++lineno;
                column = 0;
            }
            else {
                ++column;
            }

            return c;
        }
    };

    static int const INDENT_SIZE = 2;
    static bool enabled;
    static int indentLevel;
    static int lineno;
    static int column;
    static Buf* lastUser;

    Buf buf;
    std::string name;
    int indent;
    int beginLine;
    ResourceUsage initialUsage;
    ResourceUsage prevUsage;
    int totalSteps;
    int stepCount;
    int dotCount;
    time_t dotTime;
    bool stepping;

public:
    MessageHandler_()
            : std::ostream(&buf), buf(*this), indent(indentLevel * INDENT_SIZE),
              beginLine(0), totalSteps(0), stepCount(0),
              dotCount(0), dotTime(0), stepping(false) {
        flags(os.flags());
        precision(os.precision());
        width(os.width());
    }

    virtual ~MessageHandler_() {
        if (!name.empty()) end("aborted");
    }

    static bool showMessages(bool flag = true) {
        bool prev = enabled;
        enabled = flag;
        return prev;
    }

    MessageHandler_& begin(std::string const& s) {
        if (!enabled) return *this;
        if (!name.empty()) end("aborted");
        name = s.empty() ? "level-" + std::to_string(indentLevel) : s;
        indent = indentLevel * INDENT_SIZE;
        *this << "\n" << capitalize(name);
        indent = ++indentLevel * INDENT_SIZE;
        beginLine = lineno;
        initialUsage.update();
        prevUsage = initialUsage;
        setSteps(10);
        return *this;
    }

    MessageHandler_& setSteps(int steps) {
        if (!enabled) return *this;
        totalSteps = steps;
        stepCount = 0;
        dotCount = 0;
        dotTime = std::time(0);
        stepping = false;
        return *this;
    }

    MessageHandler_& step(char dot = '-') {
        if (!enabled) return *this;

        if (!stepping && dotTime + 4 < std::time(0)) {
            *this << '\n';
            stepping = true;
        }

        if (stepping) {
            if (stepCount % 50 != column - indent) {
                *this << '\n';
                for (int i = stepCount % 50; i > 0; --i) {
                    *this << '-';
                }
            }
            *this << dot;
            ++stepCount;
            if (column - indent >= 50) {
                ResourceUsage usage;
                ResourceUsage diff = usage - prevUsage;
                *this << std::setw(3) << std::right
                        << (stepCount * 100 / totalSteps);
                *this << "% (" << diff.elapsedTime() << ", " << diff.memory()
                        << ")\n";
                prevUsage = usage;
            }
        }
        else {
            ++stepCount;
            while (dotCount * totalSteps < stepCount * 10) {
                if (dotCount == 0) *this << ' ';
                *this << '.';
                ++dotCount;
                dotTime = std::time(0);
            }
        }

        return *this;
    }

    MessageHandler_& end(std::string const& msg = "", std::string const& info =
            "") {
        if (!enabled) return *this;
        if (name.empty()) return *this;

        ResourceUsage rusage = ResourceUsage() - initialUsage;

        if (beginLine == lineno) {
            if (!info.empty()) {
                *this << " " << info;
            }
            else if (msg.empty()) {
                *this << " done";
            }
            else {
                *this << " " << msg;
            }
            *this << " in " << rusage << ".\n";

            indent = --indentLevel * INDENT_SIZE;
        }
        else {
            indent = --indentLevel * INDENT_SIZE;

            if (msg.empty()) {
                *this << "\nDone " << name;
            }
            else {
                *this << "\n" << capitalize(msg);
            }
            if (!info.empty()) *this << " " << info;
            *this << " in " << rusage << ".\n";
        }

        name = "";
        return *this;
    }

    MessageHandler_& end(size_t n) {
        if (!enabled) return *this;
        return end("", "<" + to_string(n) + ">");
    }

    int col() const {
        return column;
    }
};

template<std::ostream& os>
bool MessageHandler_<os>::enabled = false;

template<std::ostream& os>
int MessageHandler_<os>::indentLevel = 0;

template<std::ostream& os>
int MessageHandler_<os>::lineno = 1;

template<std::ostream& os>
int MessageHandler_<os>::column = 0;

template<std::ostream& os>
typename MessageHandler_<os>::Buf* MessageHandler_<os>::lastUser = 0;

typedef MessageHandler_<std::cerr> MessageHandler;

} // namespace tdzdd
