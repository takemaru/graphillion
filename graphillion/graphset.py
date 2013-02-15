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

from graphillion import setset


class GraphSet(object):
    """Represents and manipulates a set of graphs.

    A GraphSet object stores a set of graphs.  A graph stored is a
    subgraph of the universal graph, and it is represented by a set of
    edges in the universal graph.  An edge is a tuple of two vertices,
    and a vertex can be any hashable object like a number, a text
    string, and a tuple.  Currently, GraphSet only supports undirected
    graphs without edge labels.

    The universal graph must be defined before creating GraphSet
    objects by `GraphSet.universe()` method.

    Like Python set types, GraphSet supports `graph in graphset`,
    `len(graphset)`, and `for graph in graphset`.  It also supports
    all set methods and operators,
    - isdisjoint(), issubset(), issuperset(), union(), intersection(),
      difference(), symmetric_difference(), copy(), update(),
      intersection_update(), difference_update(),
      symmetric_difference_update(), add(), remove(), discard(),
      pop(), clear(),
    - ==, !=, <=, <, >=, >, |, &, -, ^, |=, &=, -=, ^=.

    Examples:
      >>> from graphillion import GraphSet

      We assume the following graph and register the edge list as the
      universe.

      1 --- 2 --- 3
      |     |     |
      4 --- 5 --- 6

      >>> GraphSet.set_universe([(1,2), (1,4), (2,3), (2,5), (3,6), (4,5), (5,6)])

      Find all paths from 1 to 6 and count them.

      >>> paths = GraphSet.path(1, 6)
      >>> len(paths)
      3

      Give constraints in which edge 1-4 must not be passed but 2 must
      be passed, and show the paths that meet the constraints.

      >>> paths = paths.exclude((1,4)).include(2)
      >>> for path in paths:
      ...   path
      set([(1, 2), (2, 3), (3, 6)])
      set([(1, 2), (2, 5), (5, 6)])
    """

    def __init__(self, graphset_or_constraints=None):
        """Initializes a GraphSet object with a set of graphs or constraints.

        Examples:
          >>> gs = GraphSet([set([]), set([(1,2), (2,3)])])
          >>> gs
          GraphSet([set([]), set([(1,2), (2,3)])])
          >>> gs = GraphSet({'include': [(1,2), (2,3)], 'exclude': [(1,4)]})
          >>> gs
          GraphSet([set([(1,2), (2,3)]), set([(1,2), (2,3), (2,5)]), ...

        Args:
          graphset_or_constraints: A set of graphs represented by a
            list of edge sets:

            [set([]), set([(1,2), (2,3)])]

            Or constraints represented by a dict of included or
            excluded edge lists (not-specified edges are not cared):

            {'include': [(1,2), (2,3)], 'exclude': [(1,4)]}

            If no argument is given, it is treated as an empty list
            `[]` and an empty GraphSet is returned.  An empty dict
            `{}` means that no constraint is specified, and so a
            GraphSet including all possible graphs in the universe is
            returned (let N the number of edges in the universe, 2^N
            graphs are stored).

        Raises:
          KeyError: If given edges are not found in the universe.

        See Also:
          copy()
        """
        obj = graphset_or_constraints
        if isinstance(obj, GraphSet):
            self.ss = obj.ss.copy()
        elif isinstance(obj, setset):
            self.ss = obj.copy()
        else:
            self.ss = setset(GraphSet._conv_arg(obj))

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
        return bool(self.ss)

    def __repr__(self):
        return repr(self.ss)

    def union(self, other):
        """Returns a new GraphSet with graphs from `self` and all others.

        The `self` GraphSet is not changed.

        Examples:
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])
          >>> gs2 = GraphSet([set([]), set([(1,2)])])
          >>> gs = gs1 | gs2
          >>> gs
          GraphSet([set([]), set([(1, 2)]), set([(1, 2), (1, 4)])])

        Returns:
          A new GraphSet object.

        See Also:
          intersection(), difference(), symmetric_difference(),
          update()
        """
        return GraphSet(self.ss.union(other.ss))

    def intersection(self, other):
        """Returns a new GraphSet with graphs common to `self` and all others.

        The `self` GraphSet is not changed.

        Examples:
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])
          >>> gs2 = GraphSet([set([]), set([(1,2)])])
          >>> gs = gs1 & gs2
          >>> s
          GraphSet([set([(1, 2)])])

        Returns:
          A new GraphSet object.

        See Also:
          union(), difference(), symmetric_difference(),
          intersection_update()
        """
        return GraphSet(self.ss.intersection(other.ss))

    def difference(self, other):
        """Returns a new GraphSet with graphs in `self` that are not in the others.

        The `self` GraphSet is not changed.

        Examples:
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])
          >>> gs2 = GraphSet([set([]), set([(1,2)])])
          >>> gs = gs1 - gs2
          >>> gs
          GraphSet([set([(1, 2), (1, 4)])])

        Returns:
          A new GraphSet object.

        See Also:
          union(), intersection(), symmetric_difference(),
          difference_update()
        """
        return GraphSet(self.ss.difference(other.ss))

    def symmetric_difference(self, other):
        """Returns a new GraphSet with graphs in either `self` or `other` but not both.\n\

        The `self` GraphSet is not changed.

        Examples:\n\
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> gs2 = GraphSet([set([]), set([(1,2)])])\n\
          >>> gs = gs1 ^ gs2\n\
          >>> gs\n\
          GraphSet([set([]), set([(1, 2), (1, 4)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          union(), intersection(), difference(), 
          symmetric_difference_update()
        """
        return GraphSet(self.ss.symmetric_difference(other.ss))

    def quotient(self, other):
        """Returns a new GraphSet of quotient.\n\

        The quotient is defined by,\n\
          gs1 / gs2 = {a | a \\cup b \\in gs1 and a \\cap b = \\empty, \\forall b \\in gs2}.\n\
        D. Knuth, Exercise 204, The art of computer programming, Sect.7.1.4.\n\

        The `self` GraphSet is not changed.

        Examples:\n\
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3), (2,5)])])\n\
          >>> gs = gs / GraphSet([set([(1,4)])])\n\
          >>> gs\n\
          GraphSet([set([(1, 2)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          remainder(), quotient_update()
        """
        return GraphSet(self.ss.quotient(other.ss))

    def remainder(self, other):
        """Returns a new GraphSet of remainder.\n\

        The remainder is defined by,\n\
          gs1 % gs2 = gs1 - (gs1 \\sqcup (gs1 / gs2)).\n\
        D. Knuth, Exercise 204, The art of computer programming, Sect.7.1.4.\n\

        The `self` GraphSet is not changed.

        Examples:\n\
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3), (2,5)])])\n\
          >>> gs = gs % GraphSet([set([(1,4)])])\n\
          >>> gs\n\
          GraphSet([set([(2,3), (2,5)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          quotient(), remainder_update()
        """
        return GraphSet(self.ss.remainder(other.ss))

    def update(self, other):
        """Updates `self`, adding graphs from all others.

        Examples:
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])
          >>> gs2 = GraphSet([set([]), set([(1,2)])])
          >>> gs1 |= gs2
          >>> gs1
          GraphSet([set([]), set([(1, 2)]), set([(1, 2), (1, 4)])])

        Returns:
          A new GraphSet object.

        See Also:
          union()
        """
        self.ss.update(other.ss)
        return self

    def intersection_update(self, other):
        """Updates `self`, keeping only graphs found in it and all others.

        Examples:
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])
          >>> gs2 = GraphSet([set([]), set([(1,2)])])
          >>> gs1 &= gs2
          >>> gs1
          GraphSet([set([(1, 2)])])

        Returns:
          A new GraphSet object.

        See Also:
          intersection()
        """
        self.ss.intersection_update(other.ss)
        return self

    def difference_update(self, other):
        """Update `self`, removing graphs found in others.\n\

        Examples:\n\
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> gs2 = GraphSet([set([]), set([(1,2)])])\n\
          >>> gs1 -= gs2\n\
          >>> gs1\n\
          GraphSet([set([(1, 2), (1, 4)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          difference()
        """
        self.ss.difference_update(other.ss)
        return self

    def symmetric_difference_update(self, other):
        """Update `self`, keeping only graphs in either GraphSet, but not in both.\n\

        Examples:\n\
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> gs2 = GraphSet([set([]), set([(1,2)])])\n\
          >>> gs1 ^= gs2\n\
          >>> gs1\n\
          GraphSet([set([]), set([(1, 2), (1, 4)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          symmetric_difference()
        """
        self.ss.symmetric_difference_update(other.ss)
        return self

    def quotient_update(self, other):
        """Updates `self` by the quotient.\n\

        Examples:\n\
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3), (2,5)])])\n\
          >>> gs /= GraphSet([set([(1,4)])])\n\
          >>> gs\n\
          GraphSet([set([(1, 2)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          quotient()
        """
        self.ss.quotient_update(other.ss)
        return self

    def remainder_update(self, other):
        """Updates `self` by the remainder.\n\

        Examples:\n\
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3), (2,5)])])\n\
          >>> gs %= GraphSet([set([(1,4)])])\n\
          >>> gs\n\
          GraphSet([set([(2,3), (2,5)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          remainder()
        """
        self.ss.remainder_update(other.ss)
        return self

    def __invert__(self):
        """Returns a new GraphSet with graphs not stored in `self`.

        Examples:
          >>> GraphSet.set_universe([(1,2), (1,4)])
          >>> gs = GraphSet([set([(1,2)])])
          >>> gs = ~gs
          >>> gs
          setset([set([]), set([(1, 4)]), set([(1, 2), (1, 4)])])

        Returns:
          A new GraphSet object.
        """
        return GraphSet(~self.ss)

    __or__ = union
    __and__ = intersection
    __sub__ = difference
    __xor__ = symmetric_difference
    __div__ = quotient
    __mod__ = remainder

    __ior__ = update
    __iand__ = intersection_update
    __isub__ = difference_update
    __ixor__ = symmetric_difference_update
    __idiv__ = quotient_update
    __imod__ = remainder_update

    def isdisjoint(self, other):
        """Returns True if `self` has no graphs in common with `other`.\n\

        Examples:\n\
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> gs2 = GraphSet([set([]), set([(1,4)])])\n\
          >>> gs1.disjoint(gs2)\n\
          True\n\

        Returns:\n\
          True or False.\n\

        See Also:\n\
          issubset(), issuperset()
        """
        return self.ss.isdisjoint(other.ss)

    def issubset(self, other):
        """Tests if every graph in `self` is in `other`.\n\

        Examples:\n\
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> gs2 = GraphSet([set([(1,2)]), set([(1,2), (1,4)]), set([(1,4)])])\n\
          >>> gs1 <= gs2\n\
          True\n\

        Returns:\n\
          True or False.\n\

        See Also:\n\
          issuperset(), isdisjoint()
        """
        return self.issubset(other.ss)

    def issuperset(self, other):
        """Tests if every graph in `other` is in `self`.\n\

        Examples:\n\
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)]), set([(1,4)])])\n\
          >>> gs2 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> gs1 >= gs2\n\
          True\n\

        Returns:\n\
          True or False.\n\

        See Also:\n\
          issubset(), isdisjoint()
        """
        return self.issuperset(other.ss)

    __le__ = issubset
    __ge__ = issuperset

    def __lt__(self, other):
        """Tests if `self` is a true subset of `other`.

        This method returns False when `self` == `other`, unlike
        issubset.

        Examples:\n\
          >>> gs < gs\n\
          False\n\

        Returns:\n\
          True or False.\n\

        See Also:\n\
          issubset(), issuperset(), isdisjoint()
        """
        return self.ss < other.ss

    def __gt__(self, other):
        """Test if `self` is a true superset of `other`.

        This method returns False when `self` == `other`, unlike
        issuperset.

        Examples:\n\
          >>> gs > gs\n\
          False\n\

        Returns:\n\
          True or False.\n\

        See Also:\n\
          issubset(), isdisjoint()
        """
        return self.ss > other.ss

    def __eq__(self, other):
        return self.ss == other.ss

    def __ne__(self, other):
        return self.ss != other.ss

    def __len__(self):
        """Returns the number of graphs in `self`.\n\

        Use gs.len() if OverflowError raised.

        Examples:\n\
          >>> gs = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> len(gs)\n\
          2\n\

        Returns:\n\
          The number of graphs.\n\

        Raises:
          OverflowError

        See Also:\n\
          len()
        """
        return len(self.ss)

    def len(self):
        """Returns the number of graphs in `self`.\n\

        Examples:\n\
          >>> gs = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> gs.len()\n\
          2L\n\

        Returns:\n\
          The number of graphs.\n\

        See Also:\n\
          __len__()
        """
        return self.ss.len()

    def randomize(self):
        """Iterates over graphs randomly.

        Examples:
          >>> gs = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])
          >>> for g in gs:
          ...   g
          set([(1, 2), (1, 4)])
          set([(1, 2)])

        Returns:
          A generator.

        Yields:
          A graph.

        See Also:
          minimize(), maximize(), pop()
        """
        return self.ss.randomize()

    __iter__ = randomize

    def minimize(self):
        """Iterates over graphs in the ascending order of weights.

        Returns a generator that iterates over graphs in `self`
        GraphSet.  The graphs are selected in the ascending order of
        weights, which are specified with the universe (or 1.0 if not
        specified).

        Examples:
          >>> GraphSet.set_universe([(1,2, 2.0), (1,4, -3.0), (2,3)])
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> for g in gs.minimize():
          ...   g
          set([(1, 2), (1, 4)])
          set([(2, 3)])

        Returns:
          A generator.

        Yields:
          A graph.

        See Also:
          maximize(), randomize()
        """
        for g in self.ss.minimize(GraphSet._weights):
            yield g

    def maximize(self):
        """Iterates over graphs in the descending order of weights.

        Returns a generator that iterates over graphs in `self`
        GraphSet.  The graphs are selected in the descending order of
        weights, which are specified with the universe (or 1.0 if not
        specified).

        Examples:
          >>> GraphSet.set_universe([(1,2, 2.0), (1,4, -3.0), (2,3)])
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> for g in gs.maximize():
          ...   g
          set([(2, 3)])
          set([(1, 2), (1, 4)])

        Returns:
          A generator.

        Yields:
          A graph.

        See Also:
          minimize(), randomize()
        """
        for g in self.ss.maximize(GraphSet._weights):
            yield g

    def __contains__(self, graph):
        """Returns True if `graph` is in the `self` GraphSet, False otherwise.

        Use the expression `graph in gs`.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> set([(2,3)]) in gs
          True

        Args:
          graph: A graph (a set of edges) in the universe.

        Returns:
          True or False.

        Raises:
          KeyError: If the given graph is not found in the universe.
        """
        graph = GraphSet._conv_arg(graph)
        return graph in self.ss

    def include(self, edge_or_vertex):
        """Returns a new set of graphs that include a given edge or vertex.

        The graphs stored in the new GraphSet are selected from the
        `self` GraphSet.  The `self` GraphSet is not changed.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> gs = gs.include(4)
          >>> gs
          GraphSet([set([(1,2), (1,4)])])

        Args:
          edge_or_vertex: An edge or a vertex in the universe.

        Returns:
          A new GraphSet object.

        Raises:
          KeyError: If a given edge or vertex is not found in the universe.

        See Also:
          exclude()
        """
        try:  # if edge
            return self.ss.include(GraphSet._conv_edge(edge_or_vertex))
        except KeyError:  # else
            gs = GraphSet()
            edges = [e for e in setset.get_universe() if edge_or_vertex in e]
            for edge in edges:
                gs.ss |= self.ss.include(edge)
            return GraphSet(gs.ss & self.ss)

    def exclude(self, edge_or_vertex):
        """Returns a new set of graphs that don't include a given edge or vertex.

        The graphs stored in the new GraphSet are selected from `self`
        GraphSet.  The `self` GraphSet is not changed.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> gs = gs.exclude(4)
          >>> gs
          GraphSet([set([(2,3)])])

        Args:
          edge_or_vertex: An edge or a vertex in the universe.

        Returns:
          A new GraphSet object.

        Raises:
          KeyError: If a given edge or vertex is not found in the universe.

        See Also:
          include()
        """
        try:  # if edge
            return self.ss.exclude(GraphSet._conv_edge(edge_or_vertex))
        except KeyError:  # else
            return self - self.include(edge_or_vertex)

    def add(self, graph_or_edge):
        """Adds a given graph or edge to `self` GraphSet.

        If a graph is given, the graph is just added to `self`
        GraphSet.  If an edge is given, the edge is added to all the
        graphs in `self` GraphSet.

        The `self` GraphSet is changed.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> gs.add((1,2))
          >>> gs
          GraphSet([set([(1,2), (1,4)]), set([(1,2), (2,3)])])

        Args:
          graph_or_edge: A graph (a set of edges) or an edge in the universe.

        Returns:
          None.

        Raises:
          KeyError: If given edges are not found in the universe.

        See Also:
          remove(), discard(), flip()
        """
        graph_or_edge = GraphSet._conv_arg(graph_or_edge)
        return self.ss.add(graph_or_edge)

    def remove(self, graph_or_edge):
        """Removes a given graph or edge from `self` GraphSet.

        If a graph is given, the graph is just removed from `self`
        GraphSet.  If an edge is given, the edge is removed from all
        the graphs in `self` GraphSet.

        The `self` GraphSet is changed.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> gs.remove((1,2))
          >>> gs
          GraphSet([set([(1,4)]), set([(2,3)])])

        Args:
          graph_or_edge: A graph (a set of edges) or an edge in the universe.

        Returns:
          None.

        Raises:
          KeyError: If given edges are not found in the universe, or
            if the given graph is not stored in `self` GraphSet.

        See Also:
          add(), discard(), flip(), pop()
        """
        graph_or_edge = GraphSet._conv_arg(graph_or_edge)
        return self.ss.remove(graph_or_edge)

    def discard(self, graph_or_edge):
        """Removes a given graph or edge from `self` GraphSet.

        If a graph is given, the graph is just removed from `self`
        GraphSet.  If an edge is given, the edge is removed from all
        the graphs in `self` GraphSet.

        The `self` GraphSet is changed.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> gs.discard((1,2))
          >>> gs
          GraphSet([set([(1,4)]), set([(2,3)])])

        Args:
          graph_or_edge: A graph (a set of edges) or an edge in the universe.

        Returns:
          None.

        Raises:
          KeyError: If given edges are not found in the universe.

        See Also:
          add(), remove(), flip(), pop()
        """
        graph_or_edge = GraphSet._conv_arg(graph_or_edge)
        return self.ss.discard(graph_or_edge)

    def pop(self):
        """Removes and returns an arbitrary graph from `self` GraphSet.

        Examlpes:
          >>> gs = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])
          >>> gs.pop()
          set([(1, 2), (1, 4)])

        Returns:
          A graph.

        Raises:
          KeyError: If `self` GraphSet is empty.

        See Also:
          remove(), discard(), randomize()
        """
        return self.ss.pop()

    def clear(self):
        """Removes all graphs from `self`.\n\

        Examples:\n\
          >>> gs = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> gs.clear()\n\
          >>> gs\n\
          GraphSet([])
        """
        return self.ss.clear(self)

    def minimal(self):
        """Returns a new GraphSet of minimal edge sets.\n\

        The minimal sets are defined by,\n\
          gs.minimal() = {a \\in gs | b \\in gs and a \\subseteq -> a = b}.\n\
        D. Knuth, Exercise 236, The art of computer programming, Sect.7.1.4.\n\

        The `self` GraphSet is not changed.

        Examples:\n\
          >>> gs = GraphSet([set([(1,2)]), set([(1,2), (1,4)]), set([(1,4), (2,3)])])\n\
          >>> gs = gs.minimal()\n\
          >>> gs\n\
          GraphSet([set([(1, 2)]), set([(1, 4), (2, 3)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          maximal(), blocking()
        """
        return GraphSet(self.ss.minimal())

    def maximal(self):
        """Returns a new GraphSet of maximal edge sets.\n\

        The maximal sets are defined by,\n\
          gs.maximal() = {a \\in gs | b \\in gs and a \\superseteq -> a = b}.\n\
        D. Knuth, Exercise 236, The art of computer programming, Sect.7.1.4.\n\

        The `self` GraphSet is not changed.

        Examples:\n\
          >>> gs = GraphSet([set([(1,2)]), set([(1,2), (1,4)]), set([(1,4), (2,3)])])\n\
          >>> gs = gs.maximal()\n\
          >>> gs\n\
          GraphSet([set([(1, 2), (1, 4)]), set([(1, 4), (2, 3)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          minimal()
        """
        return GraphSet(self.ss.maximal())

    def blocking(self):
        """Returns a new GraphSet of all blocking sets.\n\

        A blocking set is often called a hitting set; all graphs in
        `self` contain at least one edge in the set.  This implies
        that all the graphs are destroyed by removing edges in the
        set.

        The blocking sets are defined by,\n\
          gs.blocking() = {a | b \\in gs -> a \\cap b \\neq \\empty}.\n\
        T. Toda, Hypergraph Dualization Algorithm Based on Binary Decision\n\
        Diagrams.\n\

        The `self` GraphSet is not changed.

        Examples:\n\
          >>> gs = GraphSet([set([(1,2)]), set([(1,2), (1,4)]), set([(1,4), (2,3)])])\n\
          >>> gs = gs.blocking().minimal()\n\
          >>> gs\n\
          GraphSet([set([(1, 2), (1, 4)]), set([(1, 4), (2, 3)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          minimal()
        """
        return GraphSet(self.ss.hitting())

    def smaller(self, size):
        """Returns a new GraphSet with graphs that have less than `size` edges.\n\

        The `self` GraphSet is not changed.

        Examples:\n\
          >>> gs = GraphSet([set([(1,2)]), set([(1,2), (1,4)]), set([(1,2), (1,4), (2,3)])])\n\
          >>> gs = gs.smaller(2)\n\
          >>> gs\n\
          GraphSet([set([(1, 2)])])\n\

        Args:
          size: The number of edges in a graph.

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          larger(), same_size()
        """
        return GraphSet(self.ss.smaller(size))

    def larger(self, size):
        """Returns a new GraphSet with graphs that have more than `size` edges.\n\

        The `self` GraphSet is not changed.

        Examples:\n\
          >>> gs = GraphSet([set([(1,2)]), set([(1,2), (1,4)]), set([(1,2), (1,4), (2,3)])])\n\
          >>> gs = gs.larger(2)\n\
          >>> gs\n\
          GraphSet([set([(1, 2), (1, 4), (2, 3)])])\n\

        Args:
          size: The number of edges in a graph.

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          smaller(), same_size()
        """
        return GraphSet(self.ss.larger(size))

    def same_size(self, size):
        """Returns a new GraphSet with graphs that have `size` edges.\n\

        The `self` GraphSet is not changed.

        Examples:\n\
          >>> gs = GraphSet([set([(1,2)]), set([(1,2), (1,4)]), set([(1,2), (1,4), (2,3)])])\n\
          >>> gs = gs.same_size(2)\n\
          >>> gs\n\
          GraphSet([set([(1, 2), (1, 4)])])\n\

        Args:
          size: The number of edges in a graph.

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          smaller(), larger()
        """
        return GraphSet(self.ss.same_size(size))

    def flip(self, edge):
        """Returns a new set of graphs by flipping the state of a given edge.

        If a graph in `self` GraphSet includes the given edge, the
        edge is removed from the graph.  If a graph in `self` GraphSet
        does not include the given edge, the edge is added to the
        graph.

        The `self` GraphSet is not changed.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> gs = gs.flip((1,2))
          >>> gs
          GraphSet([set([(1,4)]), set([(1,2), (2,3)])])

        Args:
          edge: An edge in the universe.

        Returns:
          A new GraphSet object.

        Raises:
          KeyError: If a given edge is not found in the universe.

        See Also:
          add(), remove(), discard()
        """
        edge = GraphSet._conv_edge(edge)
        return GraphSet(self.ss.flip(edge))

    def complement(self):
        """Returns a new GraphSet with complement graphs of `self`.

        The `self` GraphSet is not changed.

        Examples:
          >>> GraphSet.set_universe([(1,2), (1,4)])
          >>> gs = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])
          >>> gs = ~gs
          >>> gs
          setset([set([]), set([(1, 4)])])

        Returns:
          A new GraphSet object.
        """
        return GraphSet(self.ss.flip())

    def join(self, other):
        """Returns a new GraphSet of join between `self` and `other`.\n\

        The join operation is defined by,\n\
          gs1 \\sqcup gs2 = {a \\cup b | a \\in gs1 and b \\in gs2}.\n\
        D. Knuth, Exercise 203, The art of computer programming, Sect.7.1.4.\n\

        The `self` GraphSet is not changed.

        Examples:\n\
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> gs2 = GraphSet([set([(2,3)])])\n\
          >>> gs = gs1.join(gs2)\n\
          >>> gs\n\
          GraphSet([set([(1, 2), (2, 3)]), set([(1, 2), (1, 4), (2, 3)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          meet()
        """
        return GraphSet(self.ss.join(other.ss))

    def meet(self, other):
        """Returns a new GraphSet of meet between `self` and `other`.\n\

        The meet operation is defined by,\n\
          gs1 \\sqcap gs2 = {a \\cap b | a \\in gs1 and b \\in gs2}.\n\
        D. Knuth, Exercise 203, The art of computer programming, Sect.7.1.4.\n\

        The `self` GraphSet is not changed.

        Examples:\n\
          >>> gs1 = GraphSet([set([(1,2), (1,4)]), set([(1,2), (2,3)])])\n\
          >>> gs2 = GraphSet([set([(1,4), (2,3)])])\n\
          >>> gs = gs1.meet(gs2)\n\
          >>> gs\n\
          GraphSet([set([(1, 4)]), set([(2, 3)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          join()
        """
        return GraphSet(self.ss.meet(other.ss))

    def subgraphs(self, other):
        """Returns a new GraphSet with subgraphs of a graph in `other`.\n\

        The `self` GraphSet is not changed.

        Examples:\n\
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> gs2 = GraphSet([set([(1,2), (2,3)]), set([(1,4), (2,3)])])\n\
          >>> gs = gs1.subgraphs(gs2)\n\
          >>> gs\n\
          GraphSet([set([(1, 2)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          supersets(), non_subsets()
        """
        return GraphSet(self.ss.subsets(other.ss))

    def supergraphs(self, other):
        """Returns a new GraphSet with supergraphs of a graph in `other`.\n\

        The `self` GraphSet is not changed.

        Examples:\n\
          >>> gs1 = GraphSet([set([(1,2), (2,3)]), set([(1,4), (2,3)])])\n\
          >>> gs2 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> gs = gs1.supergraphs(gs2)\n\
          >>> gs\n\
          GraphSet([set([(1, 2), (2, 3)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          subsets(), non_supersets()
        """
        return GraphSet(self.ss.supersets(other.ss))

    def non_subgraphs(self, other):
        """Returns a new GraphSet with graphs that aren't subgraphs of any graph in `other`.\n\

        The `self` GraphSet is not changed.

        The non_subsets are defined by,\n\
          gs1.non_subsets(gs2) = {a \\in gs1 | b \\in gs2 -> a \\not\\subseteq b}.\n\
        D. Knuth, Exercise 236, The art of computer programming, Sect.7.1.4.\n\

        Examples:\n\
          >>> gs1 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> gs2 = GraphSet([set([(1,2), (2,3)]), set([(1,4), (2,3)])])\n\
          >>> gs = gs1.non_subgraphs(gs2)\n\
          >>> gs\n\
          GraphSet([set([(1, 2), (1, 4)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          non_supersets(), subsets()
        """
        return GraphSet(self.ss.non_subsets(other.ss))

    def non_supergraphs(self, other):
        """Returns a new GraphSet with graphs that aren't supergraphs of any graph in `other`.\n\

        The `self` GraphSet is not changed.

        The non_supersets are defined by,\n\
          gs1.non_supersets(gs2) = {a \\in gs1 | b \\in gs2 -> a \\not\\superseteq b}.\n\
        D. Knuth, Exercise 236, The art of computer programming, Sect.7.1.4.\n\

        Examples:\n\
          >>> gs1 = GraphSet([set([(1,2), (2,3)]), set([(1,4), (2,3)])])\n\
          >>> gs2 = GraphSet([set([(1,2)]), set([(1,2), (1,4)])])\n\
          >>> gs = gs1.non_supergraphs(gs2)\n\
          >>> gs\n\
          GraphSet([set([(1, 4), (2, 3)])])\n\

        Returns:\n\
          A new GraphSet object.\n\

        See Also:\n\
          non_subsets(), supersets()
        """
        return GraphSet(self.ss.non_supersets(other.ss))

    def dump(self, fp):
        """Serialize `self` to a file `fp`.\n\

        This method does not serialize the universe, which should be saved\n\
        separately by pickle.\n\

        Examples:\n\
          >>> import pickle\n\
          >>> fp = open('/path/to/graphset', 'wb')\n\
          >>> gs.dump(fp)\n\
          >>> fp = open('/path/to/universe' 'wb')\n\
          >>> pickle.dump(GraphSet.get_universe(), fp)\n\

        Args:\n\
          fp: A write-supporting file-like object.\n\

        See Also:\n\
          dumps(), load()
        """
        return self.ss.dump(fp)

    def dumps(self):
        """Returns a serialized `self`.\n\

        This method does not serialize the universe, which should be saved\n\
        separately by pickle.\n\

        Examples:\n\
          >>> import pickle\n\
          >>> graphset_str = gs.dumps()\n\
          >>> universe_str = pickle.dumps(GraphSet.get_universe())\n\

        See Also:\n\
          dump(), loads()
        """
        return self.ss.dumps()

    def load(self, fp):
        """Deserialize a file `fp` to `self`.\n\

        This method does not deserialize the universe, which should be\n\
        loaded separately by pickle.\n\

        Args:\n\
          fp: A read-supporting file-like object.\n\

        Examples:\n\
          >>> import pickle\n\
          >>> fp = open('/path/to/universe')\n\
          >>> GraphSet.set_universe(pickle.load(fp))\n\
          >>> fp = open('/path/to/graphset')\n\
          >>> gs = GraphSet().load(fp)\n\

        See Also:\n\
          loads(), dump()
        """
        return self.ss.load(fp)

    def loads(self, s):
        """Deserialize `s` to `self`.\n\

        This method does not deserialize the universe, which should be\n\
        loaded separately by pickle.\n\

        Args:\n\
          s: A string instance.\n\

        Examples:\n\
          >>> import pickle\n\
          >>> GraphSet.set_universe(pickle.loads(universe_str))\n\
          >>> gs = Graphset().load(graphset_str)\n\

        See Also:\n\
          load(), dumps()
        """
        return self.ss.loads(s)

    @staticmethod
    def set_universe(universe, traversal='bfs', source=None):
        """Registers the new universe.

        Examples:
          >>> GraphSet.set_universe([(1,2, 2.0), (1,4, -3.0), (2,3)])

        Args:
          universe: A list of edges that represents the new universe.
            An edge may come along with a weight, which can be
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
        GraphSet._weights = {}
        for e in universe:
            edges.append(e[:2])
            if len(e) > 2:
                GraphSet._weights[e[:2]] = e[2]
        if traversal == 'bfs' or traversal == 'dfs':
            if not source:
                source = edges[0][0]
                for e in edges:
                    source = min(e[0], e[1], source)
            edges = GraphSet._traverse(edges, traversal, source)
        setset.set_universe(edges)

    @staticmethod
    def get_universe():
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
        for e in setset.get_universe():
            if e in GraphSet._weights:
                edges.append((e[0], e[1], GraphSet._weights[e]))
            else:
                edges.append(e)
        return edges

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
        if isinstance(obj, list):  # a set of graphs [set+]
            l = []
            for s in obj:
                l.append([GraphSet._conv_edge(e) for e in s])
            return l
        elif obj is None:  # an empty set of graphs
            return []
        elif isinstance(obj, (set, frozenset)):  # a graph
            return set([GraphSet._conv_edge(e) for e in obj])
        elif isinstance(obj, dict):  # constraints
            d = {}
            for k, l in obj.iteritems():
                d[k] = [GraphSet._conv_edge(e) for e in l]
            return d
        else:  # an edge
            return GraphSet._conv_edge(obj)

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

    _weights = {}
