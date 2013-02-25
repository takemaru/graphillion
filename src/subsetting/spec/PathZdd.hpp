/*
 * Top-Down ZDD Construction Library for Frontier-Based Search
 * by Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2012 Japan Science and Technology Agency
 * $Id: PathZdd.hpp 414 2013-02-15 04:04:57Z iwashita $
 */

#pragma once

#include "SimpathBasedImpl.hpp"

struct PathZdd: public PodArrayDdSpec<PathZdd,
        SimpathBasedImpl<Path,false,true>::Mate>, public SimpathBasedImpl<Path,
        false,true> {
    PathZdd(Graph const& graph)
            : SimpathBasedImpl<Path,false,true>(graph) {
        setArraySize(mateArraySize());
    }
};
