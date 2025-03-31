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

"""Module for a set of sets containing edges and vertices.
"""

from builtins import range, int
import _graphillion
from graphillion.universe import ObjectTable
from graphillion.universe import Universe
from graphillion.setset_base import setset_base


class EdgeVertexSetSet(object):
    """Represents and manipulates a set of sets containing edges and vertices.

    A EdgeVertexSetSet object stores a set of sets containing edges and vertices.
    A graph stored must be a subgraph of the universal graph, and is represented
    by a list of edges and vertices in the universal graph. If an edge is
    contained in a list, both the endpoints of the edge are also contained in the
    list. An edge is a tuple of two vertices, and a vertex can be any hashable
    object like a number, a text string, and a tuple.  Currently,
    EdgeVertexSetSet only supports undirected graphs without edge labels.

    The universal graph must be defined before creating EdgeVertexSetSet
    objects by `Universe.set_universe()` method.

    Like Python set types, EdgeVertexSetSet supports `graph in graphset`,
    `len(graphset)`, and `for graph in graphset`.  It also supports
    all set methods and operators,
    * isdisjoint(), issubset(), issuperset(), union(), intersection(),
      difference(), symmetric_difference(), copy(), update(),
      intersection_update(), difference_update(),
      symmetric_difference_update(), add(), remove(), discard(),
      pop(), clear(),
    * ==, !=, <=, <, >=, >, |, &, -, ^, |=, &=, -=, ^=.

    Examples:
      >>> from graphillion import GraphSet, EdgeVertexSetSet, Universe

      We assume the following graph and register the edge list as the
      universe.

      1 --- 2 --- 3
      |     |     |
      4 --- 5 --- 6

      >>> universe = [(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5), (5, 6)]
      >>> Universe.set_universe(universe)

      Find all paths from 1 to 6.

      >>> paths = GraphSet.paths(1, 6)

      Convert GraphSet to EdgeVertexSetSet.

      >>> paths = paths.to_edgevertexsetset()
      >>> [(1, 4), (4, 5), (5, 6), 1, 4, 5, 6]
      True
      >>> len(paths)
      4

      Give constraints in which edge 1-4 must not be passed but 2 must
      be passed, and show the paths that meet the constraints.

      >>> paths = paths.excluding((1, 4)).including(2)
      >>> for path in paths:
      ...   path
      [(1, 2), (2, 5), (5, 6), 1, 2, 5, 6]
      [(1, 2), (2, 3), (3, 6), 1, 2, 3, 6]
    """

    def __init__(self, graphset_or_constraints=None):
        """Initializes an EdgeVertexSetSet object with a set of graphs or constraints.

        Examples:
          >>> graph1 = [(1, 4), 1, 4]
          >>> graph2 = [(1, 2), (2, 3), 1, 2, 3]
          >>> EdgeVertexSetSet([graph1, graph2])
          EdgeVertexSetSet([[(1, 4), 1, 4], [(1, 2), (2, 3), 1, 2, 3]])

        Args:
          graphset_or_constraints: A set of graphs represented by a
            list of lists of edges and vertices:

            [[(1, 4), 1, 4], [(1, 2), (2, 3), 1, 2, 3]]

            If no argument is given, it is treated as an empty list
            `[]` and an empty EdgeVertexSetSet is returned.  An empty dict
            `{}` means that an EdgeVertexSetSet including all possible
            graphs in the universe is returned (let m be the number of edges
            in the universe. 2^m graphs are stored in the new object).

        Raises:
          KeyError: If given edges are not found in the universe.

        See Also:
          copy()
        """
        obj = graphset_or_constraints
        if isinstance(obj, EdgeVertexSetSet):
            self._ss = obj._ss.copy()
        elif isinstance(obj, setset_base):
            self._ss = obj.copy()
        else:
            if obj is None:
                obj = []
            elif obj == {}:
                from graphillion import GraphSet
                self._ss = GraphSet({}).to_edgevertexsetset()._ss
                return
            elif isinstance(obj, (set, frozenset, list)):  # a list of edgevertexsets [graph+]
                l = []
                for edgesvertices in obj:
                    if not isinstance(edgesvertices, (set, frozenset, list)):
                        raise KeyError(edgesvertices)
                    EdgeVertexSetSet._check_consistency(edgesvertices)
                    l.append(set([EdgeVertexSetSet._conv_edgevertex(e) for e in edgesvertices]))
                obj = l
            elif isinstance(obj, dict):
                raise TypeError("graphset_or_constraints does not support dict other than {}")

            self._ss = setset_base(Universe.ev_objtable, obj)

    def copy(self):
        """Returns a new EdgeVertexSetSet with a shallow copy of `self`.

        Examples:
          >>> gs2 = gs1.copy()
          >>> gs1 -= gs2
          >>> gs1 == gs2
          False

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          __init__()
        """
        return EdgeVertexSetSet(self)

    def __bool__(self):
        return bool(self._ss)

    def __repr__(self):
        return self._repr(Universe.ev_objtable, (self.__class__.__name__ + '([', '])'))

    # obj_to_str: dict[tuple, str]
    def _repr(self, objtable, outer_braces=('[', ']'), inner_braces=('[', ']'), obj_to_str=None):
        n = objtable.num_elems()
        w = {}
        for i in range(1, n + 1):
            e = objtable.int2obj[i]
            w[e] = 1 + float(i) / n**2
        ret = outer_braces[0]
        maxchar = 80
        no_comma = True
        for s in self._ss.min_iter(objtable, w):
            if no_comma:
                no_comma = False
            else:
                ret += ', '
            if obj_to_str is None:
                ret += inner_braces[0] + str(EdgeVertexSetSet._sort_edgesvertices(list(s)))[1:-1] + inner_braces[1]
            else:
                ret += inner_braces[0] + str(EdgeVertexSetSet._sort_edgesvertices([eval(obj_to_str[tuple(obj)]) for obj in s]))[1:-1] + inner_braces[1]
            if len(ret) > maxchar - 2:
                break
        if len(ret) <= maxchar - 2:
            return ret + outer_braces[1]
        else:
            return ret[:(maxchar - 4)] + ' ...'

    def union(self, *others):
        """Returns a new EdgeVertexSetSet with graphs from `self` and all others.

        The `self` is not changed.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2), 1, 2]
          >>> graph3 = [(1, 2), (1, 4), 1, 2, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph2, graph3])
          >>> gs1 | gs2
          EdgeVertexSetSet([[], [(1, 2), 1, 2], [(1, 2), (1, 4), 1, 2, 4]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          intersection(), difference(), symmetric_difference(),
          update()
        """
        return EdgeVertexSetSet(self._ss.union(*[gs._ss for gs in others]))

    def intersection(self, *others):
        """Returns a new EdgeVertexSetSet with graphs common to `self` and all others.

        The `self` is not changed.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2), 1, 2]
          >>> graph3 = [(1, 2), (1, 4), 1, 2, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph2, graph3])
          >>> gs1 & gs2
          EdgeVertexSetSet([[(1, 2), 1, 2]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          union(), difference(), symmetric_difference(),
          intersection_update()
        """
        return EdgeVertexSetSet(self._ss.intersection(*[gs._ss for gs in others]))

    def difference(self, *others):
        """Returns a new EdgeVertexSetSet with graphs in `self` that are not in the others.

        The `self` is not changed.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2), 1, 2]
          >>> graph3 = [(1, 2), (1, 4), 1, 2, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph2, graph3])
          >>> gs1 - gs2
          EdgeVertexSetSet([])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          union(), intersection(), symmetric_difference(),
          difference_update()
        """
        return EdgeVertexSetSet(self._ss.difference(*[gs._ss for gs in others]))

    def symmetric_difference(self, *others):
        """Returns a new EdgeVertexSetSet with graphs in either `self` or `other` but not both.

        The `self` is not changed.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2), 1, 2]
          >>> graph3 = [(1, 2), (1, 4), 1, 2, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph2, graph3])
          >>> gs1 ^ gs2
          EdgeVertexSetSet([[], [(1, 2), (1, 4), 1, 2, 4]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          union(), intersection(), difference(), 
          symmetric_difference_update()
        """
        return EdgeVertexSetSet(self._ss.symmetric_difference(*[gs._ss for gs in others]))

    def update(self, *others):
        """Updates `self`, adding graphs from all others.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2), 1, 2]
          >>> graph3 = [(1, 2), (1, 4), 1, 2, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph2, graph3])
          >>> gs1 |= gs2
          >>> gs1
          EdgeVertexSetSet([[], [(1, 2), 1, 2], [(1, 2), (1, 4), 1, 2, 4]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          union()
        """
        self._ss.update(*[gs._ss for gs in others])
        return self

    def intersection_update(self, *others):
        """Updates `self`, keeping only graphs found in it and all others.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2), 1, 2]
          >>> graph3 = [(1, 2), (1, 4), 1, 2, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph2, graph3])
          >>> gs1 &= gs2
          >>> gs1
          EdgeVertexSetSet([[(1, 2), 1, 2]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          intersection()
        """
        self._ss.intersection_update(*[gs._ss for gs in others])
        return self

    def difference_update(self, *others):
        """Update `self`, removing graphs found in others.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2), 1, 2]
          >>> graph3 = [(1, 2), (1, 4), 1, 2, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph2, graph3])
          >>> gs1 -= gs2
          >>> gs1
          EdgeVertexSetSet([[]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          difference()
        """
        self._ss.difference_update(*[gs._ss for gs in others])
        return self

    def symmetric_difference_update(self, *others):
        """Update `self`, keeping only graphs in either EdgeVertexSetSet, but not in both.

        Examples:
          >>> graph1 = []
          >>> graph2 = [(1, 2), 1, 2]
          >>> graph3 = [(1, 2), (1, 4), 1, 2, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph2, graph3])
          >>> gs1 ^= gs2
          >>> gs1
          EdgeVertexSetSet([[], [(1, 2), (1, 4), 1, 2, 4]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          symmetric_difference()
        """
        self._ss.symmetric_difference_update(*[gs._ss for gs in others])
        return self

    __or__ = union
    __and__ = intersection
    __sub__ = difference
    __xor__ = symmetric_difference

    __ior__ = update
    __iand__ = intersection_update
    __isub__ = difference_update
    __ixor__ = symmetric_difference_update

    def isdisjoint(self, other):
        """Returns True if `self` has no graphs in common with `other`.

        Examples:
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph3, graph4, graph5])
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
          >>> gs1 = EdgeVertexSetSet([graph1, graph3])
          >>> gs2 = EdgeVertexSetSet([graph1, graph2, graph3])
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
          >>> gs1 = EdgeVertexSetSet([graph1, graph2, graph3])
          >>> gs2 = EdgeVertexSetSet([graph1, graph3])
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
          >>> gs = EdgeVertexSetSet([graph1, graph2])
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
        """Returns the number of graphs, or a new EdgeVertexSetSet with `size` edges.

        If no argument is given, this method returns the number of
        graphs in `self`.  Otherwise, this method returns a new
        EdgeVertexSetSet with graphs that have `size` edges; this usage of
        `len(size)` is obsoleted, and use `graph_size(size)` instead.

        This method never raises OverflowError unlike built-in len(gs).

        Examples:
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> gs.len()
          2L

        Examples:
          >>> graph1 = [(1, 2), 1, 2]
          >>> graph2 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph3 = [(1, 2), (1, 4), (2, 3), 1, 2, 3, 4]
          >>> gs = EdgeVertexSetSet([graph1, graph2, graph3])
          >>> gs.len(5)
          EdgeVertexSetSet([[(1, 2), (1, 4), 1, 2, 4]])

        Args:
          size: Optional.  The number of edges in a graph.

        Returns:
          The number of graphs, or a new EdgeVertexSetSet object.

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
          >>> graph1 = [(1, 2), 1, 2]
          >>> graph2 = [(1, 2), (1, 4), 1, 2, 4]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> for g in gs:
          ...   g
          [(1, 2), (1, 4), 1, 2, 4]
          [(1, 2), 1, 2]

        Returns:
          A generator.

        Yields:
          A graph.

        See Also:
          rand_iter(), max_iter(), min_iter()
        """
        for g in self._ss._iter(Universe.ev_objtable):
            try:
                yield EdgeVertexSetSet._conv_ret(g)
            except StopIteration:
                return

    def rand_iter(self):
        """Iterates over graphs uniformly randomly.

        This method relies on its own random number generator, doesn't
        rely on Python random module.

        Examples:
          >>> graph1 = [(1, 2), 1, 2]
          >>> graph2 = [(1, 2), (1, 4), 1, 2, 4]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> for g in gs.rand_iter():
          ...   g
          [(1, 2), 1, 2]
          [(1, 2), (1, 4), 1, 2, 4]

        Returns:
          A generator.

        Yields:
          A graph.

        See Also:
          __iter__(), max_iter(), min_iter()
        """
        for g in self._ss.rand_iter(Universe.ev_objtable):
            try:
                yield EdgeVertexSetSet._conv_ret(g)
            except StopIteration:
                return

    def min_iter(self, weights=None):
        """Iterates over graphs in the ascending order of weights.

        Returns a generator that iterates over graphs in `self`
        EdgeVertexSetSet.  The graphs are selected in the ascending order of
        edge/vertex weights, which are specified by the argument `weights`
        or those set as the universe (1.0 for unspecified edges/vertices).
        The `weights` does not overwrite the weights of the universe.

        Examples:
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(2, 3), 2, 3]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> weights = {(1, 2): 2.0, (1, 4): -3.0, 1: 4.0,
                          2: -5.0, 3: -2.0, 4: 3.0 }  # (2, 3): 1.0
          >>> for g in gs.min_iter(weights):
          ...   g
          [(2, 3), 2, 3]
          [(1, 2), (1, 4), 1, 2, 4]

        Args:
          weights: Optional.  A dictionary of edges/vertices to the weight
            values.

        Returns:
          A generator.

        Yields:
          A graph.

        See Also:
          __iter__(), rand_iter(), max_iter()

        """
        if weights is None:
            weights = Universe.weights
        for g in self._ss.min_iter(Universe.ev_objtable, weights):
            try:
                yield EdgeVertexSetSet._conv_ret(g)
            except StopIteration:
                return

    def max_iter(self, weights=None):
        """Iterates over graphs in the descending order of weights.

        Returns a generator that iterates over graphs in `self`
        EdgeVertexSetSet.  The graphs are selected in the descending order of
        edge/vertex weights, which are specified by the argument `weights` or
        those set as the universe (1.0 for unspecified edges/vertices).  The
        `weights` does not overwrite the weights of the universe.

        Examples:
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(2, 3), 2, 3]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> weights = {(1, 2): 2.0, (1, 4): -3.0, 1: 4.0,
                          2: -5.0, 3: -2.0, 4: 3.0 }  # (2, 3): 1.0
          >>> for g in gs.max_iter(weights):
          ...   g
          [(1, 2), (1, 4), 1, 2, 4]
          [(2, 3), 2, 3]

        Args:
          weights: Optional.  A dictionary of edges/vertices to the weight
            values.

        Returns:
          A generator.

        Yields:
          A graph.

        See Also:
          __iter__(), rand_iter(), min_iter()
        """
        if weights is None:
            weights = Universe.weights
        for g in self._ss.max_iter(Universe.ev_objtable, weights):
            try:
                yield EdgeVertexSetSet._conv_ret(g)
            except StopIteration:
                return

    def __contains__(self, obj):
        """Returns True if `obj` is in the `self`, False otherwise.
        When `obj` is a graph (list of edges/vertices), returns True
        if the graph is in the `self`. When `obj` is an edge or a
        vertex, returns True if a graph containing `obj` is in the `self`.

        Use the expression `obj in gs`.

        Examples:
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(2, 3), 2, 3]
          >>> gs = GraphSet([graph1, graph2])
          >>> graph1 in gs
          True
          >>> (1, 2) in gs
          True
          >>> (1, 5) in gs  # assume that (1, 5) is in the universe
          False
          >>> 1 in gs
          True
          >>> 5 in gs
          False

        Args:
          obj: A graph (an edge/vertex list), an edge, or a vertex
            in the universe.

        Returns:
          True or False.

        Raises:
          KeyError: If the given object is not found in the universe.
        """
        type, obj = EdgeVertexSetSet._conv_arg(obj)
        if type == 'edgesvertices' or type == 'edge' or type == 'vertex':
            return self._ss._contains(Universe.ev_objtable, obj)
        raise TypeError(obj)

    def add(self, edgesvertices):
        """Adds a given graph to `self`.

        If a graph (a list of edges/vertices) is given, the graph
        is just added to `self` EdgeVertexSetSet. The `self` will
        be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(2, 3), 2, 3]
          >>> graph3 = [(1, 4), (2, 3), 1, 2, 3, 4]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> gs.add(graph3)
          >>> for g in gs:
          ...     g
          [(1, 2), (1, 4), 1, 2, 4]
          [(2, 3), 2, 3]
          [(1, 4), (2, 3), 1, 2, 3, 4]

        Args:
          graph: A graph (an edge/vertex list).

        Returns:
          None.

        Raises:
          KeyError: If a given edge or vertex is not found in the
            universe.

        See Also:
          add_vertex(), remove(), discard()
        """
        type, obj = EdgeVertexSetSet._conv_arg(edgesvertices)
        if type == 'edgesvertices':
            self._ss.add(Universe.ev_objtable, obj)
        else:
            raise TypeError(edgesvertices)
    
    def add_vertex(self, vertex):
        """Adds a vertex to each graph in `self`.

        The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(2, 3), 2, 3]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> gs.add_vertex(1)
          >>> for g in gs:
          ...     g
          EdgeVertexSetSet([[(1, 2), (1, 4), 1, 2, 4], [(2, 3), 1, 2, 3]])

        Args:
          vertex: A vertex.

        Returns:
          None.

        Raises:
          KeyError: If a vertex is not found in the universe.

        See Also:
          add()
        """
        type, obj = EdgeVertexSetSet._conv_arg(vertex)
        if type == 'vertex':
            self._ss.add(Universe.ev_objtable, obj)
        else:
            raise TypeError(vertex)

    def remove(self, obj):
        """Removes a given graph from `self`.

        A given graph is just removed from `self` EdgeVertexSetSet.
        The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(2, 3), 2, 3]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> gs.remove(graph2)
          >>> gs
          EdgeVertexSetSet([[(1, 2), (1, 4), 1, 2, 4]])

        Args:
          obj: A graph (an edge/vertex list).

        Returns:
          None.

        Raises:
          KeyError: If a given edge or vertex is not found in the
            universe, or if the given graph is not stored in `self`.

        See Also:
          add(), discard(), pop()
        """
        type, obj = EdgeVertexSetSet._conv_arg(obj)
        if type == 'edgesvertices':
            self._ss.remove(Universe.ev_objtable, obj)
        else:
            raise TypeError(obj)
        return None

    def discard(self, obj):
        """Removes a given graph from `self`.

        A graph is just removed from `self` EdgeVertexSetSet.
        The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(2, 3), 2, 3]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> gs.discard(graph2)
          >>> gs
          EdgeVertexSetSet([[(1, 2), (1, 4), 1, 2, 4]])

        Args:
          obj: A graph (an edge/vertex list).

        Returns:
          None.

        Raises:
          KeyError: If a given edge or vertex is not found in the
            universe.

        See Also:
          add(), remove(), pop()
        """
        type, obj = EdgeVertexSetSet._conv_arg(obj)
        if type == 'edgesvertices':
            self._ss.discard(Universe.ev_objtable, obj)
        else:
            raise TypeError(obj)
        return None

    def pop(self):
        """Removes and returns an arbitrary graph from `self`.

        The `self` will be changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(2, 3), 2, 3]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> gs.pop()
          [(1, 2), (1, 4), 1, 2, 4]
          >>> gs
          EdgeVertexSetSet([[(2, 3), 2, 3]])

        Returns:
          A graph.

        Raises:
          KeyError: If `self` is empty.

        See Also:
          remove(), discard(), choice()
        """
        return EdgeVertexSetSet._conv_ret(self._ss.pop(Universe.ev_objtable))

    def clear(self):
        """Removes all graphs from `self`.

        Examples:
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(2, 3), 2, 3]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> gs.clear()
          >>> gs
          EdgeVertexSetSet([])
        """
        return self._ss.clear()

    def minimal(self):
        """Returns a new EdgeVertexSetSet of minimal graphs.

        The minimal sets are defined by,
          gs.minimal() = {a \\in gs | b \\in gs and a \\subseteq -> a = b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), 1, 2]
          >>> graph2 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph3 = [(1, 4), (2, 3), 1, 2, 3, 4]
          >>> gs = EdgeVertexSetSet([graph1, graph2, graph3])
          >>> gs.minimal()
          EdgeVertexSetSet([[(1, 2), 1, 2], [(1, 4), (2, 3), 1, 2, 3, 4]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          maximal(), blocking()
        """
        return EdgeVertexSetSet(self._ss.minimal())

    def maximal(self):
        """Returns a new EdgeVertexSetSet of maximal graphs.

        The maximal sets are defined by,
          gs.maximal() = {a \\in gs | b \\in gs and a \\superseteq -> a = b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), 1, 2]
          >>> graph2 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph3 = [(1, 4), (2, 3), 1, 2, 3, 4]
          >>> gs = EdgeVertexSetSet([graph1, graph2, graph3])
          >>> gs.maximal()
          EdgeVertexSetSet([[(1, 2), (1, 4), 1, 2, 4], [(1, 4), (2, 3), 1, 2, 3, 4]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          minimal()
        """
        return EdgeVertexSetSet(self._ss.maximal())

    def smaller(self, size):
        """Returns a new EdgeVertexSetSet with graphs that have less than `size` edges/vertices.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), 1, 2]
          >>> graph2 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph3 = [(1, 2), (1, 4), (2, 3), 1, 2, 3, 4]
          >>> gs = EdgeVertexSetSet([graph1, graph2, graph3])
          >>> gs.smaller(5)
          EdgeVertexSetSet([[(1, 2), 1, 2]])

        Args:
          size: The number of edges/vertices in a graph.

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          larger(), graph_size()
        """
        return EdgeVertexSetSet(self._ss.smaller(size))

    def larger(self, size):
        """Returns a new EdgeVertexSetSet with graphs that have more than `size` edges/vertices.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), 1, 2]
          >>> graph2 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph3 = [(1, 2), (1, 4), (2, 3), 1, 2, 3, 4]
          >>> gs = EdgeVertexSetSet([graph1, graph2, graph3])
          >>> gs.larger(2)
          EdgeVertexSetSet([[(1, 2), (1, 4), (2, 3), 1, 2, 3, 4]])

        Args:
          size: The number of edges/vertices in a graph.

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          smaller(), graph_size()
        """
        return EdgeVertexSetSet(self._ss.larger(size))

    def graph_size(self, size):
        """Returns a new EdgeVertexSetSet with `size` edges/vertices.

        This method returns a new EdgeVertexSetSet with graphs that have
        `size` edges/vertices.

        Examples:
          >>> graph1 = [(1, 2), 1, 2]
          >>> graph2 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph3 = [(1, 2), (1, 4), (2, 3), 1, 2, 3, 4]
          >>> gs = EdgeVertexSetSet([graph1, graph2, graph3])
          >>> gs.graph_size(5)
          EdgeVertexSetSet([[(1, 2), (1, 4), 1, 2, 4]])

        Args:
          size: The number of edges/vertices in a graph.

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          smaller(), larger()

        """
        return EdgeVertexSetSet(self._ss.set_size(size))

    def join(self, other):
        """Returns a new EdgeVertexSetSet of join between `self` and `other`.

        The join operation is defined by,
          gs1 \\sqcup gs2 = {a \\cup b | a \\in gs1 and b \\in gs2}.
        D. Knuth, Exercise 203, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), 1, 2]
          >>> graph2 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph3 = [(2, 3), 2, 3]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph3])
          >>> gs1.join(gs2)
          EdgeVertexSetSet([[(1, 2), (2, 3), 1, 2, 3], [(1, 2), (1, 4), (2, 3), 1, 2, 3, 4]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          meet()
        """
        return EdgeVertexSetSet(self._ss.join(other._ss))

    def meet(self, other):
        """Returns a new EdgeVertexSetSet of meet between `self` and `other`.

        The meet operation is defined by,
          gs1 \\sqcap gs2 = {a \\cap b | a \\in gs1 and b \\in gs2}.
        D. Knuth, Exercise 203, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(1, 2), (2, 3), 1, 2, 3]
          >>> graph3 = [(1, 4), (2, 3), 1, 2, 3, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph3])
          >>> gs1.meet(gs2)
          EdgeVertexSetSet([[(1, 4), 1, 2, 4], [(2, 3), 1, 2, 3]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          join()
        """
        return EdgeVertexSetSet(self._ss.meet(other._ss))

    def subgraphs(self, other):
        """Returns a new EdgeVertexSetSet with subgraphs of a graph in `other`.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), 1, 2]
          >>> graph2 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph3 = graph1 + [(2, 3), 3]
          >>> graph4 = [(1, 4), (2, 3), 1, 2, 3, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph3, graph4])
          >>> gs1.subgraphs(gs2)
          EdgeVertexSetSet([[(1, 2), 1, 2]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          supersets(), non_subsets()
        """
        return EdgeVertexSetSet(self._ss.subsets(other._ss))

    def supergraphs(self, other):
        """Returns a new EdgeVertexSetSet with supergraphs of a graph in `other`.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (2, 3), 1, 2, 3]
          >>> graph2 = [(1, 4), (2, 3), 1, 2, 3, 4]
          >>> graph3 = [(1, 2), 1, 2]          # graph1 - (2, 3)
          >>> graph4 = [(1, 2), (1, 4), 1, 2, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph3, graph4])
          >>> gs1.supergraphs(gs2)
          EdgeVertexSetSet([[(1, 2), (2, 3), 1, 2, 3]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          subsets(), non_supersets()
        """
        return EdgeVertexSetSet(self._ss.supersets(Universe.ev_objtable, other._ss))

    def non_subgraphs(self, other):
        """Returns a new EdgeVertexSetSet with graphs that aren't subgraphs of any graph in `other`.

        The `self` is not changed.

        The non_subsets are defined by,
          gs1.non_subsets(gs2) = {a \\in gs1 | b \\in gs2 -> a \\not\\subseteq b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        Examples:
          >>> graph1 = [(1, 2), 1, 2]
          >>> graph2 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph3 = [(1, 2), (2, 3), 1, 2, 3]
          >>> graph4 = [(1, 4), (2, 3), 1, 2, 3, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph3, graph4])
          >>> gs1.non_subgraphs(gs2)
          EdgeVertexSetSet([[(1, 2), (1, 4), 1, 2, 4]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          non_supersets(), subsets()
        """
        return EdgeVertexSetSet(self._ss.non_subsets(other._ss))

    def non_supergraphs(self, other):
        """Returns a new EdgeVertexSetSet with graphs that aren't supergraphs of any graph in `other`.

        The `self` is not changed.

        The non_supersets are defined by,
          gs1.non_supersets(gs2) = {a \\in gs1 | b \\in gs2 -> a \\not\\superseteq b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        Examples:
          >>> graph1 = [(1, 2), (2, 3), 1, 2, 3]
          >>> graph2 = [(1, 4), (2, 3), 1, 2, 3, 4]
          >>> graph3 = [(1, 2), 1, 2]
          >>> graph4 = [(1, 2), (1, 4), 1, 2, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph3, graph4])
          >>> gs1.non_supergraphs(gs2)
          EdgeVertexSetSet([[(1, 4), (2, 3), 1, 2, 3, 4]])

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          non_subsets(), supersets()
        """
        return EdgeVertexSetSet(self._ss.non_supersets(Universe.ev_objtable, other._ss))

    def including(self, obj):
        """Returns a new EdgeVertexSetSet that includes supergraphs of `obj`.

        Returns a new set of graphs that include `obj`, which can be a
        EdgeVertexSetSet, a graph, an edge, or a vertex.  If `obj` is a
        EdgeVertexSetSet, a graph returned includes *one of* graphs in the
        given EdgeVertexSetSet.

        The graphs stored in the new EdgeVertexSetSet are selected from `self`
        EdgeVertexSetSet.  The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(2, 3), 2, 3]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> vertex = 4
          >>> gs.including(vertex)
          EdgeVertexSetSet([[(1, 2), (1, 4), 1, 2, 4]])

        Args:
          obj: A EdgeVertexSetSet, a graph (an edge/vertex list),
            an edge, or a vertex.

        Returns:
          A new EdgeVertexSetSet object.

        Raises:
          KeyError: If a given edge or a vertex is not found in the
            universe.

        See Also:
          excluding()
        """
        type, obj = EdgeVertexSetSet._conv_arg(obj)
        if type == 'edgevertexsetset':
            return EdgeVertexSetSet(self._ss.supersets(Universe.ev_objtable, obj._ss))
        elif type == 'edgesvertices':
            return self.including(EdgeVertexSetSet([obj]))
        elif type == 'edge' or type == 'vertex':
            return EdgeVertexSetSet(self._ss.supersets(Universe.ev_objtable, obj))
        else:
            raise TypeError(obj)

    def excluding(self, obj):
        """Returns a new EdgeVertexSetSet that doesn't include `obj`.

        Returns a new set of graphs that don't include `obj`, which
        can be an EdgeVertexSetSet, a graph, an edge, or a vertex.  If `obj` is
        an EdgeVertexSetSet, a graph returned doesn't include *any of* graphs
        in the given EdgeVertexSetSet.

        The graphs stored in the new EdgeVertexSetSet are selected from `self`
        EdgeVertexSetSet.  The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(2, 3), 2, 3]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> vertex = 4
          >>> gs.excluding(vertex)
          EdgeVertexSetSet([[(2, 3), 2, 3]])

        Args:
          obj: A EdgeVertexSetSet, a graph (an edge/vertex list),
            an edge, or a vertex.

        Returns:
          A new EdgeVertexSetSet object.

        Raises:
          KeyError: If a given edge or vertex is not found in the
            universe.

        See Also:
          including()
        """
        type, obj = EdgeVertexSetSet._conv_arg(obj)
        if type == 'edgevertexsetset':
            return self - self.including(obj)
        elif type == 'edgesvertices':
            return self.excluding(EdgeVertexSetSet([obj]))
        elif type == 'edge' or type == 'vertex':
            return EdgeVertexSetSet(self._ss.non_supersets(Universe.ev_objtable, obj))
        else:
            raise TypeError(obj)

    def included(self, obj):
        """Returns a new EdgeVertexSetSet with subgraphs of a graph in `obj`.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), 1, 2]
          >>> graph2 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph3 = graph1 + [(2, 3), 2, 3]
          >>> graph4 = [(1, 4), (2, 3), 1, 2, 3, 4]
          >>> gs1 = EdgeVertexSetSet([graph1, graph2])
          >>> gs2 = EdgeVertexSetSet([graph3, graph4])
          >>> gs1.included(gs2)
          EdgeVertexSetSet([[(1, 2), 1, 2]])

        Args:
          obj: A EdgeVertexSetSet or a graph (an edge/vertex list).

        Returns:
          A new EdgeVertexSetSet object.

        See Also:
          including()
        """
        type, obj = EdgeVertexSetSet._conv_arg(obj)
        if type == 'edgevertexsetset':
            return EdgeVertexSetSet(self._ss.subsets(obj._ss))
        elif type == 'edgesvertices':
            return self.included(EdgeVertexSetSet([obj]))
        else:
            raise TypeError(obj)

    def choice(self):
        """Returns an arbitrary graph from `self`.

        The `self` is not changed.

        Examples:
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(2, 3), 2, 3]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> gs.choice()
          [(1, 2), (1, 4), 1, 2, 4]

        Returns:
          A graph.

        Raises:
          KeyError: If `self` is empty.

        See Also:
          pop()
        """
        return EdgeVertexSetSet._conv_ret(self._ss.choice(Universe.ev_objtable))

    def probability(self, probabilities):
        """Returns the probability of `self` with edge/vertex `probabilities`.

        This method calculates the probability of occurrence of any
        graph in `self` given `probabilities` of each edge.

        Examples:
          >>> Universe.set_universe([(1, 2), (1, 4), (2, 3)])
          >>> graph1 = [(1, 2), (1, 4), 1, 2, 4]
          >>> graph2 = [(2, 3), 2, 3]
          >>> gs = EdgeVertexSetSet([graph1, graph2])
          >>> probabilities = {(1, 2): .9, (1, 4): .8, (2, 3): .7, 1:.6, 2:.5, 3:.4, 4:.3}
          >>> gs.probability(probabilities)
          0.012448
          # 0.9*0.8*(1-0.7)*0.6*0.5*(1-0.4)*0.3
          #   + (1-0.9)*(1-0.8)*0.7*(1-0.6)*0.5*0.4*(1-0.3)

        Args:
          probabilities: A dictionary of probabilities of each edge.

        Returns:
          Probability.

        Raises:
          KeyError: If a given edge/vertex is not found in the universe.
        """
        probabilities = {EdgeVertexSetSet._conv_edgevertex(ev): p for ev, p in probabilities.items()}
        return self._ss.probability(Universe.ev_objtable, probabilities)

    def cost_le(self, costs, cost_bound):
        """Returns a new EdgeVertexSetSet with subgraphs whose cost is less than or equal to the cost bound.

        This method constructs a Graphset of subgraphs
        whose cost is less than or equal to the cost bound,
        where `costs` of each edge/vertex and the `cost_bound` are
        given as arguments.

        Examples:
          >>> universe = [(1, 2), (1, 4), (2, 3), (3, 4)]
          >>> Universe.set_universe(universe)

          >>> graph1 = [(1, 2), (2, 3), 1, 2, 3]
          >>> graph2 = [(3, 4), 3, 4]
          >>> graph3 = [(1, 2), (1, 4), (3, 4), 1, 2, 3, 4]
          >>> gs = EdgeVertexSetSet([graph1, graph2, graph3])
          >>> costs = {(1, 2): 2, (1, 4): 3, (2, 3): 1, (3, 4): 7}
          >>> cost_bound = 7
          >>> print(gs.cost_le(costs, cost_bound))
          EdgeVertexSetSet([[(3, 4), 3, 4], [(1, 2), (2, 3), 1, 2, 3]])

        Args:
          costs: A dictionary of the cost of each edge/vertex.
          cost_bound: The upper limit of the cost of each graph. 32 bit signed integer.

        Returns:
          A new EdgeVertexSetSet object.

        Raises:
          KeyError: If a given edge/vertex is not found in the universe.
          AssertionError: If the cost of at least one edge/vertex is not given, or outside the range of 32 bit signed integer.
          TypeError: If at least one cost is not integer.

        """
        return EdgeVertexSetSet(self._ss.cost_le(objtable=Universe.ev_objtable, costs=costs, cost_bound=cost_bound))

    def cost_ge(self, costs, cost_bound):
        """Returns a new EdgeVertexSetSet with subgraphs whose cost is greater than or equal to the cost bound.

        This method constructs a Graphset of subgraphs
        whose cost is greater than or equal to the cost bound,
        where `costs` of each edge/vertex and the `cost_bound` are
        given as arguments.

        Examples:
          >>> universe = [(1, 2), (1, 4), (2, 3), (3, 4)]
          >>> Universe.set_universe(universe)

          >>> graph1 = [(1, 2), (2, 3), 1, 2, 3]
          >>> graph2 = [(3, 4), 3, 4]
          >>> graph3 = [(1, 2), (1, 4), (3, 4), 1, 2, 3, 4]
          >>> gs = EdgeVertexSetSet([graph1, graph2, graph3])
          >>> costs = {(1, 2): 2, (1, 4): 3, (2, 3): 1, (3, 4): 7}
          >>> cost_bound = 7
          >>> print(gs.cost_ge(costs, cost_bound))
          EdgeVertexSetSet([[(3, 4), 3, 4], [(1, 2), (1, 4), (3, 4), 1, 2, 3, 4]])

        Args:
          costs: A dictionary of the cost of each edge/vertex.
          cost_bound: The lower limit of the cost of each graph. 32 bit signed integer.

        Returns:
          A new EdgeVertexSetSet object.

        Raises:
          KeyError: If a given edge/vertex is not found in the universe.
          AssertionError: If the cost of at least one edge/vertex is not given, or outside the range of 32 bit signed integer.
          TypeError: If at least one cost is not integer.

        """
        inv_costs = {e: -cost for e, cost in costs.items()}
        return EdgeVertexSetSet(self._ss.cost_le(objtable=Universe.ev_objtable, costs=inv_costs, cost_bound=-cost_bound))

    def cost_eq(self, costs, cost_bound):
        """Returns a new EdgeVertexSetSet with subgraphs whose cost is equal to the cost bound.

        This method constructs a Graphset of subgraphs
        whose cost is equal to the cost bound,
        where `costs` of each edge/vertex and the `cost_bound` are
        given as arguments.

        Examples:
          >>> universe = [(1, 2), (1, 4), (2, 3), (3, 4)]
          >>> Universe.set_universe(universe)

          >>> graph1 = [(1, 2), (2, 3), 1, 2, 3]
          >>> graph2 = [(3, 4), 3, 4]
          >>> graph3 = [(1, 2), (1, 4), (3, 4), 1, 2, 3, 4]
          >>> gs = EdgeVertexSetSet([graph1, graph2, graph3])
          >>> costs = {(1, 2): 2, (1, 4): 3, (2, 3): 1, (3, 4): 7}
          >>> cost_bound = 7
          >>> print(gs.cost_eq(costs, cost_bound))
          EdgeVertexSetSet([[(3, 4), 3, 4]])

        Args:
          costs: A dictionary of the cost of each edge/vertex.
          cost_bound: The limit of the cost of each graph. 32 bit signed integer.

        Returns:
          A new EdgeVertexSetSet object.

        Raises:
          KeyError: If a given edge/vertex is not found in the universe.
          AssertionError: If the cost of at least one edge/vertex is not given, or outside the range of 32 bit signed integer.
          TypeError: If at least one cost is not integer.

        """
        le_ss = self._ss.cost_le(objtable=Universe.ev_objtable, costs=costs, cost_bound=cost_bound)
        lt_ss = self._ss.cost_le(objtable=Universe.ev_objtable, costs=costs, cost_bound=cost_bound - 1)
        return EdgeVertexSetSet(le_ss.difference(lt_ss))

    def dump(self, fp):
        """Serialize `self` to a file `fp`.

        This method does not serialize the universe, which should be
        saved separately by pickle.

        Examples:
          >>> import pickle
          >>> fp = open('/path/to/graphset', 'wb')
          >>> gs.dump(fp)
          >>> fp = open('/path/to/universe' 'wb')
          >>> pickle.dump(EdgeVertexSetSet.universe(), fp)

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
          >>> universe_str = pickle.dumps(EdgeVertexSetSet.universe())

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
          >>> Universe.set_universe(pickle.load(fp), traversal='as-is')
          >>> fp = open('/path/to/graphset')
          >>> gs = EdgeVertexSetSet.load(fp)

        See Also:
          loads(), dump()
        """
        return EdgeVertexSetSet(setset_base.load(fp))

    @staticmethod
    def loads(s):
        """Deserialize `s` to `self`.

        This method does not deserialize the universe, which should be
        loaded separately by pickle.

        Args:
          s: A string instance.

        Examples:
          >>> import pickle
          >>> Universe.set_universe(pickle.loads(universe_str), traversal='as-is')
          >>> gs = EdgeVertexSetSet.load(graphset_str)

        See Also:
          load(), dumps()
        """
        return EdgeVertexSetSet(setset_base.loads(s))

    @staticmethod
    def reliability(probabilities, terminals):
        raise NotImplementedError("The reliability method of EdgeVertexSetSet has not been implemented yet.")

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
    def _conv_arg(obj):
        if isinstance(obj, EdgeVertexSetSet):
            return 'edgevertexsetset', obj
        elif isinstance(obj, (set, frozenset, list)):
            return 'edgesvertices', set([EdgeVertexSetSet._conv_edgevertex(e) for e in obj])
        elif isinstance(obj, tuple):
            return 'edge', EdgeVertexSetSet._conv_edgevertex(obj)
        elif obj in Universe.edge_vertex_kind and not Universe.edge_vertex_kind[obj]: # obj is a vertex
            return 'vertex', obj
        else:
            raise KeyError(obj)

    @staticmethod
    def _conv_edgevertex(edge_or_vertex):
        if not isinstance(edge_or_vertex, tuple) or len(edge_or_vertex) < 2:
            return edge_or_vertex # interpret it as a vertex
        if len(edge_or_vertex) > 2:
            edge_or_vertex = edge_or_vertex[:2]
        if edge_or_vertex in Universe.ev_objtable.obj2int:
            return edge_or_vertex
        elif (edge_or_vertex[1], edge_or_vertex[0]) in Universe.ev_objtable.obj2int:
            return (edge_or_vertex[1], edge_or_vertex[0])
        raise KeyError(edge_or_vertex)

    @staticmethod
    def _conv_ret(obj):
        if isinstance(obj, (set, frozenset)):  # a graph
            #return EdgeVertexSetSet.converters['to_graph'](sorted(list(obj)))
            return EdgeVertexSetSet._sort_edgesvertices(list(obj))
        raise TypeError(obj)

    # check if each member in `edgesvertices' is in the universe
    # and if the endpoints of each edge in `edgesvertices' are also in `edgesvertices'
    @staticmethod
    def _check_consistency(edgesvertices):
        for ev in edgesvertices:
            if ev not in Universe.edge_vertex_kind:
                raise KeyError("{} is not in the universe".format(ev))
            if Universe.edge_vertex_kind[ev]: # ev is an edge
                for v in ev:
                    if v not in edgesvertices:
                        raise KeyError("{} is in the edge/vertex list but {} is not in it".format(ev, v))

    @staticmethod
    def _sort_edgesvertices(edgesvertices):
        vertices = []
        edges = []
        for ev in edgesvertices:
            if Universe.edge_vertex_kind[ev]: # ev is an edge
                edges.append(ev)
            else:
                vertices.append(ev)
        vertices.sort()
        edges.sort()
        return edges + vertices
