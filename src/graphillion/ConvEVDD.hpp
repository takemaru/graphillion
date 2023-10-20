// This file is copied from the repository below and edited.
// https://github.com/junkawahara/enum_cis
#ifndef CONVEVDD_HPP
#define CONVEVDD_HPP

// original
// #include "tdzdd/util/Graph.hpp"
// #include "InducingColoringSpec.hpp"
// original

// added
#include "SAPPOROBDD/ZBDD.h"
#include "subsetting/util/Graph.hpp"
// added

// TODO: remove
#include <iostream>
using namespace std;

class ConvEVDD {
private:
    struct ZDDEVSpecConf {
        tdzdd::NodeId node;
    };

public:
    class VariableList {
    public:
        enum Kind {VERTEX, EDGE};
    private:
        std::vector<Kind> kind_list_; // index: ev
        std::vector<int> variable_number_list_; // index: ev
        std::vector<int> ev_to_newv_; // index: ev
        std::vector<int> v_to_newv_; // index: v
        std::vector<int> newv_to_v_; // index: newv
        int m_;
        int n_;

    public:
        VariableList(const tdzdd::Graph& graph) : m_(graph.edgeSize()),
                                                  n_(graph.vertexSize())
        {
            constructEVArray(graph);
        }

        void constructEVArray(const tdzdd::Graph& graph)
        {
            int pos = m_ + n_;
            int new_n = n_;
            kind_list_.resize(m_ + n_ + 1);
            variable_number_list_.resize(m_ + n_ + 1);
            ev_to_newv_.resize(m_ + n_ + 1);
            v_to_newv_.resize(n_ + 1);
            newv_to_v_.resize(n_ + 1);
            for (int i = 0; i < m_; ++i) {
                const tdzdd::Graph::EdgeInfo& e = graph.edgeInfo(i);
                kind_list_[pos] = Kind::EDGE;
                variable_number_list_[pos] = i; // edge number
                --pos;
                if (e.v1final) {
                    kind_list_[pos] = Kind::VERTEX;
                    variable_number_list_[pos] = e.v1; // vertex number
                    ev_to_newv_[pos] = new_n;
                    v_to_newv_[e.v1] = new_n;
                    newv_to_v_[new_n] = e.v1;
                    --pos;
                    --new_n;
                }
                if (e.v2final) {
                    kind_list_[pos] = Kind::VERTEX;
                    variable_number_list_[pos] = e.v2; // vertex number
                    ev_to_newv_[pos] = new_n;
                    v_to_newv_[e.v2] = new_n;
                    newv_to_v_[new_n] = e.v2;
                    --pos;
                    --new_n;
                }
            }
            assert(pos == 0);
        }

        Kind getKind(int evindex) const
        {
            return kind_list_[evindex];
        }

        int getVariableNumber(int evindex) const
        {
            return variable_number_list_[evindex];
        }

        int evToNewV(int evindex) const
        {
            return ev_to_newv_[evindex];
        }

        int newVToV(int newv) const
        {
            return newv_to_v_[newv];
        }
    };

    static tdzdd::DdStructure<2> eToEvZdd(const tdzdd::DdStructure<2>& dd,
                                          const tdzdd::Graph& graph,
                                          const VariableList& vlist)
    {
        ConvEVDD::ZDDEVSpec spec(dd, graph, vlist);
        tdzdd::DdStructure<2> out_dd(spec);
        return out_dd;
    }

    static ZBDD eToVZdd(const tdzdd::DdStructure<2>& dd,
                        const tdzdd::Graph& graph)
    {
        VariableList vlist(graph);
        return eToVZdd(dd, graph, vlist);
    }

    static ZBDD eToVZdd(const tdzdd::DdStructure<2>& dd,
                        const tdzdd::Graph& graph,
                        const VariableList& vlist,
                        const int offset=0)
    {
        tdzdd::DdStructure<2> ev_dd = ConvEVDD::eToEvZdd(dd, graph, vlist);
        return ev_dd.evaluate(EVToVEval(vlist, offset));
    }

    class ZDDEVSpec : public tdzdd::PodHybridDdSpec<ZDDEVSpec, ZDDEVSpecConf,
                                                    uint16_t, 2> {
    private:
        const tdzdd::Graph& graph_;
        tdzdd::DdStructure<2> dd_;
        const VariableList& vlist_;
        const int stateSize_;
        const int n_;
        const int m_;

    public:
        ZDDEVSpec(tdzdd::DdStructure<2> dd, const tdzdd::Graph& graph,
                  const VariableList& vlist)
            : graph_(graph), dd_(dd), vlist_(vlist),
              stateSize_(graph.vertexSize() + 1/*graph.maxFrontierSize()*/),
              n_(graph.vertexSize()),
              m_(graph.edgeSize())
        {
            this->setArraySize(stateSize_);
        }

        size_t hashCode(const ZDDEVSpecConf& conf) const {
            return conf.node.hash();
        }

        int getRoot(ZDDEVSpecConf& conf, uint16_t* state) const {
            conf.node = dd_.root();
            for (int i = 0; i < stateSize_; ++i) {
                state[i] = 0;
            }
            return ((conf.node == 1) ? -1 : m_ + n_);
        }

        int getChild(ZDDEVSpecConf& conf, uint16_t* state, int level,
                     int value) const {

            //std::cerr << "level = " << level << ", value = " << value << ", state = " << state[1] << state[2] << state[3] << std::endl;
            if (vlist_.getKind(level) == VariableList::Kind::EDGE) {
                int e_num = vlist_.getVariableNumber(level); // edge number
                if (conf.node.row() < m_ - e_num) {
                    if (value != 0) { // zero-suppressed rule
                        return 0;
                    }
                } else {
                    if (conf.node.row() > 0) { // non-terminal
                        conf.node = dd_.child(conf.node, value);
                    }
                    if (conf.node == 0) { // arrived at 0-terminal
                        return 0;
                    }
                }

                const tdzdd::Graph::EdgeInfo& edge =
                    graph_.edgeInfo(e_num);

                if (value == 1) { // update state
                    state[edge.v1] = 1;
                    state[edge.v2] = 1;
                }
            } else {
                assert(vlist_.getKind(level) == VariableList::Kind::VERTEX);
                int v = vlist_.getVariableNumber(level); // vertex number
                assert(1 <= v && v <= n_);
                if ((state[v] != 0) != (value != 0)) {
                    return 0;
                } else {
                    state[v] = 0;
                }
            }
            return (level - 1 > 0 ? level - 1 : -1);
        }
    };

    class ArrangeESpec : public tdzdd::DdSpec<ArrangeESpec,
                                              tdzdd::NodeId,
                                              2> {
    private:
        tdzdd::DdStructure<2> dd_;
        const VariableList& vlist_;
    public:
        ArrangeESpec(tdzdd::DdStructure<2> dd,
                     const VariableList& vlist)
            : dd_(dd), vlist_(vlist) { }

        int getRoot(tdzdd::NodeId& node) const {
            node = dd_.root();
            if (node.row() > 0) {
                return vlist_.evToNewV(node.row());
            } else {
                return -node.col();
            }
        }

        int getChild(tdzdd::NodeId& node, int level, int value) const {
            tdzdd::NodeId child = dd_.child(node, value);
            if (child.row() > 0) {
                node = child;
                return vlist_.evToNewV(node.row());
            } else {
                return -child.col();
            }
        }
    };

    class EVToVEval : public tdzdd::DdEval<EVToVEval, ZBDD> {
    private:
        const VariableList& vlist_;
        const int offset_;

    public:
        EVToVEval(const VariableList& vlist, const int offset)
            : vlist_(vlist), offset_(offset) { }

        void evalTerminal(ZBDD& zbdd, int id) const
        {
            zbdd = ZBDD(id);
        }

        void evalNode(ZBDD& zbdd, int level,
                      const tdzdd::DdValues<ZBDD, 2>& values) const
        {
            ZBDD z0 = values.get(0);
            ZBDD z1 = values.get(1);

            if (vlist_.getKind(level) == VariableList::Kind::EDGE) {
                zbdd = z0 + z1;
            } else {
                assert(vlist_.getKind(level) == VariableList::Kind::VERTEX);
                // cout << "original level: " << BDD_VarOfLev(vlist_.evToNewV(level)) << endl;
                // cout << "new level: " << BDD_VarOfLev(vlist_.evToNewV(level) + offset_) << endl;
                zbdd = z0 + z1.Change(BDD_VarOfLev(vlist_.evToNewV(level) + offset_));
            }
        }
    };

    class OnlyInducedEval : public tdzdd::DdEval<OnlyInducedEval, ZBDD> {
    public:
        void evalTerminal(ZBDD& zbdd, int id) const
        {
            zbdd = ZBDD(id);
        }

        void evalNode(ZBDD& zbdd, int level,
                      const tdzdd::DdValues<ZBDD, 3>& values) const
        {
            ZBDD z0 = values.get(0);
            ZBDD z1 = values.get(1);
            // values.get(2) is not used
            zbdd = z0 + z1.Change(BDD_VarOfLev(level));
        }
    };
};

#endif // CONVEVDD_HPP
