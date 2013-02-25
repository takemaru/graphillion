/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: CycleZdd.hpp 414 2013-02-15 04:04:57Z iwashita $
 */

#pragma once

#include "../dd/DdSpec.hpp"
#include "SimpathBasedImpl.hpp"

struct CycleZdd: public PodArrayDdSpec<CycleZdd,
        SimpathBasedImpl<Cycle,false,true>::Mate>, public SimpathBasedImpl<
        Cycle,false,true> {
    CycleZdd(Graph const& graph)
            : SimpathBasedImpl<Cycle,false,true>(graph) {
        setArraySize(mateArraySize());
    }
};
