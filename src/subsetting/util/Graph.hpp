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

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <climits>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <vector>

#include "MessageHandler.hpp"

namespace tdzdd {

class Graph {
public:
    typedef int VertexNumber;
    typedef int EdgeNumber;
    typedef int ColorNumber;
    typedef std::pair<VertexNumber,VertexNumber> VertexNumberPair;

    struct EdgeInfo {
        VertexNumber v0;
        VertexNumber v1;
        VertexNumber v2;
        bool v1final;
        bool v2final;
        bool v1final2;
        bool v2final2;
        bool allColorsSeen;
        bool finalEdge;

        EdgeInfo(VertexNumber v1, VertexNumber v2)
                : v0(0), v1(v1), v2(v2), v1final(false), v2final(false),
                  v1final2(false), v2final2(false), allColorsSeen(false),
                  finalEdge(false) {
        }

        friend std::ostream& operator<<(std::ostream& os, EdgeInfo const& o) {
            os << o.v0 << "--" << o.v1;
            if (o.v1final) os << "$";
            if (o.v1final2) os << "-";
            os << "--" << o.v2;
            if (o.v2final) os << "$";
            if (o.v2final2) os << "-";
            if (o.allColorsSeen) os << "*";
            if (o.finalEdge) os << "$";
            return os;
        }
    };

    static VertexNumber const MAX_VERTICES = USHRT_MAX;
    static EdgeNumber const MAX_EDGES = INT_MAX;
    static ColorNumber const MAX_COLORS = USHRT_MAX;

private:
    std::vector<std::pair<std::string,std::string> > edgeNames;
    std::map<std::string,std::string> name2label;
    std::map<std::string,std::string> name2color;
    std::map<std::string,VertexNumber> name2vertex;
    std::vector<std::string> vertex2name;
    std::map<std::pair<std::string,std::string>,EdgeNumber> name2edge;
    std::vector<std::pair<std::string,std::string> > edge2name;
    std::vector<EdgeInfo> edgeInfo_;
    std::map<VertexNumberPair,EdgeNumber> edgeIndex;
    std::vector<VertexNumber> virtualMate_;
    std::vector<ColorNumber> colorNumber_;
    VertexNumber vMax;
    ColorNumber numColor_;
    bool hasColorPairs_;

public:
    void addEdge(std::string vertexName1, std::string vertexName2) {
        edgeNames.push_back(std::make_pair(vertexName1, vertexName2));
    }

    void setColor(std::string v, std::string color) {
        name2color[v] = color;
    }

    void setColor(std::string v, int color) {
        name2color[v] = getColor(color);
    }

    void readEdges(std::string const& filename) {
        tdzdd::MessageHandler mh;
        mh.begin("reading");

        if (filename.empty()) {
            mh << " STDIN ...";
            readEdges(std::cin);
        }
        else {
            mh << " \"" << filename << "\" ...";
            std::ifstream fin(filename.c_str(), std::ios::in);
            if (!fin) throw std::runtime_error(strerror(errno));
            readEdges(fin);
        }

        mh.end();
        update();
    }

    void readAdjacencyList(std::string const& filename) {
        tdzdd::MessageHandler mh;
        mh.begin("reading");

        if (filename.empty()) {
            mh << " STDIN ...";
            readAdjacencyList(std::cin);
        }
        else {
            mh << " \"" << filename << "\" ...";
            std::ifstream fin(filename.c_str(), std::ios::in);
            if (!fin) throw std::runtime_error(strerror(errno));
            readAdjacencyList(fin);
        }

        mh.end();
        update();
    }

    void readVertexGroups(std::string const& filename) {
        tdzdd::MessageHandler mh;
        mh.begin("reading");

        if (filename.empty()) {
            mh << " STDIN ...";
            readVertexGroups(std::cin);
        }
        else {
            mh << " \"" << filename << "\" ...";
            std::ifstream fin(filename.c_str(), std::ios::in);
            if (!fin) throw std::runtime_error(strerror(errno));
            readVertexGroups(fin);
        }

        mh.end();
        update();
    }

private:
    void readEdges(std::istream& is) {
        std::string v, v1, v2;

        while (is) {
            char c = is.get();

            if (isspace(c)) {
                if (!v.empty()) {
                    if (v1.empty()) {
                        v1 = v;
                        v.clear();
                    }
                    else if (v2.empty()) {
                        v2 = v;
                        v.clear();
                    }
                    else {
                        throw std::runtime_error(
                                "ERROR: More than two tokens in a line");
                    }
                }

                if (c == '\n') {
                    if (!v1.empty() && !v2.empty()) {
                        edgeNames.push_back(std::make_pair(v1, v2));
                        v1.clear();
                        v2.clear();
                    }
                    else if (!v1.empty()) {
                        throw std::runtime_error(
                                "ERROR: Only one token in a line");
                    }
                }
            }
            else {
                v += c;
            }
        }

        if (!v1.empty() && !v2.empty()) {
            edgeNames.push_back(std::make_pair(v1, v2));
        }
        else if (!v1.empty()) {
            throw std::runtime_error("ERROR: Only one token in a line");
        }
    }

    void readAdjacencyList(std::istream& is) {
        edgeNames.clear();
        name2label.clear();
        name2color.clear();

        VertexNumber v1 = 1;
        VertexNumber v2;

        while (is) {
            char c;
            while (isspace(c = is.get())) {
                if (c == '\n') ++v1;
            }
            if (!is) break;
            is.unget();
            is >> v2;

            edgeNames.push_back(std::make_pair(to_string(v1), to_string(v2)));
        }
    }

    void readVertexGroups(std::istream& is) {
        std::vector<VertexNumber> group;
        int n = 0;
        std::string color = getColor(n++);
        name2color.clear();

        while (is) {
            char c;
            while (isspace(c = is.get())) {
                if (c == '\n') {
                    color = getColor(n++);
                }
            }
            if (!is) break;
            is.unget();

            VertexNumber v;
            is >> v;

            name2color[to_string(v)] = color;
        }
    }

    std::string getColor(int n) {
        char const HEX[] = "0123456789abcdef";
        std::string color("#000000");

        color[2] = HEX[(n / 256) % 16];
        color[4] = HEX[(n / 16) % 16];
        color[6] = HEX[n % 16];
        color[1] = HEX[15 - ((n * 3) % 11)];
        color[3] = HEX[(n * 5 + 5) % 11 + 5];
        color[5] = HEX[15 - ((n * 2 + 7) % 11)];
        return color;
    }

public:
    /*
     * INPUT:
     *   edgeNames
     *   name2label
     *   name2color
     */
    void update() {
        name2vertex.clear();
        vertex2name.clear();
        name2edge.clear();
        edge2name.clear();
        edgeInfo_.clear();
        edgeIndex.clear();
        vMax = 0;

        // Make unique edge name list
        for (size_t i = 0; i < edgeNames.size(); ++i) {
            std::pair<std::string,std::string> const& e = edgeNames[i];

            if (name2edge.count(e) == 0) {
                edge2name.push_back(e);
                name2edge[e] = -1;
                name2edge[std::make_pair(e.second, e.first)] = -1;
            }
        }

        // Sort vertices by leaving order
        {
            std::vector<std::string> stack;
            stack.reserve(edge2name.size() * 2);

            for (size_t i = edge2name.size() - 1; i + 1 > 0; --i) {
                std::string const& s1 = edge2name[i].first;
                std::string const& s2 = edge2name[i].second;

                if (name2vertex.count(s2) == 0) {
                    name2vertex[s2] = 0;
                    stack.push_back(s2);
                }

                if (name2vertex.count(s1) == 0) {
                    name2vertex[s1] = 0;
                    stack.push_back(s1);
                }
            }

            vertex2name.push_back(""); // begin vertex number with 1

            while (!stack.empty()) {
                std::string const& s = stack.back();
                name2vertex[s] = vertex2name.size();
                vertex2name.push_back(s);
                if (vertex2name.size() > size_t(MAX_VERTICES)) throw std::runtime_error(
                        "ERROR: Vertex number > " + to_string(MAX_VERTICES));
                stack.pop_back();
            }
        }

        for (size_t i = 0; i < edge2name.size(); ++i) {
            std::pair<std::string,std::string> const& e = edge2name[i];
            std::string s1 = e.first;
            std::string s2 = e.second;

            if (name2vertex[s1] > name2vertex[s2]) {
                std::swap(s1, s2);
            }

            VertexNumber v1 = name2vertex[s1];
            VertexNumber v2 = name2vertex[s2];

            if (v1 == 0) throw std::runtime_error(
                    "ERROR: " + s1 + ": No such vertex");
            if (v2 == 0) throw std::runtime_error(
                    "ERROR: " + s2 + ": No such vertex");

            VertexNumberPair vp(v1, v2);

            if (edgeIndex.count(vp) == 0) {
                EdgeNumber a = edgeInfo_.size();
                edgeInfo_.push_back(EdgeInfo(v1, v2));
                edgeIndex[vp] = a;
                name2edge[std::make_pair(s1, s2)] = a;
                name2edge[std::make_pair(s2, s1)] = a;
                if (vMax < v2) vMax = v2;
            }

            if (edgeInfo_.size() > size_t(MAX_EDGES)) throw std::runtime_error(
                    "ERROR: Edge number > " + to_string(MAX_EDGES));
        }

        {
            std::map<std::string,std::set<VertexNumber> > color2vertices;

            for (std::map<std::string,std::string>::iterator t =
                    name2color.begin(); t != name2color.end(); ++t) {
                VertexNumber v = name2vertex[t->first];
                if (v == 0) throw std::runtime_error(
                        "ERROR: " + t->first + ": No such vertex");
                color2vertices[t->second].insert(v); // color => set of vertices
            }

            if (color2vertices.size() > size_t(MAX_COLORS)) {
                throw std::runtime_error(
                        "ERROR: Target paths > " + to_string(MAX_COLORS));
            }

            virtualMate_.resize(vMax + 1);
            colorNumber_.resize(vMax + 1);
            for (VertexNumber v = 1; v <= vMax; ++v) {
                virtualMate_[v] = 0;
                colorNumber_[v] = 0;
            }
            numColor_ = 0;
            hasColorPairs_ = !color2vertices.empty();

            for (std::map<std::string,std::set<VertexNumber> >::iterator t =
                    color2vertices.begin(); t != color2vertices.end(); ++t) {
                ++numColor_;

                if (t->second.size() == 2) {
                    std::set<VertexNumber>::iterator tt = t->second.begin();
                    VertexNumber v1 = *tt++;
                    VertexNumber v2 = *tt;
                    virtualMate_[v1] = v2;
                    virtualMate_[v2] = v1;
                }
                else {
                    hasColorPairs_ = false;
                }

                for (std::set<VertexNumber>::iterator tt = t->second.begin();
                        tt != t->second.end(); ++tt) {
                    VertexNumber v = *tt;
                    colorNumber_[v] = numColor_;
                }
            }

            // sort colorNumber
            std::vector<ColorNumber> colorMap(numColor_ + 1);
            ColorNumber cn = 0;
            for (VertexNumber v = 1; v <= vMax; ++v) {
                ColorNumber c = colorNumber_[v];
                if (c == 0) continue;
                ColorNumber& cc = colorMap[c];
                if (cc == 0) cc = ++cn;
                colorNumber_[v] = cc;
            }
        }

        {
            std::vector<EdgeNumber> lastEdge(vMax + 1);
            std::vector<EdgeNumber> secondLastEdge(vMax + 1);
            for (VertexNumber v = 1; v <= vMax; ++v) {
                lastEdge[v] = -1;
                secondLastEdge[v] = -1;
            }
            for (EdgeNumber a = 0; a < edgeSize(); ++a) {
                EdgeInfo& e = edgeInfo_[a];
                secondLastEdge[e.v1] = lastEdge[e.v1];
                secondLastEdge[e.v2] = lastEdge[e.v2];
                lastEdge[e.v1] = a;
                lastEdge[e.v2] = a;
            }

            std::vector<EdgeNumber> finalEdgeToColor(numColor_ + 1);
            EdgeNumber fitstEdgeToFinalColor = 0;
            std::vector<bool> touched(numColor_ + 1);
            touched[0] = true;
            ColorNumber k = numColor_;
            for (EdgeNumber a = 0; a < edgeSize(); ++a) {
                EdgeInfo const& e = edgeInfo(a);
                ColorNumber n1 = colorNumber(e.v1);
                ColorNumber n2 = colorNumber(e.v2);
                finalEdgeToColor[n1] = a;
                finalEdgeToColor[n2] = a;
                if (!touched[n1]) {
                    if (--k == 0) {
                        fitstEdgeToFinalColor = a;
                        break;
                    }
                    touched[n1] = true;
                }
                if (!touched[n2]) {
                    if (--k == 0) {
                        fitstEdgeToFinalColor = a;
                        break;
                    }
                    touched[n2] = true;
                }
            }

            VertexNumber v0 = 1;

            for (EdgeNumber a = 0; a < edgeSize(); ++a) {
                EdgeInfo& e = edgeInfo_[a];
                while (lastEdge[v0] < a) {
                    ++v0;
                    assert(v0 <= vMax);
                }
                e.v0 = v0;
                e.v1final = (a == lastEdge[e.v1]);
                e.v2final = (a == lastEdge[e.v2]);
                e.v1final2 = (a == secondLastEdge[e.v1]);
                e.v2final2 = (a == secondLastEdge[e.v2]);
                e.allColorsSeen = (a >= fitstEdgeToFinalColor);
                e.finalEdge = (a == edgeSize() - 1);
            }
        }
    }

    VertexNumber vertexSize() const {
        return vMax;
    }

    EdgeNumber edgeSize() const {
        return edgeInfo_.size();
    }

    EdgeInfo
    const& edgeInfo(EdgeNumber a) const {
        assert(0 <= a && size_t(a) < edgeInfo_.size());
        return edgeInfo_[a];
    }

    VertexNumber getVertex(std::string const& name) const {
        std::map<std::string,VertexNumber>::const_iterator found =
                name2vertex.find(name);
        if (found == name2vertex.end()) throw std::runtime_error(
                "ERROR: " + name + ": No such vertex");
        return found->second;
    }

    std::string vertexName(VertexNumber v) const {
        if (v < 1 || vertexSize() < v) return "?";
        return vertex2name[v];
    }

    std::string vertexLabel(VertexNumber v) const {
        std::string label = vertexName(v);

        std::map<std::string,std::string>::const_iterator found =
                name2label.find(label);
        if (found != name2label.end()) {
            label = found->second;
        }

        return label;
    }

    EdgeNumber getEdge(std::pair<std::string,std::string> const& name) const {
        std::map<std::pair<std::string,std::string>,EdgeNumber>::const_iterator found =
                name2edge.find(name);
        if (found == name2edge.end()) throw std::runtime_error(
                "ERROR: " + name.first + "," + name.second + ": No such edge");
        return found->second;
    }

    EdgeNumber getEdge(std::string const& name1,
            std::string const& name2) const {
        return getEdge(std::make_pair(name1, name2));
    }

    std::pair<std::string,std::string> edgeName(EdgeNumber e) const {
        if (e < 0 || edgeSize() <= e) return std::make_pair("?", "?");
        return edge2name[e];
    }

    std::string edgeLabel(EdgeNumber e) const {
        std::pair<std::string,std::string> name = edgeName(e);
        std::string label = name.first + "," + name.second;

        std::map<std::string,std::string>::const_iterator found =
                name2label.find(label);
        if (found != name2label.end()) {
            label = found->second;
        }

        return label;
    }

    VertexNumber virtualMate(VertexNumber v) const {
        return (1 <= v && v <= vMax) ? virtualMate_[v] : 0;
    }

    EdgeNumber getEdge(VertexNumber v1, VertexNumber v2) const {
        assert(1 <= v1 && v1 <= vMax);
        assert(1 <= v2 && v2 <= vMax);
        if (v1 > v2) std::swap(v1, v2);
        VertexNumberPair vp(v1, v2);
        std::map<VertexNumberPair,EdgeNumber>::const_iterator found =
                edgeIndex.find(vp);
        if (found == edgeIndex.end()) throw std::runtime_error(
                "ERROR: (" + to_string(v1) + "," + to_string(v2)
                        + "): No such edge");
        return found->second;
    }

    VertexNumber maxFrontierSize() const {
        VertexNumber n = 0;
        for (EdgeNumber a = 0; a < edgeSize(); ++a) {
            EdgeInfo const& e = edgeInfo(a);
            VertexNumber m = e.v2 - e.v0 + 1;
            if (n < m) n = m;
        }
        return n;
    }

    void clearColors() {
        name2color.clear();
        virtualMate_.clear();
        virtualMate_.resize(vMax + 1);
        colorNumber_.clear();
        colorNumber_.resize(vMax + 1);
        numColor_ = 0;
    }

    void setDefaultPathColor() {
        name2color.clear();
        name2color[to_string(1)] = "#ff7777";
        name2color[to_string(vMax)] = "#ff7777";
        update();
    }

    ColorNumber colorNumber(VertexNumber v) const {
        return (1 <= v && v <= vMax) ? colorNumber_[v] : 0;
    }

    ColorNumber numColor() const {
        return numColor_;
    }

    bool hasColorPairs() const {
        return hasColorPairs_;
    }

    template<typename E>
    std::ostream& dump(std::ostream& os, E const& edgeDecorator) const {
        os << "graph {\n";
        //os << "  layout=neato;\n";

        for (std::vector<std::string>::const_iterator t = vertex2name.begin();
                t != vertex2name.end(); ++t) {
            if (t->empty()) continue;
            os << "  \"" << *t << "\"";
            std::map<std::string,std::string>::const_iterator e =
                    name2label.find(*t);
            if (e != name2label.end()) {
                os << "[label=\"" << e->second << "\"]";
            }
            e = name2color.find(*t);
            if (e != name2color.end()) {
                os << "[color=\"" << e->second << "\",style=filled]";
            }
            os << ";\n";
        }

        for (EdgeNumber a = 0; a < edgeSize(); ++a) {
            EdgeInfo const& e = edgeInfo(a);
            std::string s1 = vertex2name[e.v1];
            std::string s2 = vertex2name[e.v2];
            os << "  \"" << s1 << "\"--\"" << s2 << "\"";
            std::map<std::string,std::string>::const_iterator t =
                    name2label.find(s1 + "," + s2);
            if (t != name2label.end()) {
                os << "[label=\"" << t->second << "\"]";
            }
            t = name2color.find(s1 + "," + s2);
            if (t != name2color.end()) {
                os << "[color=\"" << t->second << "\",style=bold]";
            }
            os << edgeDecorator(a);
            os << ";\n";
        }

        os << "}\n";
        os.flush();
        return os;
    }

private:
    struct NoEdgeDecorator {
        std::string operator()(EdgeNumber a) const {
            return "";
        }
    };

public:
    std::ostream& dump(std::ostream& os) const {
        return dump(os, NoEdgeDecorator());
    }

    friend std::ostream& operator<<(std::ostream& os, Graph const& g) {
        return g.dump(os);
    }

private:
    static std::string to_string(int i) {
        std::ostringstream oss;
        oss << i;
        return oss.str();
    }
};

} // namespace tdzdd
