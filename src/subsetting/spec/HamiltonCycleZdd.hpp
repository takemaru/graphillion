/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: HamiltonCycleZdd.hpp 414 2013-02-15 04:04:57Z iwashita $
 */

#pragma once

#include "SimpathBasedImpl.hpp"

struct HamiltonCycleZdd: public PodArrayDdSpec<HamiltonCycleZdd,
        SimpathBasedImpl<Cycle,true,true>::Mate>, public SimpathBasedImpl<Cycle,
        true,true> {
    HamiltonCycleZdd(Graph const& graph)
            : SimpathBasedImpl<Cycle,true,true>(graph) {
        setArraySize(mateArraySize());
    }
};
