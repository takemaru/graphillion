/*
 * Message Handler with CPU Status Report
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: MessageHandler.hpp 425 2013-02-25 09:39:15Z iwashita $
 */

#pragma once

#include <iostream>
#include <streambuf>
#include <string>

#include "ResourceUsage.hpp"

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
                    mh.stepCount = 0;
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
                mh.stepCount = 0;
            }
            else {
                ++column;
            }

            if (c == '.' && ++mh.stepCount >= 50) {
                ResourceUsage usage;
                ResourceUsage diff = usage - mh.prevUsage;
                os << " " << diff.elapsedTime() << ", " << diff.memory()
                        << "\n";
                //        auto backup = os.flags(std::ios::fixed);
                //        os << " " << std::setprecision(2) << diff.utime << "s, ";
                //        os << std::setprecision(0) << diff.maxrss / 1024.0 << "MB\n";
                //        os.flags(backup);
                ++lineno;
                column = 0;
                mh.prevUsage = usage;
                mh.stepCount = 0;
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

    std::string name;
    int indent;
    int beginLine;
    ResourceUsage initialUsage;
    ResourceUsage prevUsage;
    int stepCount;

public:
    MessageHandler_()
            : std::ostream(new Buf(*this)), indent(indentLevel * INDENT_SIZE),
              beginLine(0), stepCount(0) {
        flags(os.flags());
        precision(os.precision());
        width(os.width());
    }

    virtual ~MessageHandler_() {
        if (!name.empty()) end("aborted");
        delete rdbuf();
    }

    static void showMessages(bool flag = true) {
        enabled = flag;
    }

    MessageHandler_& begin(std::string const& s) {
        if (!name.empty()) end("aborted");
        name = s.empty() ? "level-" + indentLevel : s;
        indent = indentLevel * INDENT_SIZE;
        *this << capitalize(name);
        indent = ++indentLevel * INDENT_SIZE;
        beginLine = lineno;
        initialUsage.update();
        prevUsage = initialUsage;
        stepCount = 0;
        return *this;
    }

    MessageHandler_& end(std::string const& msg = "", std::string const& info =
            "") {
        if (name.empty()) return *this;
        indent = --indentLevel * INDENT_SIZE;
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
        }
        else {
            if (msg.empty()) {
                *this << "\nDone " << name;
            }
            else {
                *this << "\n" << capitalize(msg);
            }
            if (!info.empty()) *this << " " << info;
        }
        *this << " in " << rusage << ".\n";
        name = "";
        return *this;
    }

    MessageHandler_& end(size_t n) {
        return end("", "<" + to_string(n) + ">");
    }

    int col() const {
        return column;
    }

private:
    static std::string capitalize(std::string const& s) {
        std::string t = s;
        if (t.size() >= 1) {
            t[0] = toupper(s[0]);
        }
        return t;
    }

    template<typename T>
    static std::string to_string(T const& o) {
        std::ostringstream oss;
        oss << o;
        return oss.str();
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
