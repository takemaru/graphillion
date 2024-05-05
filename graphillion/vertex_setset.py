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

"""Module for a family of vertex sets.
"""

from collections import deque
from future.utils import viewitems

import _graphillion
from graphillion import setset

class VertexSetSet(object):
    """Represents and manipulates a family of vertex sets.

    A VertexSetSet object stores a family of vertex sets.  A set of
    vertices stored must be a subset of the vertices of the universal graph, and is
    represented by a list of the vertices of the universal graph.
    A vertex can be any hashable object like a number, a text string, and a tuple.

    The universal graph must be defined before creating VertexSetSet
    objects by `VertexSetSet.set_universe()` method.
    Furthermore, for a technical reason, `GraphSet.set_universe()` or
    `setset.set_universe()` must be called before the first call of
    `VertexSetSet.set_universet()` and the size of the universe of GraphSet
    or setset must not be smaller than that of the universe of VertexSetSet.
    By the limitation above, for example, if the universe graph is a forest,
    GraphSet and VertexSetSet cannot be used simultaneously.

    Like Python set types, VertexSetSet supports `vertexset in vertexsetset`,
    `len(vertexsetset)`, and `for vertexset in vertexsetset`.  It also supports
    all set methods and operators,
    * isdisjoint(), issubset(), issuperset(), union(), intersection(),
      difference(), symmetric_difference(), copy(), update(),
      intersection_update(), difference_update(),
      symmetric_difference_update(), add(), remove(), discard(),
      pop(), clear(),
    * ==, !=, <=, <, >=, >, |, &, -, ^, |=, &=, -=, ^=.

    Examples:
      >>> from graphillion import GraphSet, VertexSetSet

      >>> universe_graph = [(1, 2), (1, 3), (1, 4),
                            (2, 5), (3, 5)]
      >>> GraphSet.set_universe(universe_graph)
      >>> universe_vertices = [1, 2, 3, 4, 5]
      >>> VertexSetSet.set_universe(universe_vertices)

      Make an VertexSetSet instance representing {{1, 2}, {2, 3, 4}, {2, 5}, {3, 5}}.

      >>> vss = VertexSetSet([[1, 2], [2, 3, 4], [2, 5], [3, 5]])
      >>> len(vss)
      4

      Give constraints in which vertex 1 must not be passed but 2 must
      be passed, and show the famuly of vertex sets.

      >>> vss2 = vss.excluding(1).including(2)
      >>> for vs in vss2:
      ...   vs
      [2, 3, 4]
      [2, 5]

    """
    def __init__(self, vertex_setset_or_constraints=None):
        """Initializes a VertexSetSet object with a set of graphs or constraints.

        Examples:
          >>> vertex_set1 = [1, 4]
          >>> vertex_set2 = [2, 3]
          >>> VertexSetSet([vertex_set1, vertex_set2])
          VertexSetSet([['2', '3'], ['1', '4']])
          >>> VertexSetSet({"include": vertex_set1, "exclude": vertex_set2})
          VertexSetSet([['1', '4'], ['1', '4', '5']])

        Args:
          vertex_setset_or_constraints: A family of vertex sets represented by a
            list of list of vertices:

            [[1, 4], [2, 3]]

            Or constraints represented by a dict of included or
            excluded vertices (not-specified vertices are not cared):

            {"include": [1, 4], "exclude": [2, 3]}

            If no argument is given, it is treated as an empty list
            `[]` and an empty VertexSetSet is returned.  An empty dict
            `{}` means that no constraint is specified, and so a
            VertexSetSet including all possible vertex sets in the universe is
            returned (let N the number of vertices in the universe, 2^N
            vertex sets are stored in the new object).

        Raises:
          KeyError: If given vertices are not found in the universe.

        See Also:
          copy()
        """
        obj = vertex_setset_or_constraints
        if isinstance(obj, VertexSetSet):
            self._ss = obj._ss.copy()
        elif isinstance(obj, setset):
            self._ss = obj.copy()
        else:
            if obj is None:
                obj = []
            elif isinstance(obj, (set, frozenset, list)): # a list of vertex lists
                for vertices in obj:
                    for vertex in vertices:
                        if vertex not in VertexSetSet._universe_vertices:
                            raise KeyError("invalid vertex:", vertex)

                l = []
                for vertices in obj:
                    l.append([VertexSetSet._vertex2obj[vertex] for vertex in vertices])
                obj = l
            elif isinstance(obj, dict): # constraints
                d = {}
                for k, l in viewitems(obj):
                    d[k] = list(VertexSetSet._conv_vertices_to_objs(l))
                if "exclude" not in d: d["exclude"] = []
                for obj in setset._int2obj[1 : -VertexSetSet._vertex_num]:
                    d["exclude"].append(obj)
                obj = d
            self._ss = setset(obj)

    def copy(self):
        """Returns a new VertexSetSet with a shallow copy of `self`.

        Examples:
          >>> vss2 = vss1.copy()
          >>> vss1 -= vss2
          >>> vss1 == vss2
          False

        Returns:
          A new VertexSetSet object.

        See Also:
          __init__()
        """
        return VertexSetSet(self)

    def __nonzero__(self):
        return bool(self._ss)

    __bool__ = __nonzero__

    def __repr__(self):
        return setset._repr(self._ss,
                            (self.__class__.__name__ + "([", "])"),
                            ("[", "]"),
                            obj_to_str=VertexSetSet._obj2str)

    def union(self, *others):
        """Returns a new VertexSetSet with vertex sets from `self` and all others.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = []
          >>> vertex_set2 = [1]
          >>> vertex_set3 = [1, 2]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set2, vertex_set3])
          >>> vss1 | vss2
          VertexSetSet([[], ['1'], ['1', '2']])

        Returns:
          A new VertexSetSet object.

        See Also:
          intersection(), difference(), symmetric_difference(),
          update()
        """
        return VertexSetSet(self._ss.union(*[vss._ss for vss in others]))

    def intersection(self, *others):
        """Returns a new VertexSetSet with vertex sets common to `self` and all others.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = []
          >>> vertex_set2 = [1]
          >>> vertex_set3 = [1, 2]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set2, vertex_set3])
          >>> vss1 & vss2
          VertexSetSet([['1']])

        Returns:
          A new VertexSetSet object.

        See Also:
          union(), difference(), symmetric_difference(),
          intersection_update()
        """
        return VertexSetSet(self._ss.intersection(*[vss._ss for vss in others]))

    def difference(self, *others):
        """Returns a new VertexSetSet with vertex sets in `self` that are not in the others.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = []
          >>> vertex_set2 = [1]
          >>> vertex_set3 = [1, 2]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set2, vertex_set3])
          >>> vss1 - vss2
          VertexSetSet([])

        Returns:
          A new VertexSetSet object.

        See Also:
          union(), intersection(), symmetric_difference(),
          difference_update()
        """
        return VertexSetSet(self._ss.difference(*[vss._ss for vss in others]))

    def symmetric_difference(self, *others):
        """Returns a new VertexSetSet with vertex sets in either `self` or `other` but not both.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = []
          >>> vertex_set2 = [1]
          >>> vertex_set3 = [1, 2]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set2, vertex_set3])
          >>> vss1 ^ vss2
          VertexSetSet([[], ['1', '2']])

        Returns:
          A new VertexSetSet object.

        See Also:
          union(), intersection(), difference(),
          symmetric_difference_update()
        """
        return VertexSetSet(self._ss.symmetric_difference(*[vss._ss for vss in others]))

    def quotient(self, other):
        """Returns a new VertexSetSet of quotient.

        The quotient is defined by,
          vss1 / vss2 = {a | a \\cup b \\in vss1 and a \\cap b = \\empty, \\forall b \\in vss2}.
        D. Knuth, Exercise 204, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3, 4]
          >>> vertex_set3 = [2]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss / VertexSetSet([vertex_set3])
          VertexSetSet([['1']])

        Returns:
          A new VertexSetSet object.

        See Also:
          remainder(), quotient_update()
        """
        return VertexSetSet(self._ss.quotient(other._ss))

    def remainder(self, other):
        """Returns a new VertexSetSet of remainder.

        The remainder is defined by,
          vss1 % vss2 = vss1 - (vss2 \\sqcup (vss1 / vss2)).
        D. Knuth, Exercise 204, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3, 4]
          >>> vertex_set3 = [2]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss % VertexSetSet([vertex_set3])
          VertexSetSet([['3', '4']])

        Returns:
          A new VertexSetSet object.

        See Also:
          quotient(), remainder_update()
        """
        return VertexSetSet(self._ss.remainder(other._ss))

    def update(self, *others):
        """Updates `self`, adding vertex sets from all others.

        Examples:
          >>> vertex_set1 = []
          >>> vertex_set2 = [1]
          >>> vertex_set3 = [1, 2]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set2, vertex_set3])
          >>> vss1 |= vss2
          >>> vss1
          VertexSetSet([[], ['1'], ['1', '2']])

        Returns:
          A new VertexSetSet object.

        See Also:
          union()
        """
        self._ss.update(*[vss._ss for vss in others])
        return self

    def intersection_update(self, *others):
        """Updates `self`, keeping only vertex sets found in it and all others.

        Examples:
          >>> vertex_set1 = []
          >>> vertex_set2 = [1]
          >>> vertex_set3 = [1, 2]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set2, vertex_set3])
          >>> vss1 |= vss2
          >>> vss1
          VertexSetSet([['1']])

        Returns:
          A new VertexSetSet object.

        See Also:
          intersection()
        """
        self._ss.intersection_update(*[vss._ss for vss in others])
        return self

    def difference_update(self, *others):
        """Update `self`, removing vertex sets found in others.

        Examples:
          >>> vertex_set1 = []
          >>> vertex_set2 = [1]
          >>> vertex_set3 = [1, 2]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set2, vertex_set3])
          >>> vss1 -= vss2
          >>> vss1
          VertexSetSet([[]])

        Returns:
          A new VertexSetSet object.

        See Also:
          difference()
        """
        self._ss.difference_update(*[vss._ss for vss in others])
        return self

    def symmetric_difference_update(self, *others):
        """Update `self`, keeping only vertex sets in either VertexSetSet, but not in both.

        Examples:
          >>> vertex_set1 = []
          >>> vertex_set2 = [1]
          >>> vertex_set3 = [1, 2]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set2, vertex_set3])
          >>> vss1 ^= vss2
          >>> vss1
          VertexSetSet([[], ['1', '2']])

        Returns:
          A new VertexSetSet object.

        See Also:
          symmetric_difference()
        """
        self._ss.symmetric_difference_update(*[vss._ss for vss in others])
        return self

    def quotient_update(self, other):
        """Updates `self` by the quotient.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3, 4]
          >>> vertex_set3 = [2]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss /= VertexSetSet([vertex_set3])
          >>> vss
          VertexSetSet([['1']])

        Returns:
          A new VertexSetSet object.

        See Also:
          quotient()
        """
        self._ss.quotient_update(other._ss)
        return self

    def remainder_update(self, other):
        """Updates `self` by the remainder.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3, 4]
          >>> vertex_set3 = [2]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss %= VertexSetSet([vertex_set3])
          >>> vss
          VertexSetSet([['3', '4']])

        Returns:
          A new VertexSetSet object.

        See Also:
          remainder()
        """
        self._ss.remainder_update(other._ss)
        return self

    def __invert__(self):
        """Returns a new VertexSetSet with vertex sets not stored in `self`.

        Examples:
          >>> VertexSetSet.set_universe([1, 2])
          >>> vertex_set = [1]
          >>> vss = VertexSetSet([vertex_set])
          >>> ~vss
          VertexSetSet([[], ['2'], ['1', '2']])

        Returns:
          A new VertexSetSet object.
        """
        invert_ss = ~self._ss
        for obj in setset._int2obj[1 : -VertexSetSet._vertex_num]:
            invert_ss = invert_ss.non_supersets(obj)
        return VertexSetSet(invert_ss)

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
        """Returns True if `self` has no vertex sets in common with `other`.

        Examples:
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set3, vertex_set4, vertex_set5])
          >>> vss1.isdisjoint(vss2)
          True

        Returns:
          True or False.

        See Also:
          issubset(), issuperset()
        """
        return self._ss.isdisjoint(other._ss)

    def issubset(self, other):
        """Tests if every vertex set in `self` is in `other`.

        Examples:
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set1, vertex_set2, vertex_set3])
          >>> vss1 <= vss2
          True

        Returns:
          True or False.

        See Also:
          issuperset(), isdisjoint()
        """
        return self._ss.issubset(other._ss)

    def issuperset(self, other):
        """Tests if every vertex set in `other` is in `self`.

        Examples:
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2, vertex_set3])
          >>> vss2 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss1 >= vss2
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
          >>> vss < vss
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
          >>> vss > vss
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
        """Returns the number of vertex sets in `self`.

        Use gs.len() if OverflowError raised.

        Examples:
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> len(vss)
          2

        Returns:
          The number of vertex sets.

        Raises:
          OverflowError

        See Also:
          len()
        """
        return len(self._ss)

    def len(self, size=None):
        """Returns the number of vertex sets, or a new VertexSetSet with `size` vertices.

        If no argument is given, this method returns the number of
        vertex sets in `self`.  Otherwise, this method returns a new
        VertexSetSet with vertex sets that have `size` vertices; this usage of
        `len(size)` is obsoleted, and use `graph_size(size)` instead.

        This method never raises OverflowError unlike built-in len(gs).

        Examples:
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss.len()
          2

        Examples:
          >>> vertex_set1 = [1]
          >>> vertex_set2 = [1, 2]
          >>> vertex_set3 = [1, 2, 3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2, vertex_set3])
          >>> vss.len(2)
          VertexSetSet([['1', '2']])

        Args:
          size: Optional.  The number of vertices in a vertex set.

        Returns:
          The number of vertex sets, or a new VertexSetSet object.

        See Also:
          __len__(), smaller(), larger(), graph_size()

        """
        if size is None:
            return self._ss.len()
        else:
            return self.graph_size(size)

    def __iter__(self):
        """Iterates over vertex sets.

        This is the fastest iterator among VertexSetSet iterators, such as
        rand_iter() and max_iter().

        Examples:
          >>> vertex_set1 = [1]
          >>> vertex_set2 = [1, 2]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> for vs in vss:
          ...     vs
          [1, 2]
          [1]

        Returns:
          A generator.

        Yields:
          A vertex set.

        See Also:
          rand_iter(), max_iter(), min_iter()
        """
        for objs in self._ss.__iter__():
            try:
                yield VertexSetSet._sort_vertices(VertexSetSet._conv_objs_to_vertices(objs))
            except StopIteration:
                return

    def rand_iter(self):
        """Iterates over vertex sets uniformly randomly.

        This method relies on its own random number generator, doesn't
        rely on Python random module.

        Examples:
          >>> vertex_set1 = [1]
          >>> vertex_set2 = [1, 2]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> for vs in vss.rand_iter():
          ...     vs
          [1]
          [1, 2]


        Returns:
          A generator.

        Yields:
          A vertex set.

        See Also:
          __iter__(), max_iter(), min_iter()
        """
        for objs in self._ss.rand_iter():
            try:
                yield VertexSetSet._sort_vertices(VertexSetSet._conv_objs_to_vertices(objs))
            except StopIteration:
                return

    def min_iter(self, weights=None):
        """Iterates over vertex sets in the ascending order of weights.

        Returns a generator that iterates over vertex sets in `self`
        VertexSetSet.  The vertex sets are selected in the ascending order of
        vertex weights, which are specified by the argument `weights` or
        those set as the universe (1.0 for unspecified vertices).  The
        `weights` does not overwrite the weights of the universe.

        Examples:
          >>> VertexSetSet.set_universe([1, 2, 3, 4, 5])
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> weights = {1: 2.0, 2: -3.0} # 3: 1.0
          >>> for vs in vss.min_iter(weights):
          ...     vs
          [1, 2]
          [3]


        Args:
          weights: Optional.  A dictionary of vertices to the weight
            values.

        Returns:
          A generator.

        Yields:
          A vertex set.

        See Also:
          __iter__(), rand_iter(), max_iter()

        """
        if weights is None:
            weights = VertexSetSet._obj2weight
        else:
            weights = {VertexSetSet._vertex2obj[vertex]: weight for vertex, weight in weights.items()}
        for objs in self._ss.min_iter(weights):
            try:
                yield VertexSetSet._sort_vertices(VertexSetSet._conv_objs_to_vertices(objs))
            except StopIteration:
                return

    def max_iter(self, weights=None):
        """Iterates over vertex sets in the descending order of weights.

        Returns a generator that iterates over vertex sets in `self`
        VertexSetSet.  The vertex sets are selected in the descending order of
        vertex weights, which are specified by the argument `weights` or
        those set as the universe (1.0 for unspecified vertices).  The
        `weights` does not overwrite the weights of the universe.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> weights = {1: 2.0, 2: -3.0} # 3: 1.0
          >>> for vs in vss.max_iter(weights):
          ...     vs
          [3]
          [1, 2]

        Args:
          weights: Optional.  A dictionary of vertices to the weight
            values.

        Returns:
          A generator.

        Yields:
          A vertex set.

        See Also:
          __iter__(), rand_iter(), min_iter()
        """
        if weights is None:
            weights = VertexSetSet._obj2weight
        else:
            weights = {VertexSetSet._vertex2obj[vertex]: weight for vertex, weight in weights.items()}
        for objs in self._ss.max_iter(weights):
            try:
                yield VertexSetSet._sort_vertices(VertexSetSet._conv_objs_to_vertices(objs))
            except StopIteration:
                return

    def __contains__(self, obj):
        """Returns True if `obj` is in the `self`, False otherwise.

        Use the expression `obj in gs`.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vertex_set1 in vss
          True

        Args:
          obj: A vertex set (a vertex list) or a vertex in the
            universe.

        Returns:
          True or False.

        Raises:
          KeyError: If the given object is not found in the universe.
        """
        type, obj = VertexSetSet._conv_arg(obj)
        if type == "vertex" or type == "vertices":
            return obj in self._ss
        raise TypeError(obj)

    def add(self, vertices_or_vertex):
        """Adds a given vertex set or vertex to `self`.

        If a vertex set is given, the vertex set is just added to `self`
        VertexSetSet.  If an vertex is given, the vertex is grafted to all the
        vertex sets in `self`.  The `self` will be changed.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vertex = 1
          >>> vss.add(vertex)
          >>> vss
          VertexSetSet([['1', '2'], ['1', '3']])

        Args:
          vertices_or_vertex: A vertex set (a vertex list) or a vertex in the
            universe.

        Returns:
          None.

        Raises:
          KeyError: If a given vertex is not found in the
            universe.

        See Also:
          remove(), discard()
        """
        type, obj = VertexSetSet._conv_arg(vertices_or_vertex)
        if type == "vertices" or type == "vertex":
            self._ss.add(obj)
        else:
            raise TypeError(obj)

    def remove(self, obj):
        """Removes a given vertex set or vertex from `self`.

        If a vertex set is given, the vertex set is just removed from `self`
        VertexSetSet.  If an vertex is given, the vertex is removed from all
        the vertex sets in `self`.  The `self` will be changed.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vertex = 1
          >>> vss.remove(vertex)
          >>> vss
          VertexSetSet([['2'], ['3']])

        Args:
          obj: A vertex set (an vertex list) or a vertex in the
            universe.

        Returns:
          None.

        Raises:
          KeyError: If a given vertex is not found in the
            universe, or if the given vertex set is not stored in `self`.

        See Also:
          add(), discard(), pop()
        """
        type, obj = VertexSetSet._conv_arg(obj)
        if type == "vertices" or type == "vertex":
            self._ss.remove(obj)
        else:
            raise TypeError(obj)

    def discard(self, obj):
        """Removes a given vertex set or vertex from `self`.

        If a vertex set is given, the vertex set is just removed from `self`
        VertexSetSet.  If an vertex is given, the vertex is removed from all
        the vertex sets in `self`.  The `self` will be changed.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vertex = 1
          >>> vss.discard(vertex)
          VertexSetSet([['2'], ['3']])

        Args:
          obj: A vertex set (an vertex list) or a vertex in the
            universe.

        Returns:
          None.

        Raises:
          KeyError: If a given vertex is not found in the
            universe.

        See Also:
          add(), remove(), pop()
        """
        type, obj = VertexSetSet._conv_arg(obj)
        if type == "vertices" or type == "vertex":
            self._ss.discard(obj)
        else:
            raise TypeError(obj)

    def pop(self):
        """Removes and returns an arbitrary vertex set from `self`.

        The `self` will be changed.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss.pop()
          [2, 1]
          >>> vss
          VertexSetSet([['3']])

        Returns:
          A vertex set.

        Raises:
          KeyError: If `self` is empty.

        See Also:
          remove(), discard(), choice()
        """
        return VertexSetSet._conv_objs_to_vertices(self._ss.pop())

    def clear(self):
        """Removes all vertex sets from `self`.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss.clear()
          >>> vss
          VertexSetSet([])
        """
        return self._ss.clear()

    def flip(self, vertex):
        """Flips the state of a given vertex over all vertex sets in `self`.

        If a vertex set in `self` includes the given vertex, the vertex is
        removed from the vertex set.  If a vertex set in `self` does not include
        the given vertex, the vertex is added to the vertex set.

        The `self` will be changed.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vertex = 1
          >>> vss.flip(vertex)
          >>> vss
          VertexSetSet([['2'], ['1', '3']])

        Args:
          vertex: An vertex in the universe.

        Returns:
          A new VertexSetSet object.

        Raises:
          KeyError: If a given vertex is not found in the universe.
        """
        type, obj = VertexSetSet._conv_arg(vertex)
        if type == "vertex":
            self._ss.flip(obj)
        else:
            raise TypeError(vertex)

    def minimal(self):
        """Returns a new VertexSetSet of minimal vertex sets.

        The minimal sets are defined by,
          vss.minimal() = {a \\in vss | b \\in vss and a \\supseteq b -> a = b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1]
          >>> vertex_set2 = [1, 2]
          >>> vertex_set3 = [2, 3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2, vertex_set3])
          >>> vss.minimal()
          VertexSetSet([['1'], ['2', '3']])

        Returns:
          A new VertexSetSet object.

        See Also:
          maximal(), blocking()
        """
        return VertexSetSet(self._ss.minimal())

    def maximal(self):
        """Returns a new VertexSetSet of maximal vertex sets.

        The maximal sets are defined by,
          vss.maximal() = {a \\in vss | b \\in vss and a \\subseteq b -> a = b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1]
          >>> vertex_set2 = [1, 2]
          >>> vertex_set3 = [2, 3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2, vertex_set3])
          >>> vss.maximal()
          VertexSetSet([['1', '2'], ['2', '3']])

        Returns:
          A new VertexSetSet object.

        See Also:
          minimal()
        """
        return VertexSetSet(self._ss.maximal())

    def blocking(self):
        """Returns a new VertexSetSet of all blocking (hitting) sets.

        A blocking set is often called a hitting set; all vertex sets in
        `self` contain at least one vertex in the set.  This implies
        that all the vertex sets are destroyed by removing vertices in the
        set.

        The blocking sets are defined by,
          vss.blocking() = {a | b \\in vss -> a \\cap b \\neq \\empty}.
        T. Toda, Hypergraph Dualization Algorithm Based on Binary
        Decision Diagrams.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [2, 3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss.blocking().minimal()
          VertexSetSet([['2'], ['1', '3']])

        Returns:
          A new VertexSetSet object.

        See Also:
          minimal()
        """
        h = self._ss.hitting()
        for obj in setset._int2obj[1 : -VertexSetSet._vertex_num]:
            h = h.non_supersets(setset([[obj]]))
        return VertexSetSet(h)

    hitting = blocking

    def smaller(self, size):
        """Returns a new VertexSetSet with vertex sets that have less than `size` vertices.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1]
          >>> vertex_set2 = [1, 2]
          >>> vertex_set3 = [1, 2, 3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2, vertex_set3])
          >>> vss.smaller(2)
          VertexSetSet([['1']])

        Args:
          size: The number of vertices in a vertex set.

        Returns:
          A new VertexSetSet object.

        See Also:
          larger(), graph_size()
        """
        return VertexSetSet(self._ss.smaller(size))

    def larger(self, size):
        """Returns a new VertexSetSet with vertex sets that have more than `size` vertices.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1]
          >>> vertex_set2 = [1, 2]
          >>> vertex_set3 = [1, 2, 3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2, vertex_set3])
          >>> vss.larger(2)
          VertexSetSet([['1', '2', '3']])

        Args:
          size: The number of vertices in a vertex set.

        Returns:
          A new VertexSetSet object.

        See Also:
          smaller(), graph_size()
        """
        return VertexSetSet(self._ss.larger(size))

    def graph_size(self, size):
        """Returns a new VertexSetSet with `size` vertices.

        This method returns a new VertexSetSet with vertex sets that have
        `size` vertices.

        Examples:
          >>> vertex_set1 = [1]
          >>> vertex_set2 = [1, 2]
          >>> vertex_set3 = [1, 2, 3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2, vertex_set3])
          >>> vss.graph_size(2)
          VertexSetSet([['1', '2']])

        Args:
          size: The number of vertices in a vertex set.

        Returns:
          A new VertexSetSet object.

        See Also:
          smaller(), larger()

        """
        return VertexSetSet(self._ss.set_size(size))

    def complement(self):
        """Returns a new VertexSetSet with complement vertex sets of `self`.

        The `self` is not changed.

        Examples:
          >>> VertexSetSet.set_universe([1, 2])
          >>> vertex_set1 = [1]
          >>> vertex_set2 = [1, 2]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss.complement()
          VertexSetSet([[], ['2']])

        Returns:
          A new VertexSetSet object.
        """
        ss = self._ss.copy()
        for obj in VertexSetSet._obj2vertex:
            ss.flip(obj)
        return VertexSetSet(ss)

    def join(self, other):
        """Returns a new VertexSetSet of join between `self` and `other`.

        The join operation is defined by,
          vss1 \\sqcup vss2 = {a \\cup b | a \\in vss1 and b \\in vss2}.
        D. Knuth, Exercise 203, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1]
          >>> vertex_set2 = [1, 2]
          >>> vertex_set3 = [3]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set3])
          >>> vss1.join(vss2)
          VertexSetSet([['1', '3'], ['1', '2', '3']])

        Returns:
          A new VertexSetSet object.

        See Also:
          meet()
        """
        return VertexSetSet(self._ss.join(other._ss))

    def meet(self, other):
        """Returns a new VertexSetSet of meet between `self` and `other`.

        The meet operation is defined by,
          vss1 \\sqcap vss2 = {a \\cap b | a \\in vss1 and b \\in vss2}.
        D. Knuth, Exercise 203, The art of computer programming,
        Sect.7.1.4.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [1, 3]
          >>> vertex_set3 = [2, 3]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set3])
          >>> vss1.meet(vss2)
          VertexSetSet([['2'], ['3']])

        Returns:
          A new GraphSet object.

        See Also:
          join()
        """
        return VertexSetSet(self._ss.meet(other._ss))

    def subgraphs(self, other):
        """Returns a new VertexSetSet with subsets of a vertex set in `other`.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1]
          >>> vertex_set2 = [1, 2]
          >>> vertex_set3 = vertex_set1 + [3]
          >>> vertex_set4 = [2, 3]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set3, vertex_set4])
          >>> vss1.subgraphs(vss2)
          VertexSetSet([['1']])

        Returns:
          A new VertexSetSet object.

        See Also:
          supersets(), non_subsets()
        """
        return VertexSetSet(self._ss.subsets(other._ss))

    def supergraphs(self, other):
        """Returns a new VertexSetSet with supersets of a vertex set in `other`.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1, 3]
          >>> vertex_set2 = [2, 3]
          >>> vertex_set3 = [1]    # vertex_set1 - 3
          >>> vertex_set4 = [1, 2]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set3, vertex_set4])
          >>> vss1.supergraphs(vss2)
          VertexSetSet([['1', '3']])

        Returns:
          A new VertexSetSet object.

        See Also:
          subsets(), non_supersets()
        """
        return VertexSetSet(self._ss.supersets(other._ss))

    def non_subgraphs(self, other):
        """Returns a new VertexSetSet with vertex sets that aren't subsets of any graph in `other`.

        The `self` is not changed.

        The non_subsets are defined by,
          vss1.non_subsets(vss2) = {a \\in vss1 | b \\in vss2 -> a \\not\\subseteq b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        Examples:
          >>> vertex_set1 = [1]
          >>> vertex_set2 = [1, 2]
          >>> vertex_set3 = [1, 3]
          >>> vertex_set4 = [2, 3]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set3, vertex_set4])
          >>> vss1.non_subgraphs(vss2)
          VertexSetSet([['1', '2']])

        Returns:
          A new VertexSetSet object.

        See Also:
          non_supersets(), subsets()
        """
        return VertexSetSet(self._ss.non_subsets(other._ss))

    def non_supergraphs(self, other):
        """Returns a new VertexSetSet with graphs that aren't supersets of any graph in `other`.

        The `self` is not changed.

        The non_supersets are defined by,
          vss1.non_supersets(vss2) = {a \\in vss1 | b \\in vss2 -> a \\not\\superseteq b}.
        D. Knuth, Exercise 236, The art of computer programming,
        Sect.7.1.4.

        Examples:
          >>> vertex_set1 = [1, 3]
          >>> vertex_set2 = [2, 3]
          >>> vertex_set3 = [1]
          >>> vertex_set4 = [1, 2]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set3, vertex_set4])
          >>> vss1.non_supergraphs(vss2)
          VertexSetSet([['2', '3']])

        Returns:
          A new VertexSetSet object.

        See Also:
          non_subsets(), supersets()
        """
        return VertexSetSet(self._ss.non_supersets(other._ss))

    def including(self, obj):
        """Returns a new VertexSetSet that includes supersets of `obj`.

        Returns a new family of vertex sets that include `obj`, which can be a
        VertexSetSet, a vertex set, or a vertex.  If `obj` is a
        VertexSetSet, a vertex set returned includes *one of* vertex sets in the
        given VertexSetSet.

        The vertex sets stored in the new VertexSetSet are selected from `self`
        VertexSetSet.  The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vertex = 2
          >>> vss.including(vertex)
          VertexSetSet([['1', '2']])

        Args:
          obj: A VertexSetSet, a vertex set (a vertex list), or a vertex.

        Returns:
          A new VertexSetSet object.

        Raises:
          KeyError: If a given vertex is not found in the universe.

        See Also:
          excluding()
        """
        type, obj = VertexSetSet._conv_arg(obj)
        if type == "vertexsetset":
            return VertexSetSet(self._ss.supersets(obj._ss))
        elif type == "vertices":
            # originally the argument "obj" is a list of vertices,
            # and VertexSetSet._conv_arg(obj) converts it to a list of
            # "object," which consists of the edges in setset.universe.
            # but VertexSetSet.__init__ doesn't accept the list of objects,
            # so convert the list back to the original form.
            obj = VertexSetSet._conv_objs_to_vertices(obj)
            return self.including(VertexSetSet([obj]))
        elif type == "vertex":
            return VertexSetSet(self._ss.supersets(obj))
        else:
            return self.including(VertexSetSet([set([e]) for e in obj]))

    def excluding(self, obj):
        """Returns a new VertexSetSet that doesn't include `obj`.

        Returns a new family of vertex sets that don't include `obj`, which
        can be a VertexSetSet, a vertex set, or a vertex.  If `obj` is
        a VertexSetSet, a vertex set returned doesn't include *any of* vertex sets
        in the given VertexSetSet.

        The vertex sets stored in the new VertexSetSet are selected from `self`
        VertexSetSet.  The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vertex = 2
          >>> vss.excluding(vertex)
          VertexSetSet([['3']])

        Args:
          obj: A VertexSetSet, a vertex set (a vertex list), or a vertex.

        Returns:
          A new VertexSetSet object.

        Raises:
          KeyError: If a given vertex is not found in the universe.

        See Also:
          including()
        """
        type, obj = VertexSetSet._conv_arg(obj)
        if type == "vertexsetset":
            return self - self.including(obj)
        elif type == "vertices":
            # the same conversion as in including()
            obj = VertexSetSet._conv_objs_to_vertices(obj)
            return self.excluding(VertexSetSet([obj]))
        elif type == "vertex":
            return VertexSetSet(self._ss.non_supersets(obj))
        else:
            return self.excluding(VertexSetSet([set([e]) for e in obj]))

    def included(self, obj):
        """Returns a new VertexSetSet with subsets of a vertex set in `obj`.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1]
          >>> vertex_set2 = [1, 2]
          >>> vertex_set3 = vertex_set1 + [3]
          >>> vertex_set4 = [2, ]
          >>> vss1 = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss2 = VertexSetSet([vertex_set3, vertex_set4])
          >>> vss1.included(vss2)
          VertexSetSet([['1']])

        Args:
          obj: A VertexSetSet or a vertex set (a vertex list).

        Returns:
          A new VertexSetSet object.

        See Also:
          including()
        """
        type, obj = VertexSetSet._conv_arg(obj)
        if type == "vertexsetset":
            return VertexSetSet(self._ss.subsets(obj._ss))
        elif type == "vertices":
            # the same conversion as in including()
            obj = VertexSetSet._conv_objs_to_vertices(obj)
            return self.included(VertexSetSet([obj]))
        else:
            raise TypeError(obj)

    def choice(self):
        """Returns an arbitrary graph from `self`.

        The `self` is not changed.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> vss.choice()
          [1, 2]

        Returns:
          A vertex set.

        Raises:
          KeyError: If `self` is empty.

        See Also:
          pop()
        """
        return VertexSetSet._conv_objs_to_vertices(self._ss.choice())

    def probability(self, probabilities):
        """Returns the probability of `self` with vertex `probabilities`.

        This method calculates the probability of occurrence of any
        vertex set in `self` given `probabilities` of each vertex.

        Examples:
          >>> vertex_set1 = [1, 2]
          >>> vertex_set2 = [3]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2])
          >>> probabilities = {1: .9, 2: .8, 3: .7}
          >>> vss.probability(probabilities)
          0.23000000000000004

        Args:
          probabilities: A dictionary of probabilities of each vertex.

        Returns:
          Probability.

        Raises:
          AssertionError: If at least one vertex's probability is not set.
          KeyError: If a given vertex is not found in the universe.
        """
        probabilities = {VertexSetSet._vertex2obj[v]: p for v, p in viewitems(probabilities)}
        for obj in setset._int2obj[1: -VertexSetSet._vertex_num]:
            probabilities[obj] = 0
        return self._ss.probability(probabilities)

    def dump(self, fp):
        """Serialize `self` to a file `fp`.

        This method does not serialize the universe, which should be
        saved separately by pickle.

        Examples:
          >>> import pickle
          >>> fp = open('/path/to/vertexsetset', 'wb')
          >>> vss.dump(fp)
          >>> fp = open('/path/to/universe', 'wb')
          >>> pickle.dump(VertexSetSet.universe(), fp)

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
          >>> vertexsetset_str = vss.dumps()
          >>> universe_str = pickle.dumps(VertexSetSet.universe())

        See Also:
          dump(), loads()
        """
        return self._ss.dumps()

    def cost_le(self, costs, cost_bound):
        """Returns a new VertexSetSet with subsets whose cost is less than or equal to the cost bound.

        This method constructs a VertexSetSet of subsets in which each vertex set's
        cost is less than or equal to the cost bound
        given `costs` of each vertex and the `cost_bound`.

        Examples:
          >>> vertex_set1 = [1, 3]
          >>> vertex_set2 = [4]
          >>> vertex_set3 = [1, 2, 4]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2, vertex_set3])
          >>> costs = {1: 2, 2: 3, 3: 1, 4: 7}
          >>> cost_bound = 7
          >>> vss.cost_le(costs, cost_bound)
          VertexSetSet([['4'], ['1', '3']])

        Args:
          costs: A dictionary of cost of each vertex.
          cost_bound: The upper limit of cost of each graph. 32 bit signed integer.

        Returns:
          A new VertexSetSet object.

        Raises:
          KeyError: If a given vertex is not found in the universe.
          AssertionError: If the cost of at least one vertex is not given, or outside the range of 32 bit signed integer.
          TypeError: If at least one cost is not integer.

        """
        assert costs.keys() == VertexSetSet._vertex2obj.keys()
        costs = {VertexSetSet._vertex2obj[v]: c for v, c in viewitems(costs)}
        for obj in setset._int2obj[1: -VertexSetSet._vertex_num]:
            costs[obj] = 0
        le = self._ss.cost_le(costs, cost_bound)
        for obj in setset._int2obj[1: -VertexSetSet._vertex_num]:
            le = le.non_supersets(setset([[obj]]))
        return VertexSetSet(le)

    def cost_ge(self, costs, cost_bound):
        """Returns a new VertexSetSet with subsets whose cost is greater than or equal to the cost bound.

        This method constructs a VertexSetSet of subsets in which each graph's
        cost is greater than or equal to the cost bound
        given `costs` of each vertex and the `cost_bound`.

        Examples:
          >>> vertex_set1 = [1, 3]
          >>> vertex_set2 = [4]
          >>> vertex_set3 = [1, 2, 4]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2, vertex_set3])
          >>> costs = {1: 2, 2: 3, 3: 1, 4: 7}
          >>> cost_bound = 7
          >>> vss.cost_ge(costs, cost_bound)
          VertexSetSet([['4'], ['1', '2', '4']])

        Args:
          costs: A dictionary of cost of each vertex.
          cost_bound: The upper limit of cost of each vertex set. 32 bit signed integer.

        Returns:
          A new VertexSetSet object.

        Raises:
          KeyError: If a given vertex is not found in the universe.
          AssertionError: If the cost of at least one vertex is not given, or outside the range of 32 bit signed integer.
          TypeError: If at least one cost is not integer.

        """
        assert costs.keys() == VertexSetSet._vertex2obj.keys()
        inv_costs = {k: -v for k, v in costs.items()}
        return self.cost_le(inv_costs, -cost_bound)

    def cost_eq(self, costs, cost_bound):
        """Returns a new VertexSetSet with subsets whose cost is equal to the cost bound.

        This method constructs a VertexSetSet of subsets in which each graph's
        cost is equal to the cost bound
        given `costs` of each vertex and the `cost_bound`.

        Examples:
          >>> vertex_set1 = [1, 3]
          >>> vertex_set2 = [4]
          >>> vertex_set3 = [1, 2, 4]
          >>> vss = VertexSetSet([vertex_set1, vertex_set2, vertex_set3])
          >>> costs = {1: 2, 2: 3, 3: 1, 4: 7}
          >>> cost_bound = 7
          >>> vss.cost_eq(costs, cost_bound)
          VertexSetSet([['4']])

        Args:
          costs: A dictionary of cost of each vertex.
          cost_bound: The upper limit of cost of each vertex set. 32 bit signed integer.

        Returns:
          A new VertexSetSet object.

        Raises:
          KeyError: If a given vertex is not found in the universe.
          AssertionError: If the cost of at least one vertex is not given, or outside the range of 32 bit signed integer.
          TypeError: If at least one cost is not integer.

        """
        assert costs.keys() == VertexSetSet._vertex2obj.keys()
        return self.cost_le(costs, cost_bound) - self.cost_le(costs, cost_bound - 1)

    def remove_some_vertex(self):
        """Returns a new VertexSetSet with vertex sets that are obtained by removing some vertex from a vertex set in `self`.

        The `self` is not changed.

        Examples:
          >>> GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (1, 3)])
          >>> VertexSetSet.set_universe()
          >>> vs1 = [1, 2, 3]
          >>> vs2 = [2, 4]
          >>> vss = VertexSetSet([vs1, vs2])
          >>> vss.remove_some_vertex()
          VertexSetSet([[4], [2], [1, 2], [1, 3], [2, 3]])

        Returns:
          A new VertexSetSet object.
        """
        return VertexSetSet(self._ss.remove_some_element())

    def add_some_vertex(self):
        """Returns a new VertexSetSet with vertex sets that are obtained by adding some vertex to a vertex set in `self`.

        The `self` is not changed.

        Examples:
          >>> GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (1, 3)])
          >>> VertexSetSet.set_universe()
          >>> vs1 = [1, 2, 3]
          >>> vs2 = [2, 4]
          >>> vss = VertexSetSet([vs1, vs2])
          >>> vss.add_some_vertex()
          VertexSetSet([[1, 2, 4], [2, 3, 4], [1, 2, 3, 4]])

        Returns:
          A new VertexSetSet object.
        """
        return VertexSetSet(self._ss.add_some_element(len(VertexSetSet._universe_vertices)))

    def remove_add_some_vertices(self):
        """Returns a new VertexSetSet with vertex sets that are obtained by removing some vertex from a vertex set in `self` and adding another vertex to the vertex set.

        The `self` is not changed.

        Examples:
          >>> GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (1, 3)])
          >>> VertexSetSet.set_universe()
          >>> vs1 = [1, 2, 3]
          >>> vs2 = [2, 4]
          >>> vss = VertexSetSet([vs1, vs2])
          >>> vss.remove_add_some_vertices()
          VertexSetSet([[1, 4], [1, 2], [3, 4], [2, 3], [1, 2, 4], [1, 3, 4], [2, 3, 4]])

        Returns:
          A new VertexSetSet object.
        """
        return VertexSetSet(self._ss.remove_add_some_elements(len(VertexSetSet._universe_vertices)))

    @staticmethod
    def load(fp):
        """Deserialize a file `fp` to `self`.

        This method does not deserialize the universe, which should be
        loaded separately by pickle.
        The universe of GraphSet does not need to be the same as the universe
        at the time of `fp` is written.

        Args:
          fp: A read-supporting file-like object.

        Examples:
          >>> import pickle
          >>> GraphSet.set_universe(some_universe)
          >>> fp = open('/path/to/universe', 'rb')
          >>> VertexSetSet.set_universe(pickle.load(fp))
          >>> fp = open('/path/to/vertexsetset', 'rb)
          >>> vss = VertexSetSet.load(fp)

        See Also:
          loads(), dump()
        """
        return VertexSetSet(setset.load(fp))

    @staticmethod
    def loads(fp):
        """Deserialize `s` to `self`.

        This method does not deserialize the universe, which should be
        loaded separately by pickle.
        The universe of GraphSet does not need to be the same as the universe
        at the time of `s` is defined.

        Args:
          s: A string instance.

        Examples:
          >>> import pickle
          >>> GraphSet.set_universe(some_universe)
          >>> VertexSetSet.set_universe(pickle.loads(universe_str))
          >>> vss = VertexSetSet.load(vertexsetset_str)

        See Also:
          load(), dumps()
        """
        return VertexSetSet(setset.loads(fp))

    @staticmethod
    # TODO: Add optional arguments "traversal" and "source" as in GraphSet class
    def set_universe(universe=None):
        """Registers the new universe.

        Examples:
          >>> # This example sets the universe as a vertex set {1, 2, 3},
          >>> # with the vertex 3 having a weight of 10.
          >>> VertexSetSet.set_universe([1, (2), (3, 10)])

        Args:
          universe: A list of vertices that represents the new universe.
            A vertex may come along with a vertex weight, which can be
            positive as well as negative (or 1.0 if not specified).
            If a vertex is weighted, it must be given in a list as a tuple of vertex and weight.
            If a vertex is not weighted, it must be given as a tuple of size one or single value.

        See Also:
          universe()
        """
        from graphillion import GraphSet
        universe_edges = GraphSet.universe()
        assert universe_edges != None and len(universe_edges) > 0, \
                "GraphSet.set_universe must be called."

        if universe is None: # adopt the vertex set of GraphSet's underlying graph
            universe = [eval(v) for v in setset.get_vertices_from_top()]
            assert len(universe) <= len(universe_edges), \
                  "The number of edges of the universe graph must be " \
                  + "larger than or equal to the number of vertices " \
                  + "of the universe graph."
        else:
            assert len(universe) <= len(universe_edges), \
                  "The size of universe must be " \
                  + "smaller than or equal to the number of edges " \
                  + "of the universe graph."

        vertex_num = len(universe)
        if vertex_num != len(set(universe)):
            raise ValueError("duplicated elements found")

        VertexSetSet._universe_vertices = []
        VertexSetSet._vertex2id = {}
        VertexSetSet._vertex2obj = {}
        VertexSetSet._obj2vertex = {}
        VertexSetSet._obj2str = {}
        VertexSetSet._obj2weight = {}
        low_level_objs = setset._int2obj[-vertex_num:]
        for i, vertex in enumerate(universe):
            if isinstance(vertex, tuple):
                if len(vertex) == 2:
                    vertex, weight = vertex
                    VertexSetSet._obj2weight[low_level_objs[i]] = weight
                elif len(vertex) == 1:
                    vertex = vertex[0]
                else:
                    raise TypeError(vertex)
            VertexSetSet._universe_vertices.append(vertex)
            VertexSetSet._vertex2id[vertex] = i
            VertexSetSet._vertex2obj[vertex] = low_level_objs[i]
            VertexSetSet._obj2vertex[low_level_objs[i]] = vertex
            VertexSetSet._obj2str[low_level_objs[i]] = repr(vertex)
        VertexSetSet._vertex_num = len(VertexSetSet._universe_vertices)

    @staticmethod
    def universe():
        """Returns the current universe.

        The list of vertices that represents the current universe is
        returned.

        Examples:
          >>> VertexSetSet.universe()
          [1, 2, (3, 10)]

        Returns:
          The universe of VertexSetSet class.

        See Also:
          set_universe()
        """
        vertices = []
        for v in VertexSetSet._universe_vertices:
            obj = VertexSetSet._vertex2obj[v]
            if obj in VertexSetSet._obj2weight:
                vertices.append((v, VertexSetSet._obj2weight[obj]))
            else:
                vertices.append(v)
        return vertices

    @staticmethod
    def independent_sets(edges, distance=1):
        '''Returns the family of independent sets.

        Examples:
        >>> VertexSetSet.set_universe([1, 2, 3, 4])
        >>> edges = [(1, 2), (2, 3), (3, 4), (4, 1), (1, 3)]
        >>> vss = VertexSetSet.independent_sets(edges)
        >>> vss
        VertexSetSet([[], ['1'], ['2'], ['3'], ['4'], ['2', '4']])

        Args:
          edges: edges of the graph
          distance: value of k in distance-k independent set problem

        Returns:
          A new VertexSetSet object.

        Raises:
          ValueError: if edge's name or format is wrong.
        '''
        for edge in edges:
            if len(edge) != 2:
                raise ValueError(f"invalid edge format: {edge}")
            if edge[0] not in VertexSetSet._universe_vertices \
               or edge[1] not in VertexSetSet._universe_vertices:
                raise ValueError(f"invalid vertex in edge {edge}")

        underlying_graph = {v: [] for v in VertexSetSet._universe_vertices}
        for u, v in edges:
            underlying_graph[u].append(v)
            underlying_graph[v].append(u)

        if distance > 1:
            vertices_within_k = {v: [] for v in VertexSetSet._universe_vertices}
            for v in VertexSetSet._universe_vertices:
                dist = {v: -1 for v in VertexSetSet._universe_vertices}
                dist[v] = 0
                qu = deque()
                for adj in underlying_graph[v]:
                    dist[adj] = 1
                    vertices_within_k[v].append(adj)
                    qu.append(adj)
                while len(qu) > 0:
                    now = qu.popleft()
                    for adj in underlying_graph[now]:
                        if dist[adj] != -1: continue
                        dist[adj] = dist[now] + 1
                        vertices_within_k[v].append(adj)
                        if (dist[adj] < distance):
                            qu.append(adj)
            underlying_graph = vertices_within_k

        p = VertexSetSet({})
        f = p.copy()
        for v in VertexSetSet._universe_vertices:
            for u in underlying_graph[v]:
                assert v != u
                if hash(v) > hash(u): continue
                u0 = p.non_supergraphs(VertexSetSet([[u]]))
                v0 = p.non_supergraphs(VertexSetSet([[v]]))
                f &= u0 | v0

        return f

    @staticmethod
    def dominating_sets(edges, distance=1):
        '''Returns the family of dominating sets.

        Examples:
        >>> VertexSetSet.set_universe([1, 2, 3, 4])
        >>> edges = [(1, 2), (2, 3), (3, 4), (4, 1), (1, 3)]
        >>> vss = VertexSetSet.dominating_sets(edges)
        >>> vss.minimal()
        VertexSetSet([['1'], ['3'], ['2', '4']])

        Args:
          edges: edges of the graph
          distance: value of k in distance-k dominating set problem

        Returns:
          A new VertexSetSet object.

        Raises:
          ValueError: if edge's name or format is wrong.
        '''
        for edge in edges:
            if len(edge) != 2:
                raise ValueError(f"invalid edge format: {edge}")
            if edge[0] not in VertexSetSet._universe_vertices \
               or edge[1] not in VertexSetSet._universe_vertices:
                raise ValueError(f"invalid vertex in edge {edge}")

        underlying_graph = {v: [] for v in VertexSetSet._universe_vertices}
        for u, v in edges:
            underlying_graph[u].append(v)
            underlying_graph[v].append(u)

        if distance > 1:
            vertices_within_k = {v: [] for v in VertexSetSet._universe_vertices}
            for v in VertexSetSet._universe_vertices:
                dist = {v: -1 for v in VertexSetSet._universe_vertices}
                dist[v] = 0
                qu = deque()
                for adj in underlying_graph[v]:
                    dist[adj] = 1
                    vertices_within_k[v].append(adj)
                    qu.append(adj)
                while len(qu) > 0:
                    now = qu.popleft()
                    for adj in underlying_graph[now]:
                        if dist[adj] != -1: continue
                        dist[adj] = dist[now] + 1
                        vertices_within_k[v].append(adj)
                        if (dist[adj] < distance):
                            qu.append(adj)
            underlying_graph = vertices_within_k

        p = VertexSetSet({})
        f = p.copy()
        for v in VertexSetSet._universe_vertices:
            g = p.supergraphs(VertexSetSet([[v]]))
            for adj in underlying_graph[v]:
                g |= p.supergraphs(VertexSetSet([[adj]]))
            f &= g

        return f

    @staticmethod
    def vertex_covers(edges):
        '''Returns the family of vertex covers.

        Examples:
        >>> VertexSetSet.set_universe([1, 2, 3, 4, 5])
        >>> edges = [(1, 2), (2, 3), (3, 4), (4, 1), (1, 3), (3, 5)]
        >>> vss = VertexSetSet.vertex_covers(edges)
        >>> vss.minimal()
        VertexSetSet([['1', '3'], ['2', '3', '4'], ['1', '2', '4', '5']])

        Args:
          edges: edges of the graph

        Returns:
          A new VertexSetSet object.

        Raises:
          ValueError: if edge's name or format is wrong.
        '''
        for edge in edges:
            if len(edge) != 2:
                raise ValueError(f"invalid edge format: {edge}")
            if edge[0] not in VertexSetSet._universe_vertices \
               or edge[1] not in VertexSetSet._universe_vertices:
                raise ValueError(f"invalid vertex in edge {edge}")

        return VertexSetSet.independent_sets(edges).complement()

    @staticmethod
    def cliques(edges):
        '''Returns the family of vertex covers.

        Examples:
        >>> VertexSetSet.set_universe([1, 2, 3, 4, 5])
        >>> edges = [(1, 2), (2, 3), (3, 4), (4, 1), (1, 3), (3, 5)]
        >>> vss = VertexSetSet.vertex_covers(edges)
        >>> vss.minimal()
        VertexSetSet([['1', '3'], ['2', '3', '4'], ['1', '2', '4', '5']])

        Args:
          edges: edges of the graph

        Returns:
          A new VertexSetSet object.

        Raises:
          ValueError: if edge's name or format is wrong.
        '''
        for edge in edges:
            if len(edge) != 2:
                raise ValueError(f"invalid edge format: {edge}")
            if edge[0] not in VertexSetSet._universe_vertices \
               or edge[1] not in VertexSetSet._universe_vertices:
                raise ValueError(f"invalid vertex in edge {edge}")

        edges = set(edges)
        complement_edges = []
        for u in VertexSetSet._universe_vertices:
            for v in VertexSetSet._universe_vertices:
                if u != v and (u, v) not in edges and (v, u) not in edges:
                    complement_edges.append((u, v))

        return VertexSetSet.independent_sets(complement_edges)

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
    def _conv_objs_to_vertices(objs):
        return [VertexSetSet._obj2vertex[obj] for obj in objs]

    @staticmethod
    def _conv_vertices_to_objs(vertices):
        return set(VertexSetSet._vertex2obj[vertex] for vertex in vertices)

    @staticmethod
    # NOTE: need to support further types as in GraphSet._conv_arg()
    def _conv_arg(obj):
        if isinstance(obj, VertexSetSet):
            return "vertexsetset", obj
        elif isinstance(obj, (set, frozenset, list)):
            return "vertices", VertexSetSet._conv_vertices_to_objs(obj)
        elif obj in VertexSetSet._vertex2obj:
            return "vertex", VertexSetSet._vertex2obj[obj]
        raise KeyError(obj)

    @staticmethod
    def _sort_vertices(obj):
        return (sorted(list(obj), key=lambda x : VertexSetSet._vertex2id[x]))

    _universe_vertices = [] # TODO: consider that is should be a list or set.
    _vertex2id = {}
    _vertex2obj = {}
    _obj2vertex = {}
    _obj2str = {}
    _obj2weight = {}
    _vertex_num = 0
