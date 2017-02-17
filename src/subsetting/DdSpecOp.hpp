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

#include "op/BinaryOperation.hpp"
#include "op/Lookahead.hpp"
#include "op/Unreduction.hpp"

namespace tdzdd {

/**
 * Returns a BDD specification for logical AND of two BDD specifications.
 * @param spec1 the first BDD specification.
 * @param spec2 the second BDD specification.
 * @return BDD specification for logical AND of @p spec1 and @p spec2.
 */
template<typename S1, typename S2>
BddAnd<S1,S2> bddAnd(S1 const& spec1, S2 const& spec2) {
    return BddAnd<S1,S2>(spec1, spec2);
}

#if __cplusplus >= 201103L
/**
 * Returns a BDD specification for logical AND of two or more BDD specifications.
 * (since C++11)
 * @param specs BDD specifications.
 * @return BDD specification for logical AND of @p specs.
 */
template<typename... SS>
BddAnd<SS...> bddAnd(SS const&... specs) {
    return BddAnd<SS...>(specs...);
}
#endif

/**
 * Returns a BDD specification for logical OR of two BDD specifications.
 * @param spec1 the first BDD specification.
 * @param spec2 the second BDD specification.
 * @return BDD specification for logical OR of @p spec1 and @p spec2.
 */
template<typename S1, typename S2>
BddOr<S1,S2> bddOr(S1 const& spec1, S2 const& spec2) {
    return BddOr<S1,S2>(spec1, spec2);
}

#if __cplusplus >= 201103L
/**
 * Returns a BDD specification for logical OR of two or more BDD specifications.
 * (since C++11)
 * @param specs BDD specifications.
 * @return BDD specification for logical OR of @p specs.
 */
template<typename... SS>
BddOr<SS...> bddOr(SS const&... specs) {
    return BddOr<SS...>(specs...);
}
#endif

/**
 * Returns a ZDD specification for set intersection of two ZDD specifications.
 * @param spec1 the first ZDD specification.
 * @param spec2 the second ZDD specification.
 * @return ZDD specification for set intersection of @p spec1 and @p spec2.
 */
template<typename S1, typename S2>
ZddIntersection<S1,S2> zddIntersection(S1 const& spec1, S2 const& spec2) {
    return ZddIntersection<S1,S2>(spec1, spec2);
}

#if __cplusplus >= 201103L
/**
 * Returns a ZDD specification for set intersection of two or more ZDD specifications.
 * (since C++11)
 * @param specs ZDD specifications.
 * @return ZDD specification for set intersection of @p specs.
 */
template<typename... SS>
ZddIntersection<SS...> zddIntersection(SS const&... specs) {
    return ZddIntersection<SS...>(specs...);
}
#endif

/**
 * Returns a ZDD specification for set union of two ZDD specifications.
 * @param spec1 the first ZDD specification.
 * @param spec2 the second ZDD specification.
 * @return ZDD specification for set union of @p spec1 and @p spec2.
 */
template<typename S1, typename S2>
ZddUnion<S1,S2> zddUnion(S1 const& spec1, S2 const& spec2) {
    return ZddUnion<S1,S2>(spec1, spec2);
}

#if __cplusplus >= 201103L
/**
 * Returns a ZDD specification for set union of two or more ZDD specifications.
 * (since C++11)
 * @param specs specifications.
 * @return ZDD specification for set union of @p specs.
 */
template<typename... SS>
ZddUnion<SS...> zddUnion(SS const&... specs) {
    return ZddUnion<SS...>(specs...);
}
#endif

/**
 * Optimizes a BDD specification in terms of the BDD node deletion rule.
 * @param spec original BDD specification.
 * @return optimized BDD specification.
 */
template<typename S>
BddLookahead<S> bddLookahead(S const& spec) {
    return BddLookahead<S>(spec);
}

/**
 * Optimizes a ZDD specification in terms of the ZDD node deletion rule.
 * @param spec original ZDD specification.
 * @return optimized ZDD specification.
 */
template<typename S>
ZddLookahead<S> zddLookahead(S const& spec) {
    return ZddLookahead<S>(spec);
}

/**
 * Creates a QDD specification from a BDD specification by complementing
 * skipped nodes in terms of the BDD node deletion rule.
 * @param spec BDD specification.
 * @return QDD specification.
 */
template<typename S>
BddUnreduction<S> bddUnreduction(S const& spec, int numVars) {
    return BddUnreduction<S>(spec, numVars);
}

/**
 * Creates a QDD specification from a ZDD specification by complementing
 * skipped nodes in terms of the ZDD node deletion rule.
 * @param spec ZDD specification.
 * @return QDD specification.
 */
template<typename S>
ZddUnreduction<S> zddUnreduction(S const& spec, int numVars) {
    return ZddUnreduction<S>(spec, numVars);
}

} // namespace tdzdd
