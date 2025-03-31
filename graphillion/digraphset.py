# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

"""Module for a set of directed graphs.
"""

from functools import partial
import _graphillion
from graphillion.universe import ObjectTable
from graphillion.setset_base import setset_base
import pickle
import heapq


class DiGraphSet(object):
    """Represents and manipulates a set of directed graphs (digraphs).

    A DiGraphSet object stores a set of digraphs.  A graph stored must be
    a subgraph of the universal digraph, and is represented by a list of
    edges in the universal digraph.  An edge is a tuple of two vertices,
    and a vertex can be any hashable object like a number, a text
    string, and a tuple.

    The universal digraph must be defined before creating DiGraphSet
    objects by `DiGraphSet.universe()` method.

    Like Python set types, DiGraphSet supports `graph in digraphset`,
    `len(digraphset)`, and `for graph in digraphset`.  It also supports
    all set methods and operators,
    * isdisjoint(), issubset(), issuperset(), union(), intersection(),
      difference(), symmetric_difference(), copy(), update(),
      intersection_update(), difference_update(),
      symmetric_difference_update(), add(), remove(), discard(),
      pop(), clear(),
    * ==, !=, <=, <, >=, >, |, &, -, ^, |=, &=, -=, ^=.

    Examples:
      >>> from graphillion import DiGraphSet

      We assume the following graph and register the edge list as the
      universe.

      1 <-> 2 <-> 3
      ^     ^     ^
      |     |     |
      v     v     v
      4 <-> 5 <-> 6

      >>> universe = [(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5), (5, 6),
                      (2, 1), (4, 1), (3, 2), (5, 2), (6, 3), (5, 4), (6, 5)]
      >>> DiGraphSet.set_universe(universe)

      Find all paths from 1 to 6 and count them.

      >>> paths = DiGraphSet.directed_paths(1, 6)
      >>> len(paths)
      4

      Give constraints in which edge 1-4 must not be passed but 2 must
      be passed, and show the paths that meet the constraints.

      >>> paths = paths.excluding((1, 4)).including(2)
      >>> for path in paths:
      ...   path
      [(1, 2), (2, 3), (3, 6)]
      [(1, 2), (2, 5), (5, 6)]
    """

    def __init__(self, digraphset_or_constraints=None):
        """Initializes a DiGraphSet object with a set of digraphs or constraints.

        Examples:
          >>> graph1 = [(1, 4)]
          >>> graph2 = [(1, 2), (2, 3)]
          >>> DiGraphSet([graph1, graph2])
          DiGraphSet([[(1, 4)], [(1, 2), (2, 3)]])
          >>> DiGraphSet({'include': graph1, 'exclude': graph2})
          DiGraphSet([[(1, 4)], [(1, 4), (2, 5)], [(1, 4), (3, 6)], ...

        Args:
          digraphset_or_constraints: A set of digraphs represented by a
            list of graphs (a list of edge lists):

            [[(1, 4)], [(1, 2), (2, 3)]]

            Or constraints represented by a dict of included or
            excluded edge lists (not-specified edges are not cared):

            {'include': [(1, 4)], 'exclude': [(1, 2), (2, 3)]}

            If no argument is given, it is treated as an empty list
            `[]` and an empty DiGraphSet is returned.  An empty dict
            `{}` means that no constraint is specified, and so a
            DiGraphSet including all possible digraphs in the universe is
            returned (let N the number of edges in the universe, 2^N
            digraphs are stored in the new object).

        Raises:
          KeyError: If given edges are not found in the universe.

        See Also:
          copy()
        """
        obj = digraphset_or_constraints
        if isinstance(obj, DiGraphSet):
            self._ss = obj._ss.copy()
        elif isinstance(obj, setset_base):
            self._ss = obj.copy()
        else:
            if obj is None:
                obj = []
            # a list of digraphs [graph+]
            elif isinstance(obj, (set, frozenset, list)):
                l = []
                for g in obj:
                    edges = DiGraphSet.converters['to_edges'](g)
                    l.append(set([DiGraphSet._conv_edge(e) for e in edges]))
                obj = l
            elif isinstance(obj, dict):  # constraints
                d = {}
                for k, l in obj.items():
                    d[k] = [DiGraphSet._conv_edge(e) for e in l]
                obj = d
            self._ss = setset_base(DiGraphSet._objtable, obj)
        methods = ['directed_cycles',
                   'directed_hamiltonian_cycles', 'directed_st_paths', 'rooted_forests',
                   'rooted_trees', 'graphs']
        for method in methods:
            setattr(self, method, partial(
                getattr(DiGraphSet, method), DiGraphSet=self))

    def copy(self):
        """Returns a new DiGraphSet with a shallow copy of `self`.

        Examples:
          >>> gs2 = gs1.copy()
          >>> gs1 -= gs2
          >>> gs1 == gs2
          False

        Returns:
          A new DiGraphSet object.

        See Also:
          __init__()
        """
        return DiGraphSet(self)

    def __bool__(self):
        return bool(self._ss)

    def __repr__(self):
        return setset_base._repr(self._ss, DiGraphSet._objtable, (self.__class__.__name__ + '([', '])'))

    def union(self, *others):
        """Returns a new DiGraphSet with digraphs from `self` and all others.

        The `self` is not changed.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph2, graph3])
          >>> gs1 | gs2
          DiGraphSet([[], [(1, 2)], [(1, 2), (1, 4)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          intersection(), difference(), symmetric_difference(),
          update()
        """
        return DiGraphSet(self._ss.union(*[gs._ss for gs in others]))

    def intersection(self, *others):
        """Returns a new DiGraphSet with digraphs common to `self` and all others.

        The `self` is not changed.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph2, graph3])
          >>> gs1 & gs2
          DiGraphSet([[(1, 2)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          union(), difference(), symmetric_difference(),
          intersection_update()
        """
        return DiGraphSet(self._ss.intersection(*[gs._ss for gs in others]))

    def difference(self, *others):
        """Returns a new DiGraphSet with digraphs in `self` that are not in the others.

        The `self` is not changed.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph2, graph3])
          >>> gs1 - gs2
          DiGraphSet([])

        Returns:
          A new DiGraphSet object.

        See Also:
          union(), intersection(), symmetric_difference(),
          difference_update()
        """
        return DiGraphSet(self._ss.difference(*[gs._ss for gs in others]))

    def symmetric_difference(self, *others):
        """Returns a new DiGraphSet with digraphs in either `self` or `other` but not both.

        The `self` is not changed.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph2, graph3])
          >>> gs1 ^ gs2
          DiGraphSet([[], [(1, 2), (1, 4)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          union(), intersection(), difference(), 
          symmetric_difference_update()
        """
        return DiGraphSet(self._ss.symmetric_difference(*[gs._ss for gs in others]))

    def quotient(self, other):
        """Returns a new DiGraphSet of quotient.

        The quotient is defined by,
          gs1 / gs2 = {a | a \\cup b \\in gs1 and a \\cap b = \\empty, \\forall b \\in gs2}.
        D. Knuth, Exercise 204, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3), (2, 5)]
          >>> graph3 = [(1, 4)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> gs / DiGraphSet([graph3])
          DiGraphSet([[(1, 2)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          remainder(), quotient_update()
        """
        return DiGraphSet(self._ss.quotient(other._ss))

    def remainder(self, other):
        """Returns a new DiGraphSet of remainder.

        The remainder is defined by,
          gs1 % gs2 = gs1 - (gs1 \\sqcup (gs1 / gs2)).
        D. Knuth, Exercise 204, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3), (2, 5)]
          >>> graph3 = [(1, 4)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> gs % DiGraphSet([graph3])
          DiGraphSet([[(2, 3), (2, 5)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          quotient(), remainder_update()
        """
        return DiGraphSet(self._ss.remainder(other._ss))

    def update(self, *others):
        """Updates `self`, adding digraphs from all others.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph2, graph3])
          >>> gs1 |= gs2
          >>> gs1
          DiGraphSet([[], [(1, 2)], [(1, 2), (1, 4)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          union()
        """
        self._ss.update(*[gs._ss for gs in others])
        return self

    def intersection_update(self, *others):
        """Updates `self`, keeping only digraphs found in it and all others.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph2, graph3])
          >>> gs1 &= gs2
          >>> gs1
          DiGraphSet([[(1, 2)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          intersection()
        """
        self._ss.intersection_update(*[gs._ss for gs in others])
        return self

    def difference_update(self, *others):
        """Update `self`, removing digraphs found in others.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph2, graph3])
          >>> gs1 -= gs2
          >>> gs1
          DiGraphSet([[]])

        Returns:
          A new DiGraphSet object.

        See Also:
          difference()
        """
        self._ss.difference_update(*[gs._ss for gs in others])
        return self

    def symmetric_difference_update(self, *others):
        """Update `self`, keeping only digraphs in either DiGraphSet, but not in both.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph2, graph3])
          >>> gs1 ^= gs2
          >>> gs1
          DiGraphSet([[], [(1, 2), (1, 4)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          symmetric_difference()
        """
        self._ss.symmetric_difference_update(*[gs._ss for gs in others])
        return self

    def quotient_update(self, other):
        """Updates `self` by the quotient.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3), (2, 5)]
          >>> graph3 = [(1, 4)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> gs /= DiGraphSet([graph3])
          >>> gs
          DiGraphSet([[(1, 2)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          quotient()
        """
        self._ss.quotient_update(other._ss)
        return self

    def remainder_update(self, other):
        """Updates `self` by the remainder.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3), (2, 5)]
          >>> graph3 = [(1, 4)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> gs %= DiGraphSet([graph3])
          >>> gs
          DiGraphSet([[(2, 3), (2, 5)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          remainder()
        """
        self._ss.remainder_update(other._ss)
        return self

    def __invert__(self):
        """Returns a new DiGraphSet with digraphs not stored in `self`.

        Examples:
          >>> DiGraphSet.set_universe([(1, 2), (1, 4)])
          >>> graph = [(1, 2)]
          >>> gs = DiGraphSet([graph])
          >>> ~gs
          DiGraphSet([[], [(1, 4)], [(1, 2), (1, 4)]])

        Returns:
          A new DiGraphSet object.
        """
        return DiGraphSet(self._ss._invert(DiGraphSet._objtable))

    __or__ = union
    __and__ = intersection
    __sub__ = difference
    __xor__ = symmetric_difference
    __div__ = quotient
    __truediv__ = quotient
    __floordiv__ = quotient
    __mod__ = remainder

    __ior__ = update
    __iand__ = intersection_update
    __isub__ = difference_update
    __ixor__ = symmetric_difference_update
    __idiv__ = quotient_update
    __itruediv__ = quotient_update
    __ifloordiv__ = quotient_update
    __imod__ = remainder_update

    def isdisjoint(self, other):
        """Returns True if `self` has no digraphs in common with `other`.

        Examples:
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph3, graph4, graph5])
          >>> gs1.disjoint(gs2)
          True

        Returns:
          True or False.

        See Also:
          issubset(), issuperset()
        """
        return self._ss.isdisjoint(other._ss)

    def issubset(self, other):
        """Tests if every digraph in `self` is in `other`.

        Examples:
          >>> gs1 = DiGraphSet([graph1, graph3])
          >>> gs2 = DiGraphSet([graph1, graph2, graph3])
          >>> gs1 <= gs2
          True

        Returns:
          True or False.

        See Also:
          issuperset(), isdisjoint()
        """
        return self._ss.issubset(other._ss)

    def issuperset(self, other):
        """Tests if every digraph in `other` is in `self`.

        Examples:
          >>> gs1 = DiGraphSet([graph1, graph2, graph3])
          >>> gs2 = DiGraphSet([graph1, graph3])
          >>> gs1 >= gs2
          True

        Returns:
          True or False.

        See Also:
          issubset(), isdisjoint()
        """
        return self._ss.issuperset(other._ss)

    __le__ = issubset
    __ge__ = issuperset

    def __lt__(self, other):
        """Tests if `self` is a true subset of `other`.

        This method returns False when `self` == `other`, unlike
        issubset.

        Examples:
          >>> gs < gs
          False

        Returns:
          True or False.

        See Also:
          issubset(), issuperset(), isdisjoint()
        """
        return self._ss < other._ss

    def __gt__(self, other):
        """Test if `self` is a true superset of `other`.

        This method returns False when `self` == `other`, unlike
        issuperset.

        Examples:
          >>> gs > gs
          False

        Returns:
          True or False.

        See Also:
          issubset(), issuperset(), isdisjoint()
        """
        return self._ss > other._ss

    def __eq__(self, other):
        return self._ss == other._ss

    def __ne__(self, other):
        return self._ss != other._ss

    def __len__(self):
        """Returns the number of digraphs in `self`.

        Use gs.len() if OverflowError raised.

        Examples:
          >>> gs = DiGraphSet([graph1, graph2])
          >>> len(gs)
          2

        Returns:
          The number of digraphs.

        Raises:
          OverflowError

        See Also:
          len()
        """
        return len(self._ss)

    def len(self, size=None):
        """Returns the number of digraphs, or a new DiGraphSet with `size` edges.

        If no argument is given, this method returns the number of
        digraphs in `self`.  Otherwise, this method returns a new
        DiGraphSet with digraphs that have `size` edges; this usage of
        `len(size)` is obsoleted, and use `graph_size(size)` instead.

        This method never raises OverflowError unlike built-in len(gs).

        Examples:
          >>> gs = DiGraphSet([graph1, graph2])
          >>> gs.len()
          2L

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(1, 2), (1, 4), (2, 3)]
          >>> gs = DiGraphSet([graph1, graph2, graph3])
          >>> gs.len(2)
          DiGraphSet([[(1, 2), (1, 4)]])

        Args:
          size: Optional.  The number of edges in a digraph.

        Returns:
          The number of digraphs, or a new DiGraphSet object.

        See Also:
          __len__(), smaller(), larger(), graph_size()

        """
        if size is None:
            return self._ss.len()
        else:
            return self.graph_size(size)

    def __iter__(self):
        """Iterates over digraphs.

        This is the fastest iterator among DiGraphSet iterators, such as
        rand_iter() and max_iter().

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> for g in gs:
          ...   g
          [(1, 2), (1, 4)]
          [(1, 2)]

        Returns:
          A generator.

        Yields:
          A digraph.

        See Also:
          rand_iter(), max_iter(), min_iter()
        """
        for g in self._ss._iter(DiGraphSet._objtable):
            try:
                yield DiGraphSet._conv_ret(g)
            except StopIteration:
                return

    def rand_iter(self):
        """Iterates over digraphs uniformly randomly.

        This method relies on its own random number generator, doesn't
        rely on Python random module.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> for g in gs.rand_iter():
          ...   g
          [(1, 2)]
          [(1, 2), (1, 4)]

        Returns:
          A generator.

        Yields:
          A digraph.

        See Also:
          __iter__(), max_iter(), min_iter()
        """
        for g in self._ss.rand_iter(DiGraphSet._objtable):
            try:
                yield DiGraphSet._conv_ret(g)
            except StopIteration:
                return

    def min_iter(self, weights=None):
        """Iterates over digraphs in the ascending order of weights.

        Returns a generator that iterates over digraphs in `self`
        DiGraphSet.  The digraphs are selected in the ascending order of
        edge weights, which are specified by the argument `weights` or
        those set as the universe (1.0 for unspecified edges).  The
        `weights` does not overwrite the weights of the universe.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> weights = {(1, 2): 2.0, (1, 4): -3.0}  # (2, 3): 1.0
          >>> for g in gs.min_iter(weights):
          ...   g
          [(1, 2), (1, 4)]
          [(2, 3)]

        Args:
          weights: Optional.  A dictionary of edges to the weight
            values.

        Returns:
          A generator.

        Yields:
          A digraph.

        See Also:
          __iter__(), rand_iter(), max_iter()

        """
        if weights is None:
            weights = DiGraphSet._weights
        for g in self._ss.min_iter(DiGraphSet._objtable, weights):
            try:
                yield DiGraphSet._conv_ret(g)
            except StopIteration:
                return

    def max_iter(self, weights=None):
        """Iterates over digraphs in the descending order of weights.

        Returns a generator that iterates over digraphs in `self`
        DiGraphSet.  The digraphs are selected in the descending order of
        edge weights, which are specified by the argument `weights` or
        those set as the universe (1.0 for unspecified edges).  The
        `weights` does not overwrite the weights of the universe.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> weights = {(1, 2): 2.0, (1, 4): -3.0}  # (2, 3): 1.0
          >>> for g in gs.max_iter(weights):
          ...   g
          [(2, 3)]
          [(1, 2), (1, 4)]

        Args:
          weights: Optional.  A dictionary of edges to the weight
            values.

        Returns:
          A generator.

        Yields:
          A digraph.

        See Also:
          __iter__(), rand_iter(), min_iter()
        """
        if weights is None:
            weights = DiGraphSet._weights
        for g in self._ss.max_iter(DiGraphSet._objtable, weights):
            try:
                yield DiGraphSet._conv_ret(g)
            except StopIteration:
                return

    def __contains__(self, obj):
        """Returns True if `obj` is in the `self`, False otherwise.

        Use the expression `obj in gs`.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> graph1 in gs
          True

        Args:
          obj: A digraph (an edge list), an edge, or a vertex in the
            universe.

        Returns:
          True or False.

        Raises:
          KeyError: If the given object is not found in the universe.
        """
        type, obj = DiGraphSet._conv_arg(obj)
        if type == 'graph' or type == 'edge':
            return self._ss._contains(DiGraphSet._objtable, obj)
        elif type == 'vertex':
            return len([e for e in obj if self._ss._contains(DiGraphSet._objtable, e)]) > 0
        raise TypeError(obj)

    def add(self, graph_or_edge):
        """Adds a given digraph or edge to `self`.

        If a digraph is given, the digraph is just added to `self`
        DiGraphSet.  If an edge is given, the edge is grafted to all the
        digraphs in `self`.  The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> edge = (1, 2)
          >>> gs.add(edge)
          >>> gs
          DiGraphSet([[(1, 2), (1, 4)], [(1, 2), (2, 3)]])

        Args:
          graph_or_edge: A digraph (an edge list) or an edge in the
            universe.

        Returns:
          None.

        Raises:
          KeyError: If a given edge or vertex is not found in the
            universe.

        See Also:
          remove(), discard()
        """
        type, obj = DiGraphSet._conv_arg(graph_or_edge)
        if type == 'graph' or type == 'edge':
            self._ss.add(DiGraphSet._objtable, obj)
        else:
            raise TypeError(graph_or_edge)

    def remove(self, obj):
        """Removes a given digraph, edge, or vertex from `self`.

        If a digraph is given, the digraph is just removed from `self`
        DiGraphSet.  If an edge is given, the edge is removed from all
        the digraphs in `self`.  The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> edge = (1, 2)
          >>> gs.remove(edge)
          >>> gs
          DiGraphSet([[(1, 4)], [(2, 3)]])

        Args:
          obj: A digraph (an edge list), an edge, or a vertex in the
            universe.

        Returns:
          None.

        Raises:
          KeyError: If a given edge or vertex is not found in the
            universe, or if the given digraph is not stored in `self`.

        See Also:
          add(), discard(), pop()
        """
        type, obj = DiGraphSet._conv_arg(obj)
        if type == 'graph' or type == 'edge':
            self._ss.remove(DiGraphSet._objtable, obj)
        elif type == 'vertex':
            for edge in obj:
                self.remove(edge)
        else:
            raise TypeError(obj)
        return None

    def discard(self, obj):
        """Removes a given digraph, edge, or vertex from `self`.

        If a digraph is given, the digraph is just removed from `self`
        DiGraphSet.  If an edge is given, the edge is removed from all
        the digraphs in `self`.  The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> edge = (1, 2)
          >>> gs.discard(edge)
          >>> gs
          DiGraphSet([[(1, 4)], [(2, 3)]])

        Args:
          obj: A digraph (an edge list), an edge, or a vertex in the
            universe.

        Returns:
          None.

        Raises:
          KeyError: If a given edge or vertex is not found in the
            universe.

        See Also:
          add(), remove(), pop()
        """
        type, obj = DiGraphSet._conv_arg(obj)
        if type == 'graph' or type == 'edge':
            self._ss.discard(DiGraphSet._objtable, obj)
        elif type == 'vertex':
            for edge in obj:
                self.discard(edge)
        else:
            raise TypeError(obj)
        return None

    def pop(self):
        """Removes and returns an arbitrary digraph from `self`.

        The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> gs.pop()
          [(1, 2), (1, 4)]
          >>> gs
          DiGraphSet([[(2, 3)]])

        Returns:
          A digraph.

        Raises:
          KeyError: If `self` is empty.

        See Also:
          remove(), discard(), choice()
        """
        return DiGraphSet._conv_ret(self._ss.pop(DiGraphSet._objtable))

    def clear(self):
        """Removes all digraphs from `self`.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> gs.clear()
          >>> gs
          DiGraphSet([])
        """
        return self._ss.clear()

    def flip(self, edge):
        """Flips the state of a given edge over all digraphs in `self`.

        If a digraph in `self` includes the given edge, the edge is
        removed from the digraph.  If a digraph in `self` does not include
        the given edge, the edge is added to the digraph.

        The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> edge = (1, 2)
          >>> gs.flip(edge)
          >>> gs
          DiGraphSet([[(1, 4)], [(1, 2), (2, 3)]])

        Args:
          edge: An edge in the universe.

        Returns:
          A new DiGraphSet object.

        Raises:
          KeyError: If a given edge is not found in the universe.
        """
        type, obj = DiGraphSet._conv_arg(edge)
        if type == 'edge':
            self._ss.flip(DiGraphSet._objtable, edge)
        else:
            raise TypeError(edge)

    def minimal(self):
        """Returns a new DiGraphSet of minimal digraphs.

        The minimal sets are defined by,
          gs.minimal() = {a \\in gs | b \\in gs and a \\subseteq -> a = b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(1, 4), (2, 3)]
          >>> gs = DiGraphSet([graph1, graph2, graph3])
          >>> gs.minimal()
          DiGraphSet([[(1, 2)], [(1, 4), (2, 3)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          maximal(), blocking()
        """
        return DiGraphSet(self._ss.minimal())

    def maximal(self):
        """Returns a new DiGraphSet of maximal digraphs.

        The maximal sets are defined by,
          gs.maximal() = {a \\in gs | b \\in gs and a \\superseteq -> a = b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(1, 4), (2, 3)]
          >>> gs = DiGraphSet([graph1, graph2, graph3])
          >>> gs.maximal()
          DiGraphSet([[(1, 2), (1, 4)], [(1, 4), (2, 3)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          minimal()
        """
        return DiGraphSet(self._ss.maximal())

    def blocking(self):
        """Returns a new DiGraphSet of all blocking (hitting) sets.

        A blocking set is often called a hitting set; all digraphs in
        `self` contain at least one edge in the set.  This implies
        that all the digraphs are destroyed by removing edges in the
        set.

        The blocking sets are defined by,
          gs.blocking() = {a | b \\in gs -> a \\cap b \\neq \\empty}.
        T. Toda, Hypergraph Dualization Algorithm Based on Binary
        Decision Diagrams.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(1, 4), (2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> gs.blocking().minimal()
          DiGraphSet([[(1, 4)], [(1, 2), (2, 3)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          minimal()
        """
        return DiGraphSet(self._ss.hitting(DiGraphSet._objtable))

    hitting = blocking

    def smaller(self, size):
        """Returns a new DiGraphSet with digraphs that have less than `size` edges.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(1, 2), (1, 4), (2, 3)]
          >>> gs = DiGraphSet([graph1, graph2, graph3])
          >>> gs.smaller(2)
          DiGraphSet([[(1, 2)]])

        Args:
          size: The number of edges in a digraph.

        Returns:
          A new DiGraphSet object.

        See Also:
          larger(), graph_size()
        """
        return DiGraphSet(self._ss.smaller(size))

    def larger(self, size):
        """Returns a new DiGraphSet with digraphs that have more than `size` edges.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(1, 2), (1, 4), (2, 3)]
          >>> gs = DiGraphSet([graph1, graph2, graph3])
          >>> gs.larger(2)
          DiGraphSet([[(1, 2), (1, 4), (2, 3)]])

        Args:
          size: The number of edges in a digraph.

        Returns:
          A new DiGraphSet object.

        See Also:
          smaller(), graph_size()
        """
        return DiGraphSet(self._ss.larger(size))

    def graph_size(self, size):
        """Returns a new DiGraphSet with `size` edges.

        This method returns a new DiGraphSet with digraphs that have
        `size` edges.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(1, 2), (1, 4), (2, 3)]
          >>> gs = DiGraphSet([graph1, graph2, graph3])
          >>> gs.graph_size(2)
          DiGraphSet([[(1, 2), (1, 4)]])

        Args:
          size: The number of edges in a digraph.

        Returns:
          A new DiGraphSet object.

        See Also:
          smaller(), larger()

        """
        return DiGraphSet(self._ss.set_size(size))

    def complement(self):
        """Returns a new DiGraphSet with complement digraphs of `self`.

        The `self` is not changed.

        Examples:
          >>> DiGraphSet.set_universe([(1, 2), (1, 4)])
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> gs.complement()
          DiGraphSet([[], [(1, 4)]])

        Returns:
          A new DiGraphSet object.
        """
        ss = self._ss.copy()
        ss.flip(DiGraphSet._objtable)
        return DiGraphSet(ss)

    def join(self, other):
        """Returns a new DiGraphSet of join between `self` and `other`.

        The join operation is defined by,
          gs1 \\sqcup gs2 = {a \\cup b | a \\in gs1 and b \\in gs2}.
        D. Knuth, Exercise 203, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(2, 3)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph3])
          >>> gs1.join(gs2)
          DiGraphSet([[(1, 2), (2, 3)], [(1, 2), (1, 4), (2, 3)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          meet()
        """
        return DiGraphSet(self._ss.join(other._ss))

    def meet(self, other):
        """Returns a new DiGraphSet of meet between `self` and `other`.

        The meet operation is defined by,
          gs1 \\sqcap gs2 = {a \\cap b | a \\in gs1 and b \\in gs2}.
        D. Knuth, Exercise 203, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(1, 2), (2, 3)]
          >>> graph3 = [(1, 4), (2, 3)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph3])
          >>> gs1.meet(gs2)
          DiGraphSet([[(1, 4)], [(2, 3)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          join()
        """
        return DiGraphSet(self._ss.meet(other._ss))

    def subgraphs(self, other):
        """Returns a new DiGraphSet with subgraphs of a digraph in `other`.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = graph1 + [(2, 3)]
          >>> graph4 = [(1, 4), (2, 3)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph3, graph4])
          >>> gs1.subgraphs(gs2)
          DiGraphSet([[(1, 2)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          supersets(), non_subsets()
        """
        return DiGraphSet(self._ss.subsets(other._ss))

    def supergraphs(self, other):
        """Returns a new DiGraphSet with supergraphs of a digraph in `other`.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (2, 3)]
          >>> graph2 = [(1, 4), (2, 3)]
          >>> graph3 = [(1, 2)]          # graph1 - (2, 3)
          >>> graph4 = [(1, 2), (1, 4)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph3, graph4])
          >>> gs1.supergraphs(gs2)
          DiGraphSet([[(1, 2), (2, 3)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          subsets(), non_supersets()
        """
        return DiGraphSet(self._ss.supersets(DiGraphSet._objtable, other._ss))

    def non_subgraphs(self, other):
        """Returns a new DiGraphSet with digraphs that aren't subgraphs of any digraph in `other`.

        The `self` is not changed.

        The non_subsets are defined by,
          gs1.non_subsets(gs2) = {a \\in gs1 | b \\in gs2 -> a \\not\\subseteq b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(1, 2), (2, 3)]
          >>> graph4 = [(1, 4), (2, 3)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph3, graph4])
          >>> gs1.non_subgraphs(gs2)
          DiGraphSet([[(1, 2), (1, 4)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          non_supersets(), subsets()
        """
        return DiGraphSet(self._ss.non_subsets(other._ss))

    def non_supergraphs(self, other):
        """Returns a new DiGraphSet with digraphs that aren't supergraphs of any digraph in `other`.

        The `self` is not changed.

        The non_supersets are defined by,
          gs1.non_supersets(gs2) = {a \\in gs1 | b \\in gs2 -> a \\not\\superseteq b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        Examples:
          >>> graph1 = [(1, 2), (2, 3)]
          >>> graph2 = [(1, 4), (2, 3)]
          >>> graph3 = [(1, 2)]
          >>> graph4 = [(1, 2), (1, 4)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph3, graph4])
          >>> gs1.non_supergraphs(gs2)
          DiGraphSet([[(1, 4), (2, 3)]])

        Returns:
          A new DiGraphSet object.

        See Also:
          non_subsets(), supersets()
        """
        return DiGraphSet(self._ss.non_supersets(DiGraphSet._objtable, other._ss))

    def including(self, obj):
        """Returns a new DiGraphSet that includes supergraphs of `obj`.

        Returns a new set of digraphs that include `obj`, which can be a
        DiGraphSet, a digraph, an edge, or a vertex.  If `obj` is a
        DiGraphSet, a digraph returned includes *one of* digraphs in the
        given DiGraphSet.

        The digraphs stored in the new DiGraphSet are selected from `self`
        DiGraphSet.  The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> vertex = 4
          >>> gs.including(vertex)
          DiGraphSet([[(1, 2), (1, 4)]])

        Args:
          obj: A DiGraphSet, a digraph (an edge list), an edge, or a
            vertex.

        Returns:
          A new DiGraphSet object.

        Raises:
          KeyError: If a given edge or a vertex is not found in the
            universe.

        See Also:
          excluding()
        """
        type, obj = DiGraphSet._conv_arg(obj)
        if type == 'digraphset':
            return DiGraphSet(self._ss.supersets(DiGraphSet._objtable, obj._ss))
        elif type == 'graph':
            return self.including(DiGraphSet([obj]))
        elif type == 'edge':
            return DiGraphSet(self._ss.supersets(DiGraphSet._objtable, obj))
        else:
            return self.including(DiGraphSet([set([e]) for e in obj]))

    def excluding(self, obj):
        """Returns a new DiGraphSet that doesn't include `obj`.

        Returns a new set of digraphs that don't include `obj`, which
        can be a DiGraphSet, a digraph, an edge, or a vertex.  If `obj` is
        a DiGraphSet, a digraph returned doesn't include *any of* digraphs
        in the given DiGraphSet.

        The digraphs stored in the new DiGraphSet are selected from `self`
        DiGraphSet.  The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> vertex = 4
          >>> gs.excluding(vertex)
          DiGraphSet([[(2, 3)]])

        Args:
          obj: A DiGraphSet, a digraph (an edge list), an edge, or a
            vertex.

        Returns:
          A new DiGraphSet object.

        Raises:
          KeyError: If a given edge or vertex is not found in the
            universe.

        See Also:
          including()
        """
        type, obj = DiGraphSet._conv_arg(obj)
        if type == 'digraphset':
            #            return DiGraphSet(self._ss.non_supersets(obj._ss))  # correct but slow
            return self - self.including(obj)
        elif type == 'graph':
            return self.excluding(DiGraphSet([obj]))
        elif type == 'edge':
            return DiGraphSet(self._ss.non_supersets(DiGraphSet._objtable, obj))
        else:
            return self.excluding(DiGraphSet([set([e]) for e in obj]))

    def included(self, obj):
        """Returns a new DiGraphSet with subgraphs of a digraph in `obj`.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = graph1 + [(2, 3)]
          >>> graph4 = [(1, 4), (2, 3)]
          >>> gs1 = DiGraphSet([graph1, graph2])
          >>> gs2 = DiGraphSet([graph3, graph4])
          >>> gs1.included(gs2)
          DiGraphSet([[(1, 2)]])

        Args:
          obj: A DiGraphSet or a digraph (an edge list).

        Returns:
          A new DiGraphSet object.

        See Also:
          including()
        """
        type, obj = DiGraphSet._conv_arg(obj)
        if type == 'digraphset':
            return DiGraphSet(self._ss.subsets(obj._ss))
        elif type == 'graph':
            return self.included(DiGraphSet([obj]))
        else:
            raise TypeError(obj)

    def choice(self):
        """Returns an arbitrary digraph from `self`.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> gs.choice()
          [(1, 2), (1, 4)]

        Returns:
          A digraph.

        Raises:
          KeyError: If `self` is empty.

        See Also:
          pop()
        """
        return DiGraphSet._conv_ret(self._ss.choice(DiGraphSet._objtable))

    def probability(self, probabilities):
        """Returns the probability of `self` with edge `probabilities`.

        This method calculates the probability of occurrence of any
        digraph in `self` given `probabilities` of each edge.

        Examples:
          >>> GraphSet.set_universe([(1, 2), (1, 4), (2, 3)])
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> probabilities = {(1, 2): .9, (1, 4): .8, (2, 3): .7}
          >>> gs.probability(probabilities)
          0.23
          # 0.9*0.8*(1-0.7) + (1-0.9)*(1-0.8)*0.7

        Args:
          probabilities: A dictionary of probabilities of each edge.

        Returns:
          Probability.

        Raises:
          KeyError: If a given edge is not found in the universe.
        """
        return self._ss.probability(DiGraphSet._objtable, probabilities)

    def cost_le(self, costs, cost_bound):
        """Returns a new DiGraphSet with subgraphs whose cost is less than or equal to the cost bound.

        This method constructs a DiGraphSet of subgraphs
        whose cost is less than or equal to the cost bound,
        where `costs` of each edge and the `cost_bound` are given as arguments.

        Examples:
          >>> universe = [(1, 2), (1, 4), (2, 3), (3, 4)]
          >>> DiGraphSet.set_universe(universe)

          >>> graph1 = [(1, 2), (2, 3)]
          >>> graph2 = [(3, 4)]
          >>> graph3 = [(1, 2), (1, 4), (3, 4)]
          >>> gs = DiGraphSet([graph1, graph2, graph3])
          >>> costs = {(1, 2): 2, (1, 4): 3, (2, 3): 1, (3, 4): 7}
          >>> cost_bound = 7
          >>> print(gs.cost_le(costs, cost_bound))
          DiGraphSet([[(3, 4)], [(1, 2), (2, 3)]])

        Args:
          costs: A dictionary of the cost of each edge.
          cost_bound: The upper limit of the cost of each digraph. 32 bit signed integer.

        Returns:
          A new DiGraphSet object.

        Raises:
          KeyError: If a given edge is not found in the universe.
          AssertionError: If the cost of at least one edge is not given, or outside the range of 32 bit signed integer.
          TypeError: If at least one cost is not integer.

        """
        return DiGraphSet(self._ss.cost_le(objtable=DiGraphSet._objtable, costs=costs, cost_bound=cost_bound))

    def cost_ge(self, costs, cost_bound):
        """Returns a new DiGraphSet with subgraphs whose cost is greater than or equal to the cost bound.

        This method constructs a DiGraphSet of subgraphs
        whose cost is greater than or equal to the cost bound,
        where `costs` of each edge and the `cost_bound` are given as arguments.

        Examples:
          >>> universe = [(1, 2), (1, 4), (2, 3), (3, 4)]
          >>> DiGraphSet.set_universe(universe)

          >>> graph1 = [(1, 2), (2, 3)]
          >>> graph2 = [(3, 4)]
          >>> graph3 = [(1, 2), (1, 4), (3, 4)]
          >>> gs = DiGraphSet([graph1, graph2, graph3])
          >>> costs = {(1, 2): 2, (1, 4): 3, (2, 3): 1, (3, 4): 7}
          >>> cost_bound = 7
          >>> print(gs.cost_ge(costs, cost_bound))
          DiGraphSet([[(3, 4)], [(1, 2), (1, 4), (3, 4)]])

        Args:
          costs: A dictionary of the cost of each edge.
          cost_bound: The lower limit of the cost of each digraph. 32 bit signed integer.

        Returns:
          A new DiGraphSet object.

        Raises:
          KeyError: If a given edge is not found in the universe.
          AssertionError: If the cost of at least one edge is not given, or outside the range of 32 bit signed integer.
          TypeError: If at least one cost is not integer.

        """
        inv_costs = {e: -cost for e, cost in costs.items()}
        return DiGraphSet(self._ss.cost_le(objtable=DiGraphSet._objtable, costs=inv_costs, cost_bound=-cost_bound))

    def cost_eq(self, costs, cost_bound):
        """Returns a new DiGraphSet with subgraphs whose cost is equal to the cost bound.

        This method constructs a DiGraphSet of subgraphs
        whose cost is equal to the cost bound,
        where `costs` of each edge and the `cost_bound` are given as arguments.

        Examples:
          >>> universe = [(1, 2), (1, 4), (2, 3), (3, 4)]
          >>> DiGraphSet.set_universe(universe)

          >>> graph1 = [(1, 2), (2, 3)]
          >>> graph2 = [(3, 4)]
          >>> graph3 = [(1, 2), (1, 4), (3, 4)]
          >>> gs = DiGraphSet([graph1, graph2, graph3])
          >>> costs = {(1, 2): 2, (1, 4): 3, (2, 3): 1, (3, 4): 7}
          >>> cost_bound = 7
          >>> print(gs.cost_eq(costs, cost_bound))
          DiGraphSet([[(3, 4)]])

        Args:
          costs: A dictionary of the cost of each edge.
          cost_bound: The limit of the cost of each digraph. 32 bit signed integer.

        Returns:
          A new DiGraphSet object.

        Raises:
          KeyError: If a given edge is not found in the universe.
          AssertionError: If the cost of at least one edge is not given, or outside the range of 32 bit signed integer.
          TypeError: If at least one cost is not integer.

        """
        le_ss = self._ss.cost_le(objtable=DiGraphSet._objtable, costs=costs, cost_bound=cost_bound)
        lt_ss = self._ss.cost_le(objtable=DiGraphSet._objtable, costs=costs, cost_bound=cost_bound - 1)
        return DiGraphSet(le_ss.difference(lt_ss))

    def remove_some_edge(self):
        """Returns a new DiGraphSet with digraphs that are obtained by removing some edge from a digraph in `self`.

        The `self` is not changed.

        Examples:
          >>> DiGraphSet.set_universe([(1, 2), (1, 4), (2, 3)])
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> gs.remove_some_edge()
          DiGraphSet([[], [(1, 4)], [(1, 2)]])

        Returns:
          A new DiGraphSet object.
        """
        return DiGraphSet(self._ss.remove_some_element())

    def add_some_edge(self):
        """Returns a new DiGraphSet with digraphs that are obtained by adding some edge to a digraph in `self`.

        The `self` is not changed.

        Examples:
          >>> DiGraphSet.set_universe([(1, 2), (1, 4), (2, 3)])
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> gs.add_some_edge()
          DiGraphSet([[(1, 4), (2, 3)], [(1, 2), (2, 3)], [(1, 2), (1, 4), (2, 3)]])

        Returns:
          A new DiGraphSet object.
        """
        return DiGraphSet(self._ss.add_some_element(DiGraphSet._objtable))

    def remove_add_some_edges(self):
        """Returns a new DiGraphSet with digraphs that are obtained by removing some edge from a digraph in `self` and adding another edge to the digraph.

        The `self` is not changed.

        Examples:
          >>> DiGraphSet.set_universe([(1, 2), (1, 4), (2, 3)])
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = DiGraphSet([graph1, graph2])
          >>> gs.remove_add_some_edges()
          DiGraphSet([[(1, 4)], [(1, 2)], [(1, 4), (2, 3)], [(1, 2), (2, 3)]])

        Returns:
          A new DiGraphSet object.
        """
        return DiGraphSet(self._ss.remove_add_some_elements(DiGraphSet._objtable))

    def dump(self, fp):
        """Serialize `self` to a file `fp`.

        This method does not serialize the universe, which should be
        saved separately by pickle.

        Examples:
          >>> import pickle
          >>> fp = open('/path/to/DiGraphSet', 'wb')
          >>> gs.dump(fp)
          >>> fp = open('/path/to/universe' 'wb')
          >>> pickle.dump(DiGraphSet.universe(), fp)

        Args:
          fp: A write-supporting file-like object.

        See Also:
          dumps(), load()
        """
        return self._ss.dump(fp)

    def dumps(self):
        """Returns a serialized `self`.

        This method does not serialize the universe, which should be
        saved separately by pickle.

        Examples:
          >>> import pickle
          >>> DiGraphSet_str = gs.dumps()
          >>> universe_str = pickle.dumps(DiGraphSet.universe())

        See Also:
          dump(), loads()
        """
        return self._ss.dumps()

    @staticmethod
    def load(fp):
        """Deserialize a file `fp` to `self`.

        This method does not deserialize the universe, which should be
        loaded separately by pickle.

        Args:
          fp: A read-supporting file-like object.

        Examples:
          >>> import pickle
          >>> fp = open('/path/to/universe')
          >>> DiGraphSet.set_universe(pickle.load(fp), traversal='as-is')
          >>> fp = open('/path/to/DiGraphSet')
          >>> gs = DiGraphSet.load(fp)

        See Also:
          loads(), dump()
        """
        return DiGraphSet(setset_base.load(fp))

    @staticmethod
    def loads(s):
        """Deserialize `s` to `self`.

        This method does not deserialize the universe, which should be
        loaded separately by pickle.

        Args:
          s: A string instance.

        Examples:
          >>> import pickle
          >>> DiGraphSet.set_universe(pickle.loads(universe_str), traversal='as-is')
          >>> gs = DiGraphSet.load(DiGraphSet_str)

        See Also:
          load(), dumps()
        """
        return DiGraphSet(setset_base.loads(s))

    @staticmethod
    def set_universe(universe, traversal='greedy', source=None):
        """Registers the new universe.

        Examples:
          >>> DiGraphSet.set_universe([(1, 2, 2.0), (1, 4, -3.0), (2, 3)])

        Args:
          universe: A list of edges that represents the new universe.
            An edge may come along with an edge weight, which can be
            positive as well as negative (or 1.0 if not specified).

          traversal: Optional.  This argument specifies the order of
            edges to be processed in the internal DiGraphSet operations.
            The default is 'greedy', best-first search from `source`
            with respect to the number of unused incident edges.
            Other options include 'bfs', the breadth-first search, 
            'dfs', the depth-first search, and 'as-is', the order of
            `universe` list.

          source: Optional.  This argument specifies the starting
            point of the edge traversal.

        See Also:
          universe()
        """
        sorted_edges = []
        indexed_edges = set()
        DiGraphSet._vertices = set()
        DiGraphSet._weights = {}
        universe = DiGraphSet.converters['to_edges'](universe)
        for e in universe:
            if e[:2] in indexed_edges:  # directed
                raise KeyError(e)
            sorted_edges.append(e[:2])
            indexed_edges.add(e[:2])
            if len(e) > 2:
                DiGraphSet._weights[e[:2]] = e[2]
        if traversal != 'as-is':
            if source is None:
                source = sorted_edges[0][0]
                for e in sorted_edges:
                    source = min(e[0], e[1], source)
            sorted_edges = DiGraphSet._traverse(
                indexed_edges, traversal, source)
        for u, v in sorted_edges:
            DiGraphSet._vertices.add(u)
            DiGraphSet._vertices.add(v)
        #setset_base.set_universe(sorted_edges)
        DiGraphSet._objtable = ObjectTable()
        for e in sorted_edges:
            DiGraphSet._objtable.add_elem(e)

    @staticmethod
    def universe():
        """Returns the current universe.

        The list of edges that represents the current universe is
        returned.

        Examples:
          >>> DiGraphSet.universe()
          [(1, 2, 2.0), (1, 4, -3.0), (2, 3)]

        Returns:
          The universe if no argument is given, or None otherwise.

        See Also:
          set_universe()
        """
        edges = []
        for e in DiGraphSet._objtable.universe():
            if e in DiGraphSet._weights:
                edges.append((e[0], e[1], DiGraphSet._weights[e]))
            else:
                edges.append(e)
        return DiGraphSet.converters['to_graph'](edges)

    @staticmethod
    def directed_cycles(graphset=None):
        """Returns a DiGraphSet with directed single cycles.

        Examples:
          >>> DiGraphSet.directed_cycles()
          DiGraphSet([[(1, 4), (4, 1)], [(4, 5), (5, 4)], [(1, 2), (2, 1)], [(2, 5), ( ...

          Args:
            graphset: Optional.  A DiGraphSet object.  Components to be
              stored are selected from this object.

          Returns:
            A new DiGraphSet object.
        """
        graph = []
        for e in DiGraphSet._objtable.universe():
            assert e[0] in DiGraphSet._vertices and e[1] in DiGraphSet._vertices
            graph.append(
                (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        ss = None if graphset is None else graphset._ss

        ss = _graphillion._directed_cycles(graph=graph, search_space=ss)
        return DiGraphSet(ss)

    @staticmethod
    def directed_hamiltonian_cycles(graphset=None):
        """Returns a DiGraphSet with directed single hamiltonian cycles.

        Examples:
          >>> DiGraphSet.directed_hamiltonian_cycles()
          DiGraphSet([[(1, 4), (2, 1), (3, 2), (4, 5), (5, 6), (6, 3)], [(1, 2), (2, 3 ...

          Args:
            graphset: Optional.  A DiGraphSet object.  Components to be
              stored are selected from this object.

          Returns:
            A new DiGraphSet object.
        """
        graph = []
        for e in DiGraphSet._objtable.universe():
            assert e[0] in DiGraphSet._vertices and e[1] in DiGraphSet._vertices
            graph.append(
                (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        ss = None if graphset is None else graphset._ss

        ss = _graphillion._directed_hamiltonian_cycles(
            graph=graph, search_space=ss)
        return DiGraphSet(ss)

    @staticmethod
    def directed_st_paths(s, t, is_hamiltonian=False, graphset=None):
        """Returns a DiGraphSet with directed directed st paths.

        Examples:
          >>> DiGraphSet.directed_st_paths(1, 6, False)
          DiGraphSet([[(1, 4), (4, 5), (5, 6)], [(1, 2), (2, 5), (5, 6)], [(1, 2), (2, ...

          >>> DiGraphSet.directed_st_paths(1, 6, True)
          DiGraphSet([[(1, 4), (2, 3), (3, 6), (4, 5), (5, 2)]])

          Args:
            s: A vertex. The start point of paths.

            t: A vertex. The endpoint of paths.

            is_hamiltonian: Optional. True or False. If true, paths
              must be hamiltonian.

            graphset: Optional.  A DiGraphSet object.  Components to be
              stored are selected from this object.

          Returns:
            A new DiGraphSet object.
        """
        graph = []
        for e in DiGraphSet._objtable.universe():
            assert e[0] in DiGraphSet._vertices and e[1] in DiGraphSet._vertices
            graph.append(
                (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        ss = None if graphset is None else graphset._ss

        assert(s in DiGraphSet._vertices and t in DiGraphSet._vertices)

        ss = _graphillion._directed_st_path(
            graph=graph, s=pickle.dumps(s, protocol=0), t=pickle.dumps(t, protocol=0),
            is_hamiltonian=is_hamiltonian, search_space=ss)
        return DiGraphSet(ss)

    @staticmethod
    def rooted_forests(roots=None, is_spanning=False, graphset=None):
        """Returns a DiGraphSet with directed forests.
        'directed forest' is also called 'brancing'.

        Examples:
          >>> DiGraphSet.directed_forests()
          DiGraphSet([[], [(4, 1)], [(1, 4)], [(5, 4)], [(4, 5)], [(2, 1)], [(1, 2)],  ...

          Args:
            roots: Optional. A list of vertices.

            is_spanning: Optional. True or False. If true, trees must
              be composed of all vertices.

            graphset: Optional.  A DiGraphSet object.  Components to be
              stored are selected from this object.

          Returns:
            A new DiGraphSet object.
        """
        graph = []
        for e in DiGraphSet._objtable.universe():
            assert e[0] in DiGraphSet._vertices and e[1] in DiGraphSet._vertices
            graph.append(
                (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        rs = []
        if roots is not None:
            for root in roots:
                assert root in DiGraphSet._vertices
                rs.append(pickle.dumps(root, protocol=0))

        ss = None if graphset is None else graphset._ss

        ss = _graphillion._rooted_forests(
            graph=graph, roots=rs, is_spanning=is_spanning, search_space=ss)
        return DiGraphSet(ss)

    @staticmethod
    def rooted_trees(root, is_spanning=False, graphset=None):
        """Returns a DiGraphSet with directed rooted trees.

        Examples:
          >>> DiGraphSet.rooted_trees(1, False)
          DiGraphSet([[(1, 4)], [(1, 2)], [(1, 4), (4, 5)], [(1, 2), (1, 4)], [(1, 2), ...

          >>> DiGraphSet.rooted_trees(1, True)
          DiGraphSet([[(1, 2), (1, 4), (2, 3), (3, 6), (4, 5)], [(1, 2), (1, 4), (2, 3 ...

          Args:
            root: A vertex, at which trees are rooted.

            is_spanning: Optional. True or False. If true, trees must
              be composed of all vertices.

            graphset: Optional.  A DiGraphSet object.  Components to be
              stored are selected from this object.

          Returns:
            A new DiGraphSet object.
        """
        graph = []
        for e in DiGraphSet._objtable.universe():
            assert e[0] in DiGraphSet._vertices and e[1] in DiGraphSet._vertices
            graph.append(
                (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        ss = None if graphset is None else graphset._ss

        ss = _graphillion._rooted_trees(
            graph=graph, root=pickle.dumps(root, protocol=0), is_spanning=is_spanning, search_space=ss)
        return DiGraphSet(ss)

    @staticmethod
    def graphs(in_degree_constraints=None, out_degree_constraints=None, graphset=None):
        """Returns a DiGraphSet with digraphs under given constraints.

        Examples:
          >>> DiGraphSet.graphs()
          DiGraphSet([[], [(4, 1)], [(1, 4)], [(5, 4)], [(4, 5)], [(2, 1)], [(1, 2)],  ...

          Args:
            in_degree_constraints: Optional. A dict with a vertex and a
            range or int.  The degree of a vertex is restricted by the
            range.  For `{1: 2, 6: range(2)}`, the degree of vertex 1
            is 2 and that of 6 is less than 2, while others are not
            cared.

            out_degree_constraints: Optional. A dict with a vertex and a
            range or int.  The degree of a vertex is restricted by the
            range.  For `{1: 2, 6: range(2)}`, the degree of vertex 1
            is 2 and that of 6 is less than 2, while others are not
            cared.

            graphset: Optional.  A DiGraphSet object.  Components to be
              stored are selected from this object.

          Returns:
            A new DiGraphSet object.
        """
        graph = []
        for e in DiGraphSet._objtable.universe():
            assert e[0] in DiGraphSet._vertices and e[1] in DiGraphSet._vertices
            graph.append(
                (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        in_dc = None
        if in_degree_constraints is not None:
            in_dc = {}
            for v, r in in_degree_constraints.items():
                if v not in DiGraphSet._vertices:
                    raise KeyError(v)
                if isinstance(r, int):
                    in_dc[pickle.dumps(v, protocol=0)] = (r, r + 1, 1)
                elif len(r) == 1:
                    in_dc[pickle.dumps(v, protocol=0)] = (r[0], r[0] + 1, 1)
                else:
                    in_dc[pickle.dumps(v, protocol=0)] = (
                        r[0], r[-1] + 1, r[1] - r[0])

        out_dc = None
        if out_degree_constraints is not None:
            out_dc = {}
            for v, r in out_degree_constraints.items():
                if v not in DiGraphSet._vertices:
                    raise KeyError(v)
                if isinstance(r, int):
                    out_dc[pickle.dumps(v, protocol=0)] = (r, r + 1, 1)
                elif len(r) == 1:
                    out_dc[pickle.dumps(v, protocol=0)] = (r[0], r[0] + 1, 1)
                else:
                    out_dc[pickle.dumps(v, protocol=0)] = (
                        r[0], r[-1] + 1, r[1] - r[0])

        ss = None if graphset is None else graphset._ss

        ss = _graphillion._directed_graphs(
            graph=graph, in_degree_constraints=in_dc, out_degree_constraints=out_dc, search_space=ss)
        return DiGraphSet(ss)

    @staticmethod
    def show_messages(flag=True):
        """Enables/disables status messages.

        Args:
          flag: Optional.  True or False.  If True, status messages are
          enabled.  If False, they are disabled (initial setting).

        Returns:
          The setting before the method call.  True (enabled) or
          False (disabled).
        """
        return _graphillion._show_messages(flag)

    @staticmethod
    def _traverse(indexed_edges, traversal, source):
        neighbors = {}
        for u, v in indexed_edges:
            if u not in neighbors:
                neighbors[u] = set([v])
            else:
                neighbors[u].add(v)
            if v not in neighbors:
                neighbors[v] = set([u])
            else:
                neighbors[v].add(u)
        assert source in neighbors
        vertices = set(neighbors.keys())

        sorted_edges = []
        visited_vertices = set()
        u = source

        if traversal == 'greedy':
            degree = dict()
            for v in vertices:
                degree[v] = len(neighbors[v])

            heap = []
            while True:
                visited_vertices.add(u)
                for v in sorted(neighbors[u]):
                    degree[v] -= 1
                    if v in visited_vertices:
                        degree[u] -= 1
                        e = (u, v) if (u, v) in indexed_edges else (v, u)
                        sorted_edges.append(e)
                        if (e[1], e[0]) in indexed_edges:
                            sorted_edges.append((e[1], e[0]))
                        if degree[v]:
                            for w in sorted(neighbors[v]):
                                if w not in visited_vertices:
                                    heapq.heappush(
                                        heap, (degree[v], degree[w], w))
                for v in sorted(neighbors[u]):
                    if v not in visited_vertices:
                        heapq.heappush(heap, (degree[u], degree[v], v))
                if visited_vertices == vertices:
                    break
                while u in visited_vertices:
                    if not heap:
                        u = min(vertices - visited_vertices)
                    else:
                        u = heapq.heappop(heap)[2]
            assert set(indexed_edges) == set(sorted_edges)
            return sorted_edges
        elif traversal == 'bfs' or traversal == 'dfs':
            queue_or_stack = []
            while True:
                visited_vertices.add(u)
                for v in sorted(neighbors[u]):
                    if v in visited_vertices:
                        e = (u, v) if (u, v) in indexed_edges else (v, u)
                        sorted_edges.append(e)
                new_vertices = neighbors[u] - \
                    visited_vertices - set(queue_or_stack)
                queue_or_stack.extend(sorted(new_vertices))
                if not queue_or_stack:
                    if visited_vertices == vertices:
                        break
                    else:
                        queue_or_stack.append(min(vertices - visited_vertices))
                if traversal == 'bfs':
                    u, queue_or_stack = queue_or_stack[0], queue_or_stack[1:]
                else:
                    queue_or_stack, u = queue_or_stack[:-1], queue_or_stack[-1]
                assert u not in visited_vertices
            assert set(indexed_edges) == set(sorted_edges)
            return sorted_edges
        else:
            raise ValueError('invalid `traversal`: {}'.format(traversal))

    @staticmethod
    def _conv_arg(obj):
        if isinstance(obj, DiGraphSet):
            return 'digraphset', obj
        elif isinstance(obj, (set, frozenset, list)):
            return 'graph', set([DiGraphSet._conv_edge(e) for e in obj])
        elif isinstance(obj, tuple):
            return 'edge', DiGraphSet._conv_edge(obj)
        elif obj in DiGraphSet._vertices:
            return 'vertex', [e for e in DiGraphSet._objtable.universe() if obj in e]
        try:
            edges = DiGraphSet.converters['to_edges'](obj)
            return 'graph', set([DiGraphSet._conv_edge(e) for e in edges])
        except TypeError:  # if fail to convert obj into edge list
            raise KeyError(obj)

    @staticmethod
    def _conv_graph(obj):
        return DiGraphSet.converters['to_edges'](obj)

    @staticmethod
    def _conv_edge(edge):
        if not isinstance(edge, tuple) or len(edge) < 2:
            raise KeyError(edge)
        if len(edge) > 2:
            edge = edge[:2]
        if edge in DiGraphSet._objtable.obj2int:
            return edge
        elif (edge[1], edge[0]) in DiGraphSet._objtable.obj2int:
            return (edge[1], edge[0])
        raise KeyError(edge)

    @staticmethod
    def _conv_ret(obj):
        if isinstance(obj, (set, frozenset)):  # a graph
            return DiGraphSet.converters['to_graph'](sorted(list(obj)))
        raise TypeError(obj)

    converters = {'to_graph': lambda edges: edges,
                  'to_edges': lambda graph: graph}

    _vertices = set()
    _weights = {}

    _objtable = ObjectTable()
