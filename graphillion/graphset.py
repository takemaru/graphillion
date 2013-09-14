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
import _graphillion
from graphillion import setset
import pickle

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
      3

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
                for k, l in obj.iteritems():
                    d[k] = [GraphSet._conv_edge(e) for e in l]
                obj = d
            self._ss = setset(obj)
        methods = ['graphs', 'connected_components', 'cliques', 'trees',
                   'forests', 'cycles', 'paths']
        for method in methods:
            expr = 'self.%s = partial(GraphSet.%s, graphset=self)'
            exec expr % (method, method)

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

#    def quotient(self, other):
#        """Returns a new GraphSet of quotient.
#
#        The quotient is defined by,
#          gs1 / gs2 = {a | a \\cup b \\in gs1 and a \\cap b = \\empty, \\forall b \\in gs2}.
#        D. Knuth, Exercise 204, The art of computer programming,
#        Sect.7.1.4.
#
#        The `self` is not changed.
#
#        Examples:
#          >>> graph1 = [(1, 2), (1, 4)]
#          >>> graph2 = [(2, 3), (2, 5)]
#          >>> graph3 = [(1, 4)]
#          >>> gs = GraphSet([graph1, graph2])
#          >>> gs / GraphSet([graph3])
#          GraphSet([[(1, 2)]])
#
#        Returns:
#          A new GraphSet object.
#
#        See Also:
#          remainder(), quotient_update()
#        """
#        return GraphSet(self._ss.quotient(other._ss))

#    def remainder(self, other):
#        """Returns a new GraphSet of remainder.
#
#        The remainder is defined by,
#          gs1 % gs2 = gs1 - (gs1 \\sqcup (gs1 / gs2)).
#        D. Knuth, Exercise 204, The art of computer programming,
#        Sect.7.1.4.
#
#        The `self` is not changed.
#
#        Examples:
#          >>> graph1 = [(1, 2), (1, 4)]
#          >>> graph2 = [(2, 3), (2, 5)]
#          >>> graph3 = [(1, 4)]
#          >>> gs = GraphSet([graph1, graph2])
#          >>> gs % GraphSet([graph3])
#          GraphSet([[(2, 3), (2, 5)]])
#
#        Returns:
#          A new GraphSet object.
#
#        See Also:
#          quotient(), remainder_update()
#        """
#        return GraphSet(self._ss.remainder(other._ss))

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

#    def quotient_update(self, other):
#        """Updates `self` by the quotient.
#
#        Examples:
#          >>> graph1 = [(1, 2), (1, 4)]
#          >>> graph2 = [(2, 3), (2, 5)]
#          >>> graph3 = [(1, 4)]
#          >>> gs = GraphSet([graph1, graph2])
#          >>> gs /= GraphSet([graph3])
#          >>> gs
#          GraphSet([[(1, 2)]])
#
#        Returns:
#          A new GraphSet object.
#
#        See Also:
#          quotient()
#        """
#        self._ss.quotient_update(other._ss)
#        return self

#    def remainder_update(self, other):
#        """Updates `self` by the remainder.
#
#        Examples:
#          >>> graph1 = [(1, 2), (1, 4)]
#          >>> graph2 = [(2, 3), (2, 5)]
#          >>> graph3 = [(1, 4)]
#          >>> gs = GraphSet([graph1, graph2])
#          >>> gs %= GraphSet([graph3])
#          >>> gs
#          GraphSet([[(2, 3), (2, 5)]])
#
#        Returns:
#          A new GraphSet object.
#
#        See Also:
#          remainder()
#        """
#        self._ss.remainder_update(other._ss)
#        return self

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
#    __div__ = quotient
#    __mod__ = remainder

    __ior__ = update
    __iand__ = intersection_update
    __isub__ = difference_update
    __ixor__ = symmetric_difference_update
#    __idiv__ = quotient_update
#    __imod__ = remainder_update

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
        GraphSet with graphs that have `size` edges.

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
          >>> gs.same_len(2)
          GraphSet([[(1, 2), (1, 4)]])

        Args:
          size: Optional.  The number of edges in a graph.

        Returns:
          The number of graphs, or a new GraphSet object.

        See Also:
          __len__(), smaller(), larger()
        """
        if size is None:
            return self._ss.len()
        else:
            return GraphSet(self._ss.len(size))

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
            yield GraphSet._conv_ret(g)

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
            yield GraphSet._conv_ret(g)

    def min_iter(self, weights=None):
        """Iterates over graphs in the ascending order of weights.

        Returns a generator that iterates over graphs in `self`
        GraphSet.  The graphs are selected in the ascending order of
        edge weights, which are specified by the argument or the
        universe (1.0 if not specified).

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
            yield GraphSet._conv_ret(g)

    def max_iter(self, weights=None):
        """Iterates over graphs in the descending order of weights.

        Returns a generator that iterates over graphs in `self`
        GraphSet.  The graphs are selected in the descending order of
        edge weights, which are specified by the argument or the
        universe (1.0 if not specified).

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
            yield GraphSet._conv_ret(g)

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
        raise TypeError, obj

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
            raise TypeError, graph_or_edge

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
            raise TypeError, obj
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
            raise TypeError, obj
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
            raise TypeError, edge

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
          larger(), len()
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
          smaller(), len()
        """
        return GraphSet(self._ss.larger(size))

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

#    def join(self, other):
#        """Returns a new GraphSet of join between `self` and `other`.
#
#        The join operation is defined by,
#          gs1 \\sqcup gs2 = {a \\cup b | a \\in gs1 and b \\in gs2}.
#        D. Knuth, Exercise 203, The art of computer programming,
#        Sect.7.1.4.
#
#        The `self` is not changed.
#
#        Examples:
#          >>> graph1 = [(1, 2)]
#          >>> graph2 = [(1, 2), (1, 4)]
#          >>> graph3 = [(2, 3)]
#          >>> gs1 = GraphSet([graph1, graph2])
#          >>> gs2 = GraphSet([graph3])
#          >>> gs1.join(gs2)
#          GraphSet([[(1, 2), (2, 3)], [(1, 2), (1, 4), (2, 3)]])
#
#        Returns:
#          A new GraphSet object.
#
#        See Also:
#          meet()
#        """
#        return GraphSet(self._ss.join(other._ss))

#    def meet(self, other):
#        """Returns a new GraphSet of meet between `self` and `other`.
#
#        The meet operation is defined by,
#          gs1 \\sqcap gs2 = {a \\cap b | a \\in gs1 and b \\in gs2}.
#        D. Knuth, Exercise 203, The art of computer programming,
#        Sect.7.1.4.
#
#        The `self` is not changed.
#
#        Examples:
#          >>> graph1 = [(1, 2), (1, 4)]
#          >>> graph2 = [(1, 2), (2, 3)]
#          >>> graph3 = [(1, 4), (2, 3)]
#          >>> gs1 = GraphSet([graph1, graph2])
#          >>> gs2 = GraphSet([graph3])
#          >>> gs1.meet(gs2)
#          GraphSet([[(1, 4)], [(2, 3)]])
#
#        Returns:
#          A new GraphSet object.
#
#        See Also:
#          join()
#        """
#        return GraphSet(self._ss.meet(other._ss))

#    def subgraphs(self, other):
#        """Returns a new GraphSet with subgraphs of a graph in `other`.
#
#        The `self` is not changed.
#
#        Examples:
#          >>> graph1 = [(1, 2)]
#          >>> graph2 = [(1, 2), (1, 4)]
#          >>> graph3 = graph1 + [(2, 3)]
#          >>> graph4 = [(1, 4), (2, 3)]
#          >>> gs1 = GraphSet([graph1, graph2])
#          >>> gs2 = GraphSet([graph3, graph4])
#          >>> gs1.subgraphs(gs2)
#          GraphSet([[(1, 2)]])
#
#        Returns:
#          A new GraphSet object.
#
#        See Also:
#          supersets(), non_subsets()
#        """
#        return GraphSet(self._ss.subsets(other._ss))

#    def supergraphs(self, other):
#        """Returns a new GraphSet with supergraphs of a graph in `other`.
#
#        The `self` is not changed.
#
#        Examples:
#          >>> graph1 = [(1, 2), (2, 3)]
#          >>> graph2 = [(1, 4), (2, 3)]
#          >>> graph3 = [(1, 2)]          # graph1 - (2, 3)
#          >>> graph4 = [(1, 2), (1, 4)]
#          >>> gs1 = GraphSet([graph1, graph2])
#          >>> gs2 = GraphSet([graph3, graph4])
#          >>> gs1.supergraphs(gs2)
#          GraphSet([[(1, 2), (2, 3)]])
#
#        Returns:
#          A new GraphSet object.
#
#        See Also:
#          subsets(), non_supersets()
#        """
#        return GraphSet(self._ss.supersets(other._ss))

#    def non_subgraphs(self, other):
#        """Returns a new GraphSet with graphs that aren't subgraphs of any graph in `other`.
#
#        The `self` is not changed.
#
#        The non_subsets are defined by,
#          gs1.non_subsets(gs2) = {a \\in gs1 | b \\in gs2 -> a \\not\\subseteq b}.
#        D. Knuth, Exercise 236, The art of computer programming,
#        Sect.7.1.4.
#
#        Examples:
#          >>> graph1 = [(1, 2)]
#          >>> graph2 = [(1, 2), (1, 4)]
#          >>> graph3 = [(1, 2), (2, 3)]
#          >>> graph4 = [(1, 4), (2, 3)]
#          >>> gs1 = GraphSet([graph1, graph2])
#          >>> gs2 = GraphSet([graph3, graph4])
#          >>> gs1.non_subgraphs(gs2)
#          GraphSet([[(1, 2), (1, 4)]])
#
#        Returns:
#          A new GraphSet object.
#
#        See Also:
#          non_supersets(), subsets()
#        """
#        return GraphSet(self._ss.non_subsets(other._ss))

#    def non_supergraphs(self, other):
#        """Returns a new GraphSet with graphs that aren't supergraphs of any graph in `other`.
#
#        The `self` is not changed.
#
#        The non_supersets are defined by,
#          gs1.non_supersets(gs2) = {a \\in gs1 | b \\in gs2 -> a \\not\\superseteq b}.
#        D. Knuth, Exercise 236, The art of computer programming,
#        Sect.7.1.4.
#
#        Examples:
#          >>> graph1 = [(1, 2), (2, 3)]
#          >>> graph2 = [(1, 4), (2, 3)]
#          >>> graph3 = [(1, 2)]
#          >>> graph4 = [(1, 2), (1, 4)]
#          >>> gs1 = GraphSet([graph1, graph2])
#          >>> gs2 = GraphSet([graph3, graph4])
#          >>> gs1.non_supergraphs(gs2)
#          GraphSet([[(1, 4), (2, 3)]])
#
#        Returns:
#          A new GraphSet object.
#
#        See Also:
#          non_subsets(), supersets()
#        """
#        return GraphSet(self._ss.non_supersets(other._ss))

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
            raise TypeError, obj

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
          >>> GraphSet.set_universe(pickle.load(fp))
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
          >>> GraphSet.set_universe(pickle.loads(universe_str))
          >>> gs = GraphSet.load(graphset_str)

        See Also:
          load(), dumps()
        """
        return GraphSet(setset.loads(s))

    @staticmethod
    def set_universe(universe, traversal='bfs', source=None):
        """Registers the new universe.

        Examples:
          >>> GraphSet.set_universe([(1, 2, 2.0), (1, 4, -3.0), (2, 3)])

        Args:
          universe: A list of edges that represents the new universe.
            An edge may come along with an edge weight, which can be
            positive as well as negative (or 1.0 if not specified).

          traversal: Optional.  This argument specifies the order of
            edges to be processed in the internal graphset operations.
            The default is 'bfs', the breadth-first search from
            `source`.  Other options include 'dfs', the depth-first
            search, and 'as-is', the order of `universe` list.

          source: Optional.  This argument specifies the starting
            point of the edge traversal.
        """
        edges = []
        GraphSet._vertices = set()
        GraphSet._weights = {}
        universe = GraphSet.converters['to_edges'](universe)
        for e in universe:
            if e[:2] in edges or (e[1], e[0]) in edges:
                raise KeyError, e
            edges.append(e[:2])
            if len(e) > 2:
                GraphSet._weights[e[:2]] = e[2]
        if traversal == 'bfs' or traversal == 'dfs':
            if not source:
                source = edges[0][0]
                for e in edges:
                    source = min(e[0], e[1], source)
            edges = GraphSet._traverse(edges, traversal, source)
        for u, v in edges:
            GraphSet._vertices.add(u)
            GraphSet._vertices.add(v)
        setset.set_universe(edges)

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
               no_loop=False, graphset=None):
        """Returns a GraphSet with graphs under given constraints.

        Examples: a set of paths from vertex 1 to vertex 6
          >>> start = 1
          >>> end = 6
          >>> zero_or_two = xrange(0, 3, 2)
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

        Returns:
          A new GraphSet object.
        """
        graph = []
        for e in setset.universe():
            assert e[0] in GraphSet._vertices and e[1] in GraphSet._vertices
            graph.append((pickle.dumps(e[0]), pickle.dumps(e[1])))

        vg = []
        nc = 0
        if vertex_groups is not None:
            for vs in vertex_groups:
                if len(vs) == 0:
                    nc += 1
                else:
                    for v in vs:
                        if v not in GraphSet._vertices:
                            raise KeyError, v
                    vg.append([pickle.dumps(v) for v in vs])
        if not vg and nc == 0:
            nc = -1

        dc = None
        if degree_constraints is not None:
            dc = {}
            for v, r in degree_constraints.iteritems():
                if v not in GraphSet._vertices:
                    raise KeyError, v
                if isinstance(r, (int, long)):
                    dc[pickle.dumps(v)] = (r, r + 1, 1)
                elif len(r) == 1:
                    dc[pickle.dumps(v)] = (r[0], r[0] + 1, 1)
                else:
                    dc[pickle.dumps(v)] = (r[0], r[-1] + 1, r[1] - r[0])

        ne = None
        if num_edges is not None:
            if isinstance(num_edges, (int, long)):
                ne = (num_edges, num_edges + 1, 1)
            elif len(num_edges) == 1:
                ne = (num_edges[0], num_edges[0] + 1, 1)
            else:
                ne = (num_edges[0], num_edges[-1] + 1,
                      num_edges[1] - num_edges[0])

        ss = None if graphset is None else graphset._ss

        ss = _graphillion._graphs(graph=graph, vertex_groups=vg,
                                  degree_constraints=dc, num_edges=ne,
                                  num_comps=nc, no_loop=no_loop,
                                  search_space=ss)
        return GraphSet(ss)

    @staticmethod
    def connected_components(vertices, graphset=None):
        """Returns a GraphSet of connected components.

        Examples:
          >>> GraphSet.connected_components([1, 3, 5])
          GraphSet([[(1, 2), (2, 3), (2, 5)], [(1, 2), (1, 4), (2, 3), (2, 5)], [(1, 2 ...

        Args:
          vertices: A list of vertices to be connected.

          graphset: Optional.  A GraphSet object.  Components to be
            stored are selected from this object.

        Returns:
          A new GraphSet object.
        """
        return GraphSet.graphs(vertex_groups=[vertices], graphset=graphset)

    @staticmethod
    def cliques(k, graphset=None):
        """Returns a GraphSet of k-cliques.

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
        """
        dc = {}
        for v in GraphSet._vertices:
            dc[v] = xrange(0, k, k - 1)
        ne = range(k * (k - 1) / 2, k * (k - 1) / 2 + 1)
        return GraphSet.graphs(vertex_groups=[[]], degree_constraints=dc,
                               num_edges=ne, graphset=graphset)

    @staticmethod
    def trees(root=None, is_spanning=False, graphset=None):
        """Returns a GraphSet of trees.

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
        """
        vg = [[]] if root is None else [[root]]
        dc = None
        if is_spanning:
            dc = {}
            for v in GraphSet._vertices:
                dc[v] = xrange(1, len(GraphSet._vertices))
        return GraphSet.graphs(vertex_groups=vg, degree_constraints=dc,
                               no_loop=True, graphset=graphset)

    @staticmethod
    def forests(roots, is_spanning=False, graphset=None):
        """Returns a GraphSet of forests, sets of trees.

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
        """
        vg = [[r] for r in roots]
        dc = None
        if is_spanning:
            dc = {}
            for v in GraphSet._vertices:
                if v not in roots:
                    dc[v] = xrange(1, len(GraphSet._vertices))
        return GraphSet.graphs(vertex_groups=vg, degree_constraints=dc,
                               no_loop=True, graphset=graphset)

    @staticmethod
    def cycles(is_hamilton=False, graphset=None):
        """Returns a GraphSet of cycles.

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
        """
        dc = {}
        for v in GraphSet._vertices:
            dc[v] = 2 if is_hamilton else xrange(0, 3, 2)
        return GraphSet.graphs(vertex_groups=[[]], degree_constraints=dc,
                               graphset=graphset)

    @staticmethod
    def paths(terminal1, terminal2, is_hamilton=False, graphset=None):
        """Returns a GraphSet of paths.

        Examples:
          >>> GraphSet.paths(1, 6)
          GraphSet([[(1, 2), (2, 3), (3, 6)], [(1, 2), (2, 5), (5, 6)], [(1, 4), (4, 5 ...

        Args:
          terminal1 and terminal2: Both end vertices of a paths.

          graphset: Optional.  A GraphSet object.  Paths to be stored
            are selected from this object.

        Returns:
          A new GraphSet object.
        """
        dc = {}
        for v in GraphSet._vertices:
            if v in (terminal1, terminal2):
                dc[v] = 1
            else:
                dc[v] = 2 if is_hamilton else xrange(0, 3, 2)
        return GraphSet.graphs(vertex_groups=[[terminal1, terminal2]],
                               degree_constraints=dc,
                               no_loop=True, graphset=graphset)

    @staticmethod
    def _traverse(edges, traversal, source):
        neighbors = {}
        for u, v in edges:
            if u not in neighbors:
                neighbors[u] = set([v])
            else:
                neighbors[u].add(v)
            if v not in neighbors:
                neighbors[v] = set([u])
            else:
                neighbors[v].add(u)
        assert source in neighbors

        sorted_edges = []
        queue_or_stack = []
        visited_vertices = set()
        u = source
        while True:
            if u in visited_vertices:
                continue
            visited_vertices.add(u)
            for v in sorted(neighbors[u]):
                if v in visited_vertices:
                    e = (u, v) if (u, v) in edges else (v, u)
                    sorted_edges.append(e)
            new_vertices = neighbors[u] - visited_vertices - set(queue_or_stack)
            queue_or_stack.extend(new_vertices)
            if not queue_or_stack:
                break
            if traversal == 'bfs':
                u, queue_or_stack = queue_or_stack[0], queue_or_stack[1:]
            else:
                queue_or_stack, u = queue_or_stack[:-1], queue_or_stack[-1]
        assert set(edges) == set(sorted_edges)
        return sorted_edges

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
            raise KeyError, obj

    @staticmethod
    def _conv_graph(obj):
        return GraphSet.converters['to_edges'](obj)

    @staticmethod
    def _conv_edge(edge):
        if not isinstance(edge, tuple) or len(edge) < 2:
            raise KeyError, edge
        if len(edge) > 2:
            edge = edge[:2]
        if edge in setset._obj2int:
            return edge
        elif (edge[1], edge[0]) in setset._obj2int:
            return (edge[1], edge[0])
        raise KeyError, edge

    @staticmethod
    def _conv_ret(obj):
        if isinstance(obj, (set, frozenset)):  # a graph
            return GraphSet.converters['to_graph'](sorted(list(obj)))
        raise TypeError, obj

    converters = { 'to_graph': lambda edges: edges,
                   'to_edges': lambda graph: graph }

    _vertices = set()
    _weights = {}
