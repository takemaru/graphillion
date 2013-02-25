/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: HamiltonPathZdd.hpp 414 2013-02-15 04:04:57Z iwashita $
 */

#pragma once

#include "SimpathBasedImpl.hpp"

struct HamiltonPathZdd: public PodArrayDdSpec<HamiltonPathZdd,
        SimpathBasedImpl<Path,true,true>::Mate>, public SimpathBasedImpl<Path,
        true,true> {
    HamiltonPathZdd(Graph const& graph)
            : SimpathBasedImpl<Path,true,true>(graph) {
        setArraySize(mateArraySize());
    }
};
