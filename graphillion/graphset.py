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

"""Module for a set of graphs.
"""

from functools import partial
from builtins import range, int
from future.utils import viewitems
import _graphillion
from graphillion import setset, VertexSetSet
import pickle
import heapq


class GraphSet(object):
    """Represents and manipulates a set of graphs.

    A GraphSet object stores a set of graphs.  A graph stored must be
    a subgraph of the universal graph, and is represented by a list of
    edges in the universal graph.  An edge is a tuple of two vertices,
    and a vertex can be any hashable object like a number, a text
    string, and a tuple.  Currently, GraphSet only supports undirected
    graphs without edge labels.

    The universal graph must be defined before creating GraphSet
    objects by `GraphSet.universe()` method.

    Like Python set types, GraphSet supports `graph in graphset`,
    `len(graphset)`, and `for graph in graphset`.  It also supports
    all set methods and operators,
    * isdisjoint(), issubset(), issuperset(), union(), intersection(),
      difference(), symmetric_difference(), copy(), update(),
      intersection_update(), difference_update(),
      symmetric_difference_update(), add(), remove(), discard(),
      pop(), clear(),
    * ==, !=, <=, <, >=, >, |, &, -, ^, |=, &=, -=, ^=.

    Examples:
      >>> from graphillion import GraphSet

      We assume the following graph and register the edge list as the
      universe.

      1 --- 2 --- 3
      |     |     |
      4 --- 5 --- 6

      >>> universe = [(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5), (5, 6)]
      >>> GraphSet.set_universe(universe)

      Find all paths from 1 to 6 and count them.

      >>> paths = GraphSet.paths(1, 6)
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

    def __init__(self, graphset_or_constraints=None):
        """Initializes a GraphSet object with a set of graphs or constraints.

        Examples:
          >>> graph1 = [(1, 4)]
          >>> graph2 = [(1, 2), (2, 3)]
          >>> GraphSet([graph1, graph2])
          GraphSet([[(1, 4)], [(1, 2), (2, 3)]])
          >>> GraphSet({'include': graph1, 'exclude': graph2})
          GraphSet([[(1, 4)], [(1, 4), (2, 5)], [(1, 4), (3, 6)], ...

        Args:
          graphset_or_constraints: A set of graphs represented by a
            list of graphs (a list of edge lists):

            [[(1, 4)], [(1, 2), (2, 3)]]

            Or constraints represented by a dict of included or
            excluded edge lists (not-specified edges are not cared):

            {'include': [(1, 4)], 'exclude': [(1, 2), (2, 3)]}

            If no argument is given, it is treated as an empty list
            `[]` and an empty GraphSet is returned.  An empty dict
            `{}` means that no constraint is specified, and so a
            GraphSet including all possible graphs in the universe is
            returned (let N the number of edges in the universe, 2^N
            graphs are stored in the new object).

        Raises:
          KeyError: If given edges are not found in the universe.

        See Also:
          copy()
        """
        obj = graphset_or_constraints
        if isinstance(obj, GraphSet):
            self._ss = obj._ss.copy()
        elif isinstance(obj, setset):
            self._ss = obj.copy()
        else:
            if obj is None:
                obj = []
            elif isinstance(obj, (set, frozenset, list)):  # a list of graphs [graph+]
                l = []
                for g in obj:
                    edges = GraphSet.converters['to_edges'](g)
                    l.append(set([GraphSet._conv_edge(e) for e in edges]))
                obj = l
            elif isinstance(obj, dict):  # constraints
                d = {}
                for k, l in viewitems(obj):
                    d[k] = [GraphSet._conv_edge(e) for e in l]
                obj = d
            self._ss = setset(obj)
        methods = ['graphs', 'connected_components', 'cliques', 'bicliques',
                   'trees', 'forests', 'cycles', 'paths', 'matchings',
                   'perfect_matchings', 'k_matchings', 'b_matchings',
                   'k_factors', 'f_factors', 'regular_graphs',
                   'bipartite_graphs', 'regular_bipartite_graphs',
                   'steiner_subgraphs', 'steiner_trees', 'steiner_cycles',
                   'steiner_paths', 'degree_distribution_graphs',
                   'letter_P_graphs']
        for method in methods:
            setattr(self, method, partial(getattr(GraphSet, method), graphset=self))

    def copy(self):
        """Returns a new GraphSet with a shallow copy of `self`.

        Examples:
          >>> gs2 = gs1.copy()
          >>> gs1 -= gs2
          >>> gs1 == gs2
          False

        Returns:
          A new GraphSet object.

        See Also:
          __init__()
        """
        return GraphSet(self)

    def __nonzero__(self):
        return bool(self._ss)

    def __repr__(self):
        return setset._repr(self._ss, (self.__class__.__name__ + '([', '])'))

    def union(self, *others):
        """Returns a new GraphSet with graphs from `self` and all others.

        The `self` is not changed.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph2, graph3])
          >>> gs1 | gs2
          GraphSet([[], [(1, 2)], [(1, 2), (1, 4)]])

        Returns:
          A new GraphSet object.

        See Also:
          intersection(), difference(), symmetric_difference(),
          update()
        """
        return GraphSet(self._ss.union(*[gs._ss for gs in others]))

    def intersection(self, *others):
        """Returns a new GraphSet with graphs common to `self` and all others.

        The `self` is not changed.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph2, graph3])
          >>> gs1 & gs2
          GraphSet([[(1, 2)]])

        Returns:
          A new GraphSet object.

        See Also:
          union(), difference(), symmetric_difference(),
          intersection_update()
        """
        return GraphSet(self._ss.intersection(*[gs._ss for gs in others]))

    def difference(self, *others):
        """Returns a new GraphSet with graphs in `self` that are not in the others.

        The `self` is not changed.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph2, graph3])
          >>> gs1 - gs2
          GraphSet([])

        Returns:
          A new GraphSet object.

        See Also:
          union(), intersection(), symmetric_difference(),
          difference_update()
        """
        return GraphSet(self._ss.difference(*[gs._ss for gs in others]))

    def symmetric_difference(self, *others):
        """Returns a new GraphSet with graphs in either `self` or `other` but not both.

        The `self` is not changed.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph2, graph3])
          >>> gs1 ^ gs2
          GraphSet([[], [(1, 2), (1, 4)]])

        Returns:
          A new GraphSet object.

        See Also:
          union(), intersection(), difference(), 
          symmetric_difference_update()
        """
        return GraphSet(self._ss.symmetric_difference(*[gs._ss for gs in others]))

    def quotient(self, other):
        """Returns a new GraphSet of quotient.

        The quotient is defined by,
          gs1 / gs2 = {a | a \\cup b \\in gs1 and a \\cap b = \\empty, \\forall b \\in gs2}.
        D. Knuth, Exercise 204, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3), (2, 5)]
          >>> graph3 = [(1, 4)]
          >>> gs = GraphSet([graph1, graph2])
          >>> gs / GraphSet([graph3])
          GraphSet([[(1, 2)]])

        Returns:
          A new GraphSet object.

        See Also:
          remainder(), quotient_update()
        """
        return GraphSet(self._ss.quotient(other._ss))

    def remainder(self, other):
        """Returns a new GraphSet of remainder.

        The remainder is defined by,
          gs1 % gs2 = gs1 - (gs1 \\sqcup (gs1 / gs2)).
        D. Knuth, Exercise 204, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3), (2, 5)]
          >>> graph3 = [(1, 4)]
          >>> gs = GraphSet([graph1, graph2])
          >>> gs % GraphSet([graph3])
          GraphSet([[(2, 3), (2, 5)]])

        Returns:
          A new GraphSet object.

        See Also:
          quotient(), remainder_update()
        """
        return GraphSet(self._ss.remainder(other._ss))

    def update(self, *others):
        """Updates `self`, adding graphs from all others.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph2, graph3])
          >>> gs1 |= gs2
          >>> gs1
          GraphSet([[], [(1, 2)], [(1, 2), (1, 4)]])

        Returns:
          A new GraphSet object.

        See Also:
          union()
        """
        self._ss.update(*[gs._ss for gs in others])
        return self

    def intersection_update(self, *others):
        """Updates `self`, keeping only graphs found in it and all others.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph2, graph3])
          >>> gs1 &= gs2
          >>> gs1
          GraphSet([[(1, 2)]])

        Returns:
          A new GraphSet object.

        See Also:
          intersection()
        """
        self._ss.intersection_update(*[gs._ss for gs in others])
        return self

    def difference_update(self, *others):
        """Update `self`, removing graphs found in others.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph2, graph3])
          >>> gs1 -= gs2
          >>> gs1
          GraphSet([[]])

        Returns:
          A new GraphSet object.

        See Also:
          difference()
        """
        self._ss.difference_update(*[gs._ss for gs in others])
        return self

    def symmetric_difference_update(self, *others):
        """Update `self`, keeping only graphs in either GraphSet, but not in both.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2)]
          >>> graph3 = [(1, 2), (1, 4)]
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph2, graph3])
          >>> gs1 ^= gs2
          >>> gs1
          GraphSet([[], [(1, 2), (1, 4)]])

        Returns:
          A new GraphSet object.

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
          >>> gs = GraphSet([graph1, graph2])
          >>> gs /= GraphSet([graph3])
          >>> gs
          GraphSet([[(1, 2)]])

        Returns:
          A new GraphSet object.

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
          >>> gs = GraphSet([graph1, graph2])
          >>> gs %= GraphSet([graph3])
          >>> gs
          GraphSet([[(2, 3), (2, 5)]])

        Returns:
          A new GraphSet object.

        See Also:
          remainder()
        """
        self._ss.remainder_update(other._ss)
        return self

    def __invert__(self):
        """Returns a new GraphSet with graphs not stored in `self`.

        Examples:
          >>> GraphSet.set_universe([(1, 2), (1, 4)])
          >>> graph = [(1, 2)]
          >>> gs = GraphSet([graph])
          >>> ~gs
          GraphSet([[], [(1, 4)], [(1, 2), (1, 4)]])

        Returns:
          A new GraphSet object.
        """
        return GraphSet(~self._ss)

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
        """Returns True if `self` has no graphs in common with `other`.

        Examples:
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph3, graph4, graph5])
          >>> gs1.disjoint(gs2)
          True

        Returns:
          True or False.

        See Also:
          issubset(), issuperset()
        """
        return self._ss.isdisjoint(other._ss)

    def issubset(self, other):
        """Tests if every graph in `self` is in `other`.

        Examples:
          >>> gs1 = GraphSet([graph1, graph3])
          >>> gs2 = GraphSet([graph1, graph2, graph3])
          >>> gs1 <= gs2
          True

        Returns:
          True or False.

        See Also:
          issuperset(), isdisjoint()
        """
        return self._ss.issubset(other._ss)

    def issuperset(self, other):
        """Tests if every graph in `other` is in `self`.

        Examples:
          >>> gs1 = GraphSet([graph1, graph2, graph3])
          >>> gs2 = GraphSet([graph1, graph3])
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
        """Returns the number of graphs in `self`.

        Use gs.len() if OverflowError raised.

        Examples:
          >>> gs = GraphSet([graph1, graph2])
          >>> len(gs)
          2

        Returns:
          The number of graphs.

        Raises:
          OverflowError

        See Also:
          len()
        """
        return len(self._ss)

    def len(self, size=None):
        """Returns the number of graphs, or a new GraphSet with `size` edges.

        If no argument is given, this method returns the number of
        graphs in `self`.  Otherwise, this method returns a new
        GraphSet with graphs that have `size` edges; this usage of
        `len(size)` is obsoleted, and use `graph_size(size)` instead.

        This method never raises OverflowError unlike built-in len(gs).

        Examples:
          >>> gs = GraphSet([graph1, graph2])
          >>> gs.len()
          2L

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(1, 2), (1, 4), (2, 3)]
          >>> gs = GraphSet([graph1, graph2, graph3])
          >>> gs.len(2)
          GraphSet([[(1, 2), (1, 4)]])

        Args:
          size: Optional.  The number of edges in a graph.

        Returns:
          The number of graphs, or a new GraphSet object.

        See Also:
          __len__(), smaller(), larger(), graph_size()

        """
        if size is None:
            return self._ss.len()
        else:
            return self.graph_size(size)

    def __iter__(self):
        """Iterates over graphs.

        This is the fastest iterator among Graphset iterators, such as
        rand_iter() and max_iter().

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> gs = GraphSet([graph1, graph2])
          >>> for g in gs:
          ...   g
          [(1, 2), (1, 4)]
          [(1, 2)]

        Returns:
          A generator.

        Yields:
          A graph.

        See Also:
          rand_iter(), max_iter(), min_iter()
        """
        for g in self._ss.__iter__():
            try:
                yield GraphSet._conv_ret(g)
            except StopIteration:
                return

    def rand_iter(self):
        """Iterates over graphs uniformly randomly.

        This method relies on its own random number generator, doesn't
        rely on Python random module.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> gs = GraphSet([graph1, graph2])
          >>> for g in gs.rand_iter():
          ...   g
          [(1, 2)]
          [(1, 2), (1, 4)]

        Returns:
          A generator.

        Yields:
          A graph.

        See Also:
          __iter__(), max_iter(), min_iter()
        """
        for g in self._ss.rand_iter():
            try:
                yield GraphSet._conv_ret(g)
            except StopIteration:
                return

    def min_iter(self, weights=None):
        """Iterates over graphs in the ascending order of weights.

        Returns a generator that iterates over graphs in `self`
        GraphSet.  The graphs are selected in the ascending order of
        edge weights, which are specified by the argument `weights` or
        those set as the universe (1.0 for unspecified edges).  The
        `weights` does not overwrite the weights of the universe.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
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
          A graph.

        See Also:
          __iter__(), rand_iter(), max_iter()

        """
        if weights is None:
            weights = GraphSet._weights
        for g in self._ss.min_iter(weights):
            try:
                yield GraphSet._conv_ret(g)
            except StopIteration:
                return

    def max_iter(self, weights=None):
        """Iterates over graphs in the descending order of weights.

        Returns a generator that iterates over graphs in `self`
        GraphSet.  The graphs are selected in the descending order of
        edge weights, which are specified by the argument `weights` or
        those set as the universe (1.0 for unspecified edges).  The
        `weights` does not overwrite the weights of the universe.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
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
          A graph.

        See Also:
          __iter__(), rand_iter(), min_iter()
        """
        if weights is None:
            weights = GraphSet._weights
        for g in self._ss.max_iter(weights):
            try:
                yield GraphSet._conv_ret(g)
            except StopIteration:
                return

    def __contains__(self, obj):
        """Returns True if `obj` is in the `self`, False otherwise.

        Use the expression `obj in gs`.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> graph1 in gs
          True

        Args:
          obj: A graph (an edge list), an edge, or a vertex in the
            universe.

        Returns:
          True or False.

        Raises:
          KeyError: If the given object is not found in the universe.
        """
        type, obj = GraphSet._conv_arg(obj)
        if type == 'graph' or type == 'edge':
            return obj in self._ss
        elif type == 'vertex':
            return len([e for e in obj if e in self._ss]) > 0
        raise TypeError(obj)

    def add(self, graph_or_edge):
        """Adds a given graph or edge to `self`.

        If a graph is given, the graph is just added to `self`
        GraphSet.  If an edge is given, the edge is grafted to all the
        graphs in `self`.  The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> edge = (1, 2)
          >>> gs.add(edge)
          >>> gs
          GraphSet([[(1, 2), (1, 4)], [(1, 2), (2, 3)]])

        Args:
          graph_or_edge: A graph (an edge list) or an edge in the
            universe.

        Returns:
          None.

        Raises:
          KeyError: If a given edge or vertex is not found in the
            universe.

        See Also:
          remove(), discard()
        """
        type, obj = GraphSet._conv_arg(graph_or_edge)
        if type == 'graph' or type == 'edge':
            self._ss.add(obj)
        else:
            raise TypeError(graph_or_edge)

    def remove(self, obj):
        """Removes a given graph, edge, or vertex from `self`.

        If a graph is given, the graph is just removed from `self`
        GraphSet.  If an edge is given, the edge is removed from all
        the graphs in `self`.  The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> edge = (1, 2)
          >>> gs.remove(edge)
          >>> gs
          GraphSet([[(1, 4)], [(2, 3)]])

        Args:
          obj: A graph (an edge list), an edge, or a vertex in the
            universe.

        Returns:
          None.

        Raises:
          KeyError: If a given edge or vertex is not found in the
            universe, or if the given graph is not stored in `self`.

        See Also:
          add(), discard(), pop()
        """
        type, obj = GraphSet._conv_arg(obj)
        if type == 'graph' or type == 'edge':
            self._ss.remove(obj)
        elif type == 'vertex':
            for edge in obj:
                self.remove(edge)
        else:
            raise TypeError(obj)
        return None

    def discard(self, obj):
        """Removes a given graph, edge, or vertex from `self`.

        If a graph is given, the graph is just removed from `self`
        GraphSet.  If an edge is given, the edge is removed from all
        the graphs in `self`.  The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> edge = (1, 2)
          >>> gs.discard(edge)
          >>> gs
          GraphSet([[(1, 4)], [(2, 3)]])

        Args:
          obj: A graph (an edge list), an edge, or a vertex in the
            universe.

        Returns:
          None.

        Raises:
          KeyError: If a given edge or vertex is not found in the
            universe.

        See Also:
          add(), remove(), pop()
        """
        type, obj = GraphSet._conv_arg(obj)
        if type == 'graph' or type == 'edge':
            self._ss.discard(obj)
        elif type == 'vertex':
            for edge in obj:
                self.discard(edge)
        else:
            raise TypeError(obj)
        return None

    def pop(self):
        """Removes and returns an arbitrary graph from `self`.

        The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> gs.pop()
          [(1, 2), (1, 4)]
          >>> gs
          GraphSet([[(2, 3)]])

        Returns:
          A graph.

        Raises:
          KeyError: If `self` is empty.

        See Also:
          remove(), discard(), choice()
        """
        return GraphSet._conv_ret(self._ss.pop())

    def clear(self):
        """Removes all graphs from `self`.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> gs.clear()
          >>> gs
          GraphSet([])
        """
        return self._ss.clear()

    def flip(self, edge):
        """Flips the state of a given edge over all graphs in `self`.

        If a graph in `self` includes the given edge, the edge is
        removed from the graph.  If a graph in `self` does not include
        the given edge, the edge is added to the graph.

        The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> edge = (1, 2)
          >>> gs.flip(edge)
          >>> gs
          GraphSet([[(1, 4)], [(1, 2), (2, 3)]])

        Args:
          edge: An edge in the universe.

        Returns:
          A new GraphSet object.

        Raises:
          KeyError: If a given edge is not found in the universe.
        """
        type, obj = GraphSet._conv_arg(edge)
        if type == 'edge':
            self._ss.flip(edge)
        else:
            raise TypeError(edge)

    def minimal(self):
        """Returns a new GraphSet of minimal graphs.

        The minimal sets are defined by,
          gs.minimal() = {a \\in gs | b \\in gs and a \\subseteq -> a = b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(1, 4), (2, 3)]
          >>> gs = GraphSet([graph1, graph2, graph3])
          >>> gs.minimal()
          GraphSet([[(1, 2)], [(1, 4), (2, 3)]])

        Returns:
          A new GraphSet object.

        See Also:
          maximal(), blocking()
        """
        return GraphSet(self._ss.minimal())

    def maximal(self):
        """Returns a new GraphSet of maximal graphs.

        The maximal sets are defined by,
          gs.maximal() = {a \\in gs | b \\in gs and a \\superseteq -> a = b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(1, 4), (2, 3)]
          >>> gs = GraphSet([graph1, graph2, graph3])
          >>> gs.maximal()
          GraphSet([[(1, 2), (1, 4)], [(1, 4), (2, 3)]])

        Returns:
          A new GraphSet object.

        See Also:
          minimal()
        """
        return GraphSet(self._ss.maximal())

    def blocking(self):
        """Returns a new GraphSet of all blocking (hitting) sets.

        A blocking set is often called a hitting set; all graphs in
        `self` contain at least one edge in the set.  This implies
        that all the graphs are destroyed by removing edges in the
        set.

        The blocking sets are defined by,
          gs.blocking() = {a | b \\in gs -> a \\cap b \\neq \\empty}.
        T. Toda, Hypergraph Dualization Algorithm Based on Binary
        Decision Diagrams.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(1, 4), (2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> gs.blocking().minimal()
          GraphSet([[(1, 4)], [(1, 2), (2, 3)]])

        Returns:
          A new GraphSet object.

        See Also:
          minimal()
        """
        return GraphSet(self._ss.hitting())

    hitting = blocking

    def smaller(self, size):
        """Returns a new GraphSet with graphs that have less than `size` edges.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(1, 2), (1, 4), (2, 3)]
          >>> gs = GraphSet([graph1, graph2, graph3])
          >>> gs.smaller(2)
          GraphSet([[(1, 2)]])

        Args:
          size: The number of edges in a graph.

        Returns:
          A new GraphSet object.

        See Also:
          larger(), graph_size()
        """
        return GraphSet(self._ss.smaller(size))

    def larger(self, size):
        """Returns a new GraphSet with graphs that have more than `size` edges.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(1, 2), (1, 4), (2, 3)]
          >>> gs = GraphSet([graph1, graph2, graph3])
          >>> gs.larger(2)
          GraphSet([[(1, 2), (1, 4), (2, 3)]])

        Args:
          size: The number of edges in a graph.

        Returns:
          A new GraphSet object.

        See Also:
          smaller(), graph_size()
        """
        return GraphSet(self._ss.larger(size))

    def graph_size(self, size):
        """Returns a new GraphSet with `size` edges.

        This method returns a new GraphSet with graphs that have
        `size` edges.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(1, 2), (1, 4), (2, 3)]
          >>> gs = GraphSet([graph1, graph2, graph3])
          >>> gs.graph_size(2)
          GraphSet([[(1, 2), (1, 4)]])

        Args:
          size: The number of edges in a graph.

        Returns:
          A new GraphSet object.

        See Also:
          smaller(), larger()

        """
        return GraphSet(self._ss.set_size(size))

    def complement(self):
        """Returns a new GraphSet with complement graphs of `self`.

        The `self` is not changed.

        Examples:
          >>> GraphSet.set_universe([(1, 2), (1, 4)])
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> gs = GraphSet([graph1, graph2])
          >>> gs.complement()
          GraphSet([[], [(1, 4)]])

        Returns:
          A new GraphSet object.
        """
        ss = self._ss.copy()
        ss.flip()
        return GraphSet(ss)

    def join(self, other):
        """Returns a new GraphSet of join between `self` and `other`.

        The join operation is defined by,
          gs1 \\sqcup gs2 = {a \\cup b | a \\in gs1 and b \\in gs2}.
        D. Knuth, Exercise 203, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = [(2, 3)]
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph3])
          >>> gs1.join(gs2)
          GraphSet([[(1, 2), (2, 3)], [(1, 2), (1, 4), (2, 3)]])

        Returns:
          A new GraphSet object.

        See Also:
          meet()
        """
        return GraphSet(self._ss.join(other._ss))

    def meet(self, other):
        """Returns a new GraphSet of meet between `self` and `other`.

        The meet operation is defined by,
          gs1 \\sqcap gs2 = {a \\cap b | a \\in gs1 and b \\in gs2}.
        D. Knuth, Exercise 203, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(1, 2), (2, 3)]
          >>> graph3 = [(1, 4), (2, 3)]
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph3])
          >>> gs1.meet(gs2)
          GraphSet([[(1, 4)], [(2, 3)]])

        Returns:
          A new GraphSet object.

        See Also:
          join()
        """
        return GraphSet(self._ss.meet(other._ss))

    def subgraphs(self, other):
        """Returns a new GraphSet with subgraphs of a graph in `other`.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = graph1 + [(2, 3)]
          >>> graph4 = [(1, 4), (2, 3)]
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph3, graph4])
          >>> gs1.subgraphs(gs2)
          GraphSet([[(1, 2)]])

        Returns:
          A new GraphSet object.

        See Also:
          supersets(), non_subsets()
        """
        return GraphSet(self._ss.subsets(other._ss))

    def supergraphs(self, other):
        """Returns a new GraphSet with supergraphs of a graph in `other`.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (2, 3)]
          >>> graph2 = [(1, 4), (2, 3)]
          >>> graph3 = [(1, 2)]          # graph1 - (2, 3)
          >>> graph4 = [(1, 2), (1, 4)]
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph3, graph4])
          >>> gs1.supergraphs(gs2)
          GraphSet([[(1, 2), (2, 3)]])

        Returns:
          A new GraphSet object.

        See Also:
          subsets(), non_supersets()
        """
        return GraphSet(self._ss.supersets(other._ss))

    def non_subgraphs(self, other):
        """Returns a new GraphSet with graphs that aren't subgraphs of any graph in `other`.

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
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph3, graph4])
          >>> gs1.non_subgraphs(gs2)
          GraphSet([[(1, 2), (1, 4)]])

        Returns:
          A new GraphSet object.

        See Also:
          non_supersets(), subsets()
        """
        return GraphSet(self._ss.non_subsets(other._ss))

    def non_supergraphs(self, other):
        """Returns a new GraphSet with graphs that aren't supergraphs of any graph in `other`.

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
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph3, graph4])
          >>> gs1.non_supergraphs(gs2)
          GraphSet([[(1, 4), (2, 3)]])

        Returns:
          A new GraphSet object.

        See Also:
          non_subsets(), supersets()
        """
        return GraphSet(self._ss.non_supersets(other._ss))

    def including(self, obj):
        """Returns a new GraphSet that includes supergraphs of `obj`.

        Returns a new set of graphs that include `obj`, which can be a
        GraphSet, a graph, an edge, or a vertex.  If `obj` is a
        GraphSet, a graph returned includes *one of* graphs in the
        given GraphSet.

        The graphs stored in the new GraphSet are selected from `self`
        GraphSet.  The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> vertex = 4
          >>> gs.including(vertex)
          GraphSet([[(1, 2), (1, 4)]])

        Args:
          obj: A GraphSet, a graph (an edge list), an edge, or a
            vertex.

        Returns:
          A new GraphSet object.

        Raises:
          KeyError: If a given edge or a vertex is not found in the
            universe.

        See Also:
          excluding()
        """
        type, obj = GraphSet._conv_arg(obj)
        if type == 'graphset':
            return GraphSet(self._ss.supersets(obj._ss))
        elif type == 'graph':
            return self.including(GraphSet([obj]))
        elif type == 'edge':
            return GraphSet(self._ss.supersets(obj))
        else:
            return self.including(GraphSet([set([e]) for e in obj]))

    def excluding(self, obj):
        """Returns a new GraphSet that doesn't include `obj`.

        Returns a new set of graphs that don't include `obj`, which
        can be a GraphSet, a graph, an edge, or a vertex.  If `obj` is
        a GraphSet, a graph returned doesn't include *any of* graphs
        in the given GraphSet.

        The graphs stored in the new GraphSet are selected from `self`
        GraphSet.  The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> vertex = 4
          >>> gs.excluding(vertex)
          GraphSet([[(2, 3)]])

        Args:
          obj: A GraphSet, a graph (an edge list), an edge, or a
            vertex.

        Returns:
          A new GraphSet object.

        Raises:
          KeyError: If a given edge or vertex is not found in the
            universe.

        See Also:
          including()
        """
        type, obj = GraphSet._conv_arg(obj)
        if type == 'graphset':
#            return GraphSet(self._ss.non_supersets(obj._ss))  # correct but slow
            return self - self.including(obj)
        elif type == 'graph':
            return self.excluding(GraphSet([obj]))
        elif type == 'edge':
            return GraphSet(self._ss.non_supersets(obj))
        else:
            return self.excluding(GraphSet([set([e]) for e in obj]))

    def included(self, obj):
        """Returns a new GraphSet with subgraphs of a graph in `obj`.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2)]
          >>> graph2 = [(1, 2), (1, 4)]
          >>> graph3 = graph1 + [(2, 3)]
          >>> graph4 = [(1, 4), (2, 3)]
          >>> gs1 = GraphSet([graph1, graph2])
          >>> gs2 = GraphSet([graph3, graph4])
          >>> gs1.included(gs2)
          GraphSet([[(1, 2)]])

        Args:
          obj: A GraphSet or a graph (an edge list).

        Returns:
          A new GraphSet object.

        See Also:
          including()
        """
        type, obj = GraphSet._conv_arg(obj)
        if type == 'graphset':
            return GraphSet(self._ss.subsets(obj._ss))
        elif type == 'graph':
            return self.included(GraphSet([obj]))
        else:
            raise TypeError(obj)

    def choice(self):
        """Returns an arbitrary graph from `self`.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> gs.choice()
          [(1, 2), (1, 4)]

        Returns:
          A graph.

        Raises:
          KeyError: If `self` is empty.

        See Also:
          pop()
        """
        return GraphSet._conv_ret(self._ss.choice())

    def probability(self, probabilities):
        """Returns the probability of `self` with edge `probabilities`.

        This method calculates the probability of occurrence of any
        graph in `self` given `probabilities` of each edge.

        Examples:
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> probabilities = {(1, 2): .9, (1, 4): .8, (2, 3): .7}
          >>> gs.probability(probabilities)
          0.23

        Args:
          probabilities: A dictionary of probabilities of each edge.

        Returns:
          Probability.

        Raises:
          KeyError: If a given edge is not found in the universe.
        """
        probabilities = {GraphSet._conv_edge(e): p for e, p in probabilities.items()}
        return self._ss.probability(probabilities)

    def dump(self, fp):
        """Serialize `self` to a file `fp`.

        This method does not serialize the universe, which should be
        saved separately by pickle.

        Examples:
          >>> import pickle
          >>> fp = open('/path/to/graphset', 'wb')
          >>> gs.dump(fp)
          >>> fp = open('/path/to/universe' 'wb')
          >>> pickle.dump(GraphSet.universe(), fp)

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
          >>> graphset_str = gs.dumps()
          >>> universe_str = pickle.dumps(GraphSet.universe())

        See Also:
          dump(), loads()
        """
        return self._ss.dumps()

    def cost_le(self, costs, cost_bound):
        """Returns a new GraphSet with subgraphs whose cost is less than or equal to the cost bound.

        This method constructs a Graphset of subgraphs
        whose cost is less than or equal to the cost bound,
        where `costs` of each edge and the `cost_bound` are given as arguments.

        Examples:
          >>> universe = [(1, 2), (1, 4), (2, 3), (3, 4)]
          >>> GraphSet.set_universe(universe)

          >>> graph1 = [(1, 2), (2, 3)]
          >>> graph2 = [(3, 4)]
          >>> graph3 = [(1, 2), (1, 4), (3, 4)]
          >>> gs = GraphSet([graph1, graph2, graph3])
          >>> costs = {(1, 2): 2, (1, 4): 3, (2, 3): 1, (3, 4): 7}
          >>> cost_bound = 7
          >>> print(gs.cost_le(costs, cost_bound))
          GraphSet([[(3, 4)], [(1, 2), (2, 3)]])

        Args:
          costs: A dictionary of the cost of each edge.
          cost_bound: The upper limit of the cost of each graph. 32 bit signed integer.

        Returns:
          A new GraphSet object.

        Raises:
          KeyError: If a given edge is not found in the universe.
          AssertionError: If the cost of at least one edge is not given, or outside the range of 32 bit signed integer.
          TypeError: If at least one cost is not integer.

        """
        return GraphSet(self._ss.cost_le(costs=costs, cost_bound=cost_bound))

    def cost_ge(self, costs, cost_bound):
        """Returns a new GraphSet with subgraphs whose cost is greater than or equal to the cost bound.

        This method constructs a Graphset of subgraphs
        whose cost is greater than or equal to the cost bound,
        where `costs` of each edge and the `cost_bound` are given as arguments.

        Examples:
          >>> universe = [(1, 2), (1, 4), (2, 3), (3, 4)]
          >>> GraphSet.set_universe(universe)

          >>> graph1 = [(1, 2), (2, 3)]
          >>> graph2 = [(3, 4)]
          >>> graph3 = [(1, 2), (1, 4), (3, 4)]
          >>> gs = GraphSet([graph1, graph2, graph3])
          >>> costs = {(1, 2): 2, (1, 4): 3, (2, 3): 1, (3, 4): 7}
          >>> cost_bound = 7
          >>> print(gs.cost_ge(costs, cost_bound))
          GraphSet([[(3, 4)], [(1, 2), (1, 4), (3, 4)]])

        Args:
          costs: A dictionary of the cost of each edge.
          cost_bound: The lower limit of the cost of each graph. 32 bit signed integer.

        Returns:
          A new GraphSet object.

        Raises:
          KeyError: If a given edge is not found in the universe.
          AssertionError: If the cost of at least one edge is not given, or outside the range of 32 bit signed integer.
          TypeError: If at least one cost is not integer.

        """
        inv_costs = {e: -cost for e, cost in costs.items()}
        return GraphSet(self._ss.cost_le(costs=inv_costs, cost_bound=-cost_bound))

    def cost_eq(self, costs, cost_bound):
        """Returns a new GraphSet with subgraphs whose cost is equal to the cost bound.

        This method constructs a Graphset of subgraphs
        whose cost is equal to the cost bound,
        where `costs` of each edge and the `cost_bound` are given as arguments.

        Examples:
          >>> universe = [(1, 2), (1, 4), (2, 3), (3, 4)]
          >>> GraphSet.set_universe(universe)

          >>> graph1 = [(1, 2), (2, 3)]
          >>> graph2 = [(3, 4)]
          >>> graph3 = [(1, 2), (1, 4), (3, 4)]
          >>> gs = GraphSet([graph1, graph2, graph3])
          >>> costs = {(1, 2): 2, (1, 4): 3, (2, 3): 1, (3, 4): 7}
          >>> cost_bound = 7
          >>> print(gs.cost_eq(costs, cost_bound))
          GraphSet([[(3, 4)]])

        Args:
          costs: A dictionary of the cost of each edge.
          cost_bound: The limit of the cost of each graph. 32 bit signed integer.

        Returns:
          A new GraphSet object.

        Raises:
          KeyError: If a given edge is not found in the universe.
          AssertionError: If the cost of at least one edge is not given, or outside the range of 32 bit signed integer.
          TypeError: If at least one cost is not integer.

        """
        le_ss = self._ss.cost_le(costs=costs, cost_bound=cost_bound)
        lt_ss = self._ss.cost_le(costs=costs, cost_bound=cost_bound - 1)
        return GraphSet(le_ss.difference(lt_ss))

    def remove_some_edge(self):
        """Returns a new GraphSet with graphs that are obtained by removing some edge from a graph in `self`.

        The `self` is not changed.

        Examples:
          >>> GraphSet.set_universe([(1, 2), (1, 4), (2, 3)])
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> gs.remove_some_edge()
          GraphSet([[], [(1, 4)], [(1, 2)]])

        Returns:
          A new GraphSet object.
        """
        return GraphSet(self._ss.remove_some_element())

    def add_some_edge(self):
        """Returns a new GraphSet with graphs that are obtained by adding some edge to a graph in `self`.

        The `self` is not changed.

        Examples:
          >>> GraphSet.set_universe([(1, 2), (1, 4), (2, 3)])
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> gs.add_some_edge()
          GraphSet([[(1, 4), (2, 3)], [(1, 2), (2, 3)], [(1, 2), (1, 4), (2, 3)]])

        Returns:
          A new GraphSet object.
        """
        return GraphSet(self._ss.add_some_element(len(setset._int2obj) - 1))
    
    def remove_add_some_edges(self):
        """Returns a new GraphSet with graphs that are obtained by removing some edge from a graph in `self` and adding another edge to the graph.

        The `self` is not changed.

        Examples:
          >>> GraphSet.set_universe([(1, 2), (1, 4), (2, 3)])
          >>> graph1 = [(1, 2), (1, 4)]
          >>> graph2 = [(2, 3)]
          >>> gs = GraphSet([graph1, graph2])
          >>> gs.remove_add_some_edges()
          GraphSet([[(1, 4)], [(1, 2)], [(1, 4), (2, 3)], [(1, 2), (2, 3)]])

        Returns:
          A new GraphSet object.
        """
        return GraphSet(self._ss.remove_add_some_elements(len(setset._int2obj) - 1))

    def to_vertexsetset(self):
        """Returns a new VertexSetSet with vertices of each graph in `self`.

        Examples:
          >>> e1 = (1, 2)
          >>> e2 = (3, 4)
          >>> e3 = (2, 3)
          >>> e4 = (2, 4)
          >>> e5 = (2, 5)
          >>> GraphSet.set_universe([e1, e2, e3, e4, e5], "as-is")
          >>> VertexSetSet.set_universe()
          >>> g1 = [e2]           # vertex set is {3, 4}
          >>> g2 = [e1, e2]       # vertex set is {1, 2, 3, 4}
          >>> g3 = [e1, e2, e4]   # vertex set is {1, 2, 3, 4}
          >>> vss1 = GraphSet([g1, g2, g3]).to_vertexsetset()
          >>> vss1
          VertexSetSet([[3, 4], [1, 2, 3, 4]])

        Returns:
          A new VertexSetSet object.

        """
        return VertexSetSet(self._ss.to_vertexsetset())

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
          >>> GraphSet.set_universe(pickle.load(fp), traversal='as-is')
          >>> fp = open('/path/to/graphset')
          >>> gs = GraphSet.load(fp)

        See Also:
          loads(), dump()
        """
        return GraphSet(setset.load(fp))

    @staticmethod
    def loads(s):
        """Deserialize `s` to `self`.

        This method does not deserialize the universe, which should be
        loaded separately by pickle.

        Args:
          s: A string instance.

        Examples:
          >>> import pickle
          >>> GraphSet.set_universe(pickle.loads(universe_str), traversal='as-is')
          >>> gs = GraphSet.load(graphset_str)

        See Also:
          load(), dumps()
        """
        return GraphSet(setset.loads(s))

    @staticmethod
    def set_universe(universe, traversal='greedy', source=None):
        """Registers the new universe.

        Examples:
          >>> GraphSet.set_universe([(1, 2, 2.0), (1, 4, -3.0), (2, 3)])

        Args:
          universe: A list of edges that represents the new universe.
            An edge may come along with an edge weight, which can be
            positive as well as negative (or 1.0 if not specified).

          traversal: Optional.  This argument specifies the order of
            edges to be processed in the internal graphset operations.
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
        GraphSet._vertices = set()
        GraphSet._weights = {}
        universe = GraphSet.converters['to_edges'](universe)
        for e in universe:
            if e[:2] in indexed_edges or (e[1], e[0]) in indexed_edges:
                raise KeyError(e)
            sorted_edges.append(e[:2])
            indexed_edges.add(e[:2])
            if len(e) > 2:
                GraphSet._weights[e[:2]] = e[2]
        if traversal != 'as-is':
            if source is None:
                source = sorted_edges[0][0]
                for e in sorted_edges:
                    source = min(e[0], e[1], source)
            sorted_edges = GraphSet._traverse(indexed_edges, traversal, source)
        for u, v in sorted_edges:
            GraphSet._vertices.add(u)
            GraphSet._vertices.add(v)
        setset.set_universe(sorted_edges)

    @staticmethod
    def universe():
        """Returns the current universe.

        The list of edges that represents the current universe is
        returned.

        Examples:
          >>> GraphSet.universe()
          [(1, 2, 2.0), (1, 4, -3.0), (2, 3)]

        Returns:
          The universe if no argument is given, or None otherwise.

        See Also:
          set_universe()
        """
        edges = []
        for e in setset.universe():
            if e in GraphSet._weights:
                edges.append((e[0], e[1], GraphSet._weights[e]))
            else:
                edges.append(e)
        return GraphSet.converters['to_graph'](edges)

    @staticmethod
    def graphs(vertex_groups=None, degree_constraints=None, num_edges=None,
               no_loop=False, graphset=None, linear_constraints=None):
        """Returns a GraphSet with graphs under given constraints.

        This is the base method for specific graph classes, e.g.,
        paths and trees.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples: a set of paths from vertex 1 to vertex 6
          >>> start = 1
          >>> end = 6
          >>> zero_or_two = range(0, 3, 2)
          >>> degree_constraints = {start: 1, end: 1,
          ...                       2: zero_or_two, 3: zero_or_two,
          ...                       4: zero_or_two, 5: zero_or_two}
          >>> GraphSet.graphs(vertex_groups=[[start, end]],
          ...                 degree_constraints=degree_constraints,
          ...                 no_loop=True)
          GraphSet([[(1, 2), (2, 3), (3, 6)], [(1, 2), (2, 5), (5, 6)], [(1, 4), (4, 5 ...

        Args:
          vertex_groups: Optional.  A nested list.  Vertices in an
            inner list are connected while those in different inner
            lists are disconnected.  For `[[1, 5], [3]]`, 1 and 5 are
            connected, while they are not connected with 3.

          degree_constraints: Optional.  A dict with a vertex and a
            range or int.  The degree of a vertex is restricted by the
            range.  For `{1: 2, 6: range(2)}`, the degree of vertex 1
            is 2 and that of 6 is less than 2, while others are not
            cared.

          num_edges: Optional.  A range or int.  This argument
            specifies the number of edges used in graphs to be stored.
            For `range(5)`, less than 5 edges can be used.

          no_loop: Optional.  True or False.  This argument specifies
            if loop is not allowed.

          graphset: Optional.  A GraphSet object.  Graphs to be stored
            are selected from this object.

          linear_constraints: Optional.  A list of linear constraints.
            A linear constraint consists of weighted edges and
            lower/upper bounds.  An edge weight is a positive or
            negative number, which defaults to 1.  Weights of the edges
            that are not included in the constraint are zeros.  For
            instance, `linear_constraints=[([(1, 2, 0.6), (2, 5),
            (3, 6, 1.2)], (1.5, 2.0))]`, feasible graph weights are
            between 1.5 and 2.0, e.g., `[(1, 2), (2, 3), (3, 6)]` or
            `[(1, 2), (2, 5), (5, 6)]`.
            See graphillion/test/graphset.py in detail.

        Returns:
          A new GraphSet object.

        See Also:
          connected_components(), cliques(), trees(), forests(),
          cycles(), paths()

        """
        graph = []
        for e in setset.universe():
            assert e[0] in GraphSet._vertices and e[1] in GraphSet._vertices
            graph.append((pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        vg = []
        nc = 0
        if vertex_groups is not None:
            for vs in vertex_groups:
                if len(vs) == 0:
                    nc += 1
                else:
                    for v in vs:
                        if v not in GraphSet._vertices:
                            raise KeyError(v)
                    vg.append([pickle.dumps(v, protocol=0) for v in vs])
        if not vg and nc == 0:
            nc = -1

        dc = None
        if degree_constraints is not None:
            dc = {}
            for v, r in viewitems(degree_constraints):
                if v not in GraphSet._vertices:
                    raise KeyError(v)
                if isinstance(r, int):
                    dc[pickle.dumps(v, protocol=0)] = (r, r + 1, 1)
                elif len(r) == 1:
                    dc[pickle.dumps(v, protocol=0)] = (r[0], r[0] + 1, 1)
                else:
                    dc[pickle.dumps(v, protocol=0)] = (r[0], r[-1] + 1, r[1] - r[0])

        ne = None
        if num_edges is not None:
            if isinstance(num_edges, int):
                ne = (num_edges, num_edges + 1, 1)
            elif len(num_edges) == 1:
                ne = (num_edges[0], num_edges[0] + 1, 1)
            else:
                ne = (num_edges[0], num_edges[-1] + 1,
                      num_edges[1] - num_edges[0])

        ss = None if graphset is None else graphset._ss

        lc = None
        if linear_constraints is not None:
            lc = []
            for c in linear_constraints:
                expr = []
                for we in c[0]:
                    u = pickle.dumps(we[0], protocol=0)
                    v = pickle.dumps(we[1], protocol=0)
                    w = float(we[2]) if len(we) >= 3 else 1.0
                    expr.append((u, v, w))
                min = float(c[1][0])
                max = float(c[1][1])
                lc.append((expr, (min, max)))

        ss = _graphillion._graphs(graph=graph, vertex_groups=vg,
                                  degree_constraints=dc, num_edges=ne,
                                  num_comps=nc, no_loop=no_loop,
                                  search_space=ss,
                                  linear_constraints=lc)
        return GraphSet(ss)

    @staticmethod
    def connected_components(vertices, graphset=None):
        """Returns a GraphSet of connected components.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> GraphSet.connected_components([1, 3, 5])
          GraphSet([[(1, 2), (2, 3), (2, 5)], [(1, 2), (1, 4), (2, 3), (2, 5)], [(1, 2 ...

        Args:
          vertices: A list of vertices to be connected.

          graphset: Optional.  A GraphSet object.  Components to be
            stored are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """
        return GraphSet.graphs(vertex_groups=[vertices], graphset=graphset)

    @staticmethod
    def cliques(k, graphset=None):
        """Returns a GraphSet of k-cliques.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> GraphSet.set_universe([(1, 2), (1, 3), (1, 4), (1, 5), (2, 3), (2, 4),
                                     (2, 5), (3, 4), (3, 5), (4, 5)])
          >>> GraphSet.cliques(4)
          GraphSet([[(1, 2), (1, 3), (1, 4), (2, 3), (2, 4), (3, 4)], [(1, 2), (1, 3), ...

        Args:
          k: An integer.  The number of vertices in a clique.

          graphset: Optional.  A GraphSet object.  Cliques to be
            stored are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """
        dc = {}
        for v in GraphSet._vertices:
            dc[v] = range(0, k, k - 1)
        ne = range(k * (k - 1) // 2, k * (k - 1) // 2 + 1)
        return GraphSet.graphs(vertex_groups=[[]], degree_constraints=dc,
                               num_edges=ne, graphset=graphset)

    @staticmethod
    def bicliques(a, b, graphset=None):
        """Returns a GraphSet of (a, b)-bicliques ((a, b)-complete bipartite graphs).
        Currently, the case of a == b is supported.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> GraphSet.set_universe([(1, 2), (1, 3), (1, 4), (1, 5), (2, 3), (2, 4),
                                     (2, 5), (3, 4), (3, 5), (4, 5)])
          >>> GraphSet.bicliques(4)

        Args:
          a: An integer.  The number of degrees of the vertices in the left part.
          b: An integer.  The number of degrees of the vertices in the right part.
            a == b must hold.

          graphset: Optional.  A GraphSet object.  Cliques to be
            stored are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """
        if a != b:
            TypeError('a == b must hold.')
        gs = GraphSet.regular_bipartite_graphs(degree=a,
                                                is_connected=True,
                                                graphset=graphset)
        return gs.graphs(num_edges=a*a)

    @staticmethod
    def trees(root=None, is_spanning=False, graphset=None):
        """Returns a GraphSet of trees.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> GraphSet.trees(1, is_spanning=True)
          GraphSet([[(1, 2), (1, 4), (2, 3), (2, 5), (3, 6)], [(1, 2), (1, 4), (2, 3), ...

        Args:
          root:  Optional.  A vertex, at which trees are rooted.

          is_spanning: Optional.  True or False.  If true, trees must
            be composed of all vertices.

          graphset: Optional.  A GraphSet object.  Trees to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """
        vg = [[]] if root is None else [[root]]
        dc = None
        if is_spanning:
            dc = {}
            for v in GraphSet._vertices:
                dc[v] = range(1, len(GraphSet._vertices))
        return GraphSet.graphs(vertex_groups=vg, degree_constraints=dc,
                               no_loop=True, graphset=graphset)

    @staticmethod
    def forests(roots, is_spanning=False, graphset=None):
        """Returns a GraphSet of forests, sets of trees.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> GraphSet.forests([1, 6])
          GraphSet([[], [(1, 2)], [(1, 4)], [(3, 6)], [(5, 6)], [(1, 2), (1, 4)], [(1, ...

        Args:
          roots: Optional.  A list of vertices, at which trees are
            rooted.

          is_spanning: Optional.  True or False.  If true, forests must
            be composed of all vertices.

          graphset: Optional.  A GraphSet object.  Forests to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """
        vg = [[r] for r in roots]
        dc = None
        if is_spanning:
            dc = {}
            for v in GraphSet._vertices:
                if v not in roots:
                    dc[v] = range(1, len(GraphSet._vertices))
        return GraphSet.graphs(vertex_groups=vg, degree_constraints=dc,
                               no_loop=True, graphset=graphset)

    @staticmethod
    def cycles(is_hamilton=False, graphset=None):
        """Returns a GraphSet of cycles.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> GraphSet.cycles(is_hamilton=True)
          GraphSet([[(1, 2), (1, 4), (2, 3), (3, 6), (4, 5), (5, 6)]])

        Args:
          is_hamilton: Optional.  True or False.  If true, cycles must
            be composed of all vertices.

          graphset: Optional.  A GraphSet object.  Cycles to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """
        dc = {}
        for v in GraphSet._vertices:
            dc[v] = 2 if is_hamilton else range(0, 3, 2)
        return GraphSet.graphs(vertex_groups=[[]], degree_constraints=dc,
                               graphset=graphset)

    @staticmethod
    def paths(terminal1=None, terminal2=None, is_hamilton=False, graphset=None):
        """Returns a GraphSet of paths.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> GraphSet.paths(1, 6)
          GraphSet([[(1, 2), (2, 3), (3, 6)], [(1, 2), (2, 5), (5, 6)], [(1, 4), (4, 5 ...

        Args:
          terminal1 and terminal2: Both end vertices of a paths. If terminal1 != None
            and terminal2 == None, all paths start from terminal1.
            If terminal1 == None and terminal2 == None, Both end vertices are arbitrary.

          graphset: Optional.  A GraphSet object.  Paths to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """
        if terminal2 == None:
            if is_hamilton:
                deg_dist = {1: 2, 2: GraphSet.DegreeDistribution_Any}
            else:
                deg_dist = {0: GraphSet.DegreeDistribution_Any,
                            1: 2, 2: GraphSet.DegreeDistribution_Any}
            gs = GraphSet.degree_distribution_graphs(deg_dist, True, graphset=graphset)
            if terminal1 == None:
              return gs
            else:
                dc = {}
                for v in GraphSet._vertices:
                    if v == terminal1:
                        dc[v] = 1
                    else:
                        dc[v] = range(1, 3) if is_hamilton else range(0, 3)
                return GraphSet.graphs(degree_constraints=dc, no_loop=True,
                                      graphset=gs)
        else:
            dc = {}
            for v in GraphSet._vertices:
                if v in (terminal1, terminal2):
                    dc[v] = 1
                else:
                    dc[v] = 2 if is_hamilton else range(0, 3, 2)
            return GraphSet.graphs(vertex_groups=[[terminal1, terminal2]],
                                  degree_constraints=dc,
                                  no_loop=True, graphset=graphset)

    @staticmethod
    def matchings(graphset=None):
        """Returns a GraphSet of matchings.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> GraphSet.matchings()
          GraphSet([[], [(1, 4)], [(4, 5)], [(1, 2)], [(2, 5)], [(2, 3)], [(3, 6)], [( ...

        Args:
          graphset: Optional.  A GraphSet object.  Matchings to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """
        return GraphSet.k_matchings(1, graphset)

    @staticmethod
    def perfect_matchings(graphset=None):
        """Return a GraphSet of perfect matchings.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> GraphSet.perfect_matchings()
          GraphSet([[(1, 4), (2, 5), (3, 6)], [(1, 2), (3, 6), (4, 5)], [(1, 4), (2, 3 ...

        Args:
          graphset: Optional.  A GraphSet object. Matchings to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """
        dc = {}
        for v in GraphSet._vertices:
          dc[v] = 1
        return GraphSet.graphs(degree_constraints=dc, graphset=graphset)

    @staticmethod
    def k_matchings(k, graphset=None):
        """Returns a GraphSet of k-matchings.
        A k-matching is a set of edges such that each vertex is incident
        to at most k edges of the k-matching.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> GraphSet.k_matchings(3)

        Args:
          k: Integer. Each vertex is incident to at most k edges.
          graphset: Optional.  A GraphSet object.  Matchings to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """
        dc = {}
        for v in GraphSet._vertices:
          dc[v] = range(0, k + 1)
        return GraphSet.graphs(degree_constraints=dc, graphset=graphset)

    @staticmethod
    def b_matchings(b, graphset=None):
        """Returns a GraphSet of b-matchings.
        For a function b that maps from a vertex to an integer,
        a b-matching is a set of edges such that each vertex v is incident
        to at most b(v) edges of the b-matching.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> b = {}
          # vertices is a list of vertices in the universe
          >>> for v in vertices:
          >>>     b[v] = 1 if v == 1 else 2
          >>> GraphSet.b_matchings(b)

        Args:
          b: Dictionary. A key is a vertex of the universe graph and a value
            is an integer. For a vertex v, if b[v] is undefined, it means b[v] == 0.
          graphset: Optional.  A GraphSet object.  Matchings to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """
        dc = {}
        for v in GraphSet._vertices:
            if v in b:
                dc[v] = range(0, b[v] + 1)
            else:
                dc[v] = 0
        return GraphSet.graphs(degree_constraints=dc, graphset=graphset)

    @staticmethod
    def k_factors(k, graphset=None):
        """Returns a GraphSet of k-factors.
        A k-factor is a set of edges such that each vertex is incident
        to exactly k edges of the k-factor.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> GraphSet.k_factors(3)

        Args:
          k: Integer. Each vertex is incident to exactly k edges.
          graphset: Optional.  A GraphSet object.  Matchings to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """
        dc = {}
        for v in GraphSet._vertices:
          dc[v] = k
        return GraphSet.graphs(degree_constraints=dc, graphset=graphset)

    @staticmethod
    def f_factors(f, graphset=None):
        """Returns a GraphSet of f-factors.
        For a function f that maps from a vertex to an integer,
        an f-factor is a set of edges such that each vertex v is incident
        to exactly f(v) edges of the f-factor.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> f = {}
          # vertices is a list of vertices in the universe
          >>> for v in vertices:
          >>>     f[v] = 1 if f == 1 else 2
          >>> GraphSet.f_factors(f)

        Args:
          f: Dictionary. A key is a vertex of the universe graph and a value
            is an integer. For a vertex v, if f[v] is undefined, it means f[v] == 0.
          graphset: Optional.  A GraphSet object.  Matchings to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """
        dc = {}
        for v in GraphSet._vertices:
            if v in f:
                dc[v] = f[v]
            else:
                dc[v] = 0
        return GraphSet.graphs(degree_constraints=dc, graphset=graphset)

    @staticmethod
    def regular_graphs(degree=None, is_connected=True, graphset=None):
        """Returns a GraphSet of regular graphs.
        A regular graph is one such that all the degrees of the vertices are the same.

        This method can be parallelized with OpenMP by specifying the
        environmental variable `OMP_NUM_THREADS`:

          `$ OMP_NUM_THREADS=4 python your_graphillion_script.py`

        Examples:
          >>> GraphSet.regular_graphs()

          # The degree is 3 and the graphs are not necessarily connected.
          >>> GraphSet.regular_graphs(3, False)

          # The degree is at least 2 and at most 5, and
          # the graphs are connected.
          >>> GraphSet.regular_graphs((2, 5), True)

        Args:
          degree: Tuple or Integer. If it is a tuple (l, u),
            the degree is at least l and at most u.
            If it is an integer d, the degree is d.
            If it is None, the degree is arbitrary.
          is_connected: Bool. If it is True, the graphs are
            connected. If it is False, the graphs are
            not necessarily connected.
          graphset: Optional.  A GraphSet object.  Matchings to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.

        See Also:
          graphs()
        """

        graph = []
        for e in setset.universe():
            assert e[0] in GraphSet._vertices and e[1] in GraphSet._vertices
            graph.append((pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        if degree == None:
            degree = (1, len(GraphSet._vertices))

        ss = None if graphset is None else graphset._ss

        ss = _graphillion._regular_graphs(graph=graph,
                                          degree=degree,
                                          is_connected=is_connected,
                                          graphset=ss)
        return GraphSet(ss)

    @staticmethod
    def bipartite_graphs(is_connected=True, graphset=None):
        """Returns a GraphSet of bipartite subgraphs.

        Example:
          >>> GraphSet.bipartite_graphs()
          GraphSet([[], [(1, 4)], [(4, 5)], [(1, 2)], [(2, 5)], [(2, 3)], [(3, 6)], [( ...

        Args:
          graphset: Optional. A GraphSet object. Subgraphs to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.
        """
        graph = []
        for e in setset.universe():
            assert e[0] in GraphSet._vertices and e[1] in GraphSet._vertices
            graph.append(
                (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        odd_gs = GraphSet(_graphillion._odd_edges_subgraphs(graph))
        odd_cycle_gs = odd_gs.cycles()

        if graphset is None:
            gs = GraphSet({}).non_supergraphs(odd_cycle_gs)
        else:
            gs = graphset.non_supergraphs(odd_cycle_gs)

        if is_connected:
            # The empty set is a bipartite graph.
            return gs.graphs(vertex_groups=[[]]) | GraphSet([[]])
        else:
            return gs

    @staticmethod
    def regular_bipartite_graphs(degree=None, is_connected=True, graphset=None):
        """Returns a GraphSet of regular bipartite subgraphs.

        Example:
          >>> GraphSet.regular_bipartite_graphs()

          # The degree is 3 and the graphs are not necessarily connected.
          >>> GraphSet.regular_graphs(3, False)

          # The degree is at least 2 and at most 5, and
          # the graphs are connected.
          >>> GraphSet.regular_graphs((2, 5), True)

        Args:
          degree: Tuple or Integer. If it is a tuple (l, u),
            the degree is at least l and at most u.
            If it is an integer d, the degree is d.
            If it is None, the degree is arbitrary.
          is_connected: Bool. If it is True, the graphs are
            connected. If it is False, the graphs are
            not necessarily connected.
          graphset: Optional. A GraphSet object. Subgraphs to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.
  
        See Also:
          regular_graphs()
          bipartite_graphs()
        """
        bi_graphs = GraphSet.bipartite_graphs(is_connected=is_connected, graphset=graphset)
        return GraphSet.regular_graphs(degree=degree, is_connected=is_connected, graphset=bi_graphs)

    @staticmethod
    def steiner_subgraphs(terminals, graphset=None):
        """Returns a GraphSet of Steiner subgraphs.
          A Steiner subgraph is a subgraph that contains all vertices in "terminals".

        Example:
          >>> GraphSet.steiner_subgraphs([1, 2, 3])

        Args:
          terminals: A list of vertices to be connected.
          graphset: Optional. A GraphSet object. Subgraphs to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.
        """
        return GraphSet.connected_components(terminals, graphset)

    @staticmethod
    def steiner_trees(terminals, graphset=None):
        """Returns a GraphSet of Steiner trees.
          A Steiner tree is a tree that contains all vertices in "terminals".

        Example:
          >>> GraphSet.steiner_trees([1, 2, 3])

        Args:
          terminals: A list of vertices to be connected.
          graphset: Optional. A GraphSet object. Subgraphs to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.
        """
        return GraphSet.graphs(vertex_groups=[terminals],
                               no_loop=True, graphset=graphset)

    @staticmethod
    def steiner_cycles(terminals, graphset=None):
        """Returns a GraphSet of Steiner cycles.
          A Steiner cycle is a cycle that contains all vertices in "terminals".

        Example:
          >>> GraphSet.steiner_cycles([1, 2, 3])

        Args:
          terminals: A list of vertices to be connected.
          graphset: Optional. A GraphSet object. Subgraphs to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.
        """
        dc = {}
        for v in GraphSet._vertices:
            dc[v] = range(0, 3, 2)
        return GraphSet.graphs(vertex_groups=[terminals], degree_constraints=dc,
                               graphset=graphset)

    @staticmethod
    def steiner_paths(terminals, graphset=None):
        """Returns a GraphSet of Steiner paths.
          A Steiner path is a path that contains all vertices in "terminals".

        Example:
          >>> GraphSet.steiner_paths([1, 2, 3])

        Args:
          terminals: A list of vertices to be connected.
          graphset: Optional. A GraphSet object. Subgraphs to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.
        """
        gs = GraphSet.paths(graphset)
        return GraphSet.graphs(vertex_groups=[terminals], graphset=gs)

    DegreeDistribution_Any = -1

    @staticmethod
    def degree_distribution_graphs(deg_dist, is_connected, graphset=None):
        """Returns a GraphSet having specified degree distribution.

        Examples:
            >>> GraphSet.set_universe([(1, 2), (1, 4), (2, 3),
                                        (2, 5), (3, 6), (4, 5),
                                        (5, 6)])
            >>> deg_dist = {0: GraphSet.DegreeDistribution_Any,
                            1: 2, 2: 1}
            # This means that each subgraph has 2 vertices with degree 1,
            # 1 vertex with degree 2, and any number of vertices with
            # degree 0.
            >>> gs = GraphSet.degree_distribution_graphs(deg_dist, True)
        Args:
          deg_dist: dictionary whose key and value mean that
                    each subgraph has 'value' number of vertices
                    with degree 'key'. If the value is
                    GraphSet.DegreeDistribution_Any, it means that
                    each subgraph has any number of vertices
                    with degree 'key'.
          connected: Each subgraph is connected if True.
                      Each subgraph is not necessarily connected if False.

        Returns:
            A new GraphSet object.
        """
        graph = []
        for e in setset.universe():
            assert e[0] in GraphSet._vertices and e[1] in GraphSet._vertices
            graph.append(
                (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        ss = _graphillion._degree_distribution_graphs(graph, deg_dist, is_connected, graphset)
        return GraphSet(ss)

    @staticmethod
    def letter_P_graphs(graphset=None):
        """Returns a GraphSet whose shape looks like letter 'P'.
            That is, each subgraph has one vertex with degree 1,
            one vertex with degree 3, and any number of vertices with
            degree 2, and is connected.

        Examples:
            >>> gs = GraphSet.letter_P_graphs()

        Returns:
            A new GraphSet object.
        """
        deg_dist = {0: GraphSet.DegreeDistribution_Any, 1: 1, 2: GraphSet.DegreeDistribution_Any, 3: 1}
        return GraphSet.degree_distribution_graphs(deg_dist, is_connected=True, graphset=graphset)

    @staticmethod
    def partitions(num_comp_lb=1, num_comp_ub=32767):
        """Returns a GraphSet with partitions of the graph.
        Examples: partitions with two or three connected components.
          >>> lb = 2
          >>> ub = 3
          >>> GraphSet.partitions(num_comp_lb=lb,num_comp_ub=ub)
          GraphSet([[(1, 4), (2, 3), (4, 5)], [(1, 2), (1, 4), (2, 3)], [(1, 4), (3, 6 ...

        Args:
          num_comp_lb: Optional. int. the lower bound of the number of 
            connected components. (including)
          num_comp_ub: Optional. int. the upper bound of the number of
            connected components. (including)

        Returns:
          A new GraphSet object.
        """
        graph = []
        for e in setset.universe():
            assert e[0] in GraphSet._vertices and e[1] in GraphSet._vertices
            graph.append(
              (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        ss = _graphillion._partitions(
          graph=graph, num_comp_lb=num_comp_lb, num_comp_ub=num_comp_ub)
        return GraphSet(ss)

    @staticmethod
    def balanced_partitions(weight_list=None, ratio=0.0, lower=0, upper=4294967295 // 4, num_comps=-1):
        """Returns a GraphSet with balanced partitions of the graph.

        Examples: balanced partitions with disparity less than or equal to 2.0.
          >>> wl = {}
          >>> for v in range(1,7):
          >>>   if v % 2:
          >>>     wl[v] = 1
          >>>   else:
          >>>     wl[v] = 2
          >>> gs = GraphSet.balanced_partitions(weight_list=wl, ratio=2, num_comps=2, lower=2)
          GraphSet([[(1, 4), (2, 3), (3, 6), (4, 5)], [(1, 4), (2, 3), (4, 5), (5, 6)] ...

        Args:
          weight_list: Optional. A list of int. Vertex weights.
          ratio: Optional. a floating point number more than or equal to 1.0.
          lower: Optional. int. the lower bound of the sum of vertex weights
            in each connected component. (including)
          upper: Optional. int. the upper bound of the sum of vertex weights
            in each connected component. (including)
          num_comps: Optional. int. the number of connected components.

        Returns:
          A new GraphSet object.
        """
        graph = []
        for e in setset.universe():
            assert e[0] in GraphSet._vertices and e[1] in GraphSet._vertices
            graph.append(
                (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        wl = None
        if weight_list is not None:
            wl = {}
            for v, r in viewitems(weight_list):
                if v not in GraphSet._vertices:
                    raise KeyError(v)
                wl[pickle.dumps(v, protocol=0)] = r

        ss = _graphillion._balanced_partitions(
            graph=graph, weight_list=wl, ratio=ratio, lower=lower, upper=upper, num_comps=num_comps)
        return GraphSet(ss)

    @staticmethod
    def induced_graphs():
        """Return a GraphSet with connected induced graphs.

        Example: all connected induced graphs (more than a vertex)
          >>> gs = GraphSet.induced_graphs()
          GraphSet([[(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5), (5, 6)],
          [(1, 2), (1, 4), (2, 3), (2, 5), (4, 5)], [(1, 2), (1, 4), (2, 5), (4, 5), (5, 6)]] ...

        Returns:
          A new GraphSet Object.
        """
        graph = []
        for e in setset.universe():
            assert e[0] in GraphSet._vertices and e[1] in GraphSet._vertices
            graph.append(
                (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        ss = _graphillion._induced_graphs(graph=graph)
        return GraphSet(ss)

    @staticmethod
    def weighted_induced_graphs(weight_list=None, lower=0, upper=4294967295//2):
        """Return a GraphSet with weighted connected induced graphs.

        Examples: weighted connected induced graphs
          >>> wl = {}
          >>> for v in range(1, 7):
          >>>   wl[v] = v
          >>> gs = GraphSet.weighted_induced_graphs(weight_list=wl, lower=10, upper=17)
          GraphSet([[(5, 6)], [(1, 4), (4, 5)], [(2, 5), (4, 5)], [(2, 3), (2, 5)], [( ...

        Args:
          weight_list: Optional. A list of int. Vertex weights. default weight is 1.
          lower: Optional. int. the lower bound of the sum of vertex weights
            in each connected component. (including)
          upper: Optional. int. the upper bound of the sum of vertex weights
            in each connected component. (including)

        Returns:
          A new GraphSet Object.
        """
        graph = []
        for e in setset.universe():
            assert e[0] in GraphSet._vertices and e[1] in GraphSet._vertices
            graph.append(
                (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        wl = None
        if weight_list is not None:
            wl = {}
            for v, r in viewitems(weight_list):
                if v not in GraphSet._vertices:
                    raise KeyError(v)
                wl[pickle.dumps(v, protocol=0)] = r

        ss = _graphillion._weighted_induced_graphs(graph=graph, weight_list=wl, lower=lower, upper=upper)
        return GraphSet(ss)

    @staticmethod
    def forbidden_induced_subgraphs(forbidden_graphset=None):
        """Returns a GraphSet characterized by forbidden induced subgraphs.

        Examples:
            >>> GraphSet.forbidden_induced_subgraphs(GraphSet.cycles())

        Returns:
            A new GraphSet object.
        """
        graph = []
        for e in setset.universe():
            assert e[0] in GraphSet._vertices and e[1] in GraphSet._vertices
            graph.append(
                (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        ss = None if forbidden_graphset is None else forbidden_graphset._ss

        ss = _graphillion._forbidden_induced_subgraphs(graph=graph, graphset=ss)
        return GraphSet(ss)

    @staticmethod
    def chordal_graphs():
        raise TypeError('chordal_graphs moved to GraphClass.chordal_graphs()')

    @staticmethod
    def reliability(probabilities, terminals):
        """Returns the reliability of the graph with edge `probabilities` and `terminals`.
        This method calculates the reliability of the graph with `probabilities` of each edge
        and terminals.

        Examples:
          >>> probabilities = {(1, 2): 0.5, (1, 4): 0.5}
          >>> terminals = [1, 2, 3, 4, 5, 6]
          >>> reliability = GraphSet.reliability(prob_list, terminals)
          >>> reliability
          0.75

        Args:
          probabilities: a dictionary of probabilities of each edge.
          terminals: terminal vertices.

        Returns:
          Reliability.

        Raises:
          KeyError: If a given edge is not found in the universe.
        """
        graph = []
        for e in setset.universe():
            assert e[0] in GraphSet._vertices and e[1] in GraphSet._vertices
            graph.append(
                (pickle.dumps(e[0], protocol=0), pickle.dumps(e[1], protocol=0)))

        terms = []
        if terminals is not None:
            for v in terminals:
                if v not in GraphSet._vertices:
                    raise KeyError(v)
                terms.append(pickle.dumps(v, protocol=0))

        ps = [1.0] * (_graphillion._num_elems())
        if probabilities is not None:
          for e, p in viewitems(probabilities):
              i = setset._obj2int[e]
              ps[i - 1] = p

        reliability = _graphillion._reliability(
            graph=graph, probabilities=ps, terminals=terms)
        return reliability

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
                        if degree[v]:
                            for w in sorted(neighbors[v]):
                                if w not in visited_vertices:
                                    heapq.heappush(heap, (degree[v], degree[w], w))
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
                new_vertices = neighbors[u] - visited_vertices - set(queue_or_stack)
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
            raise ValueError('invalid `traversal`: %s' % traversal)

    @staticmethod
    def _conv_arg(obj):
        if isinstance(obj, GraphSet):
            return 'graphset', obj
        elif isinstance(obj, (set, frozenset, list)):
            return 'graph', set([GraphSet._conv_edge(e) for e in obj])
        elif isinstance(obj, tuple):
            return 'edge', GraphSet._conv_edge(obj)
        elif obj in GraphSet._vertices:
            return 'vertex', [e for e in setset.universe() if obj in e]
        try:
            edges = GraphSet.converters['to_edges'](obj)
            return 'graph', set([GraphSet._conv_edge(e) for e in edges])
        except TypeError:  # if fail to convert obj into edge list
            raise KeyError(obj)

    @staticmethod
    def _conv_graph(obj):
        return GraphSet.converters['to_edges'](obj)

    @staticmethod
    def _conv_edge(edge):
        if not isinstance(edge, tuple) or len(edge) < 2:
            raise KeyError(edge)
        if len(edge) > 2:
            edge = edge[:2]
        if edge in setset._obj2int:
            return edge
        elif (edge[1], edge[0]) in setset._obj2int:
            return (edge[1], edge[0])
        raise KeyError(edge)

    @staticmethod
    def _conv_ret(obj):
        if isinstance(obj, (set, frozenset)):  # a graph
            return GraphSet.converters['to_graph'](sorted(list(obj)))
        raise TypeError(obj)

    converters = { 'to_graph': lambda edges: edges,
                   'to_edges': lambda graph: graph }

    _vertices = set()
    _weights = {}
