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

from future.utils import viewitems

import _graphillion
from graphillion import setset

class VertexSetSet(object):
    # ? Is "universal vertices" a meaningful expression?
    """Represents and manipulates a family of vertex sets.

    A VertexSetSet object stores a family of vertex sets.  A set of
    vertices stored must be a subset of the universal vertices, and is
    represented by a list of vertices in the universal vertices.
    A vertex can be any hashable object like a number, a text string, and a tuple.

    The universal vertices must be defined before creating VertexSetSet
    objects by `VertexSetSet.set_universe()` method.
    Furthermore, for a technical reason, `GraphSet.set_universe()` or
    `setset.set_universe()` must be called before the first call of
    `VertexSetSet.set_universet()` and the size of the universe of GraphSet
    or setset must not be smaller than that of the universe of VertexSetSet.
    By the limitation above, currently a forest graph and its vertex set
    as the universal graph and the univarsal vertices simultaneously.

    Like Python set types, VertexSetSet supports `graph in vertexsetset`,
    `len(vertexsetset)`, and `for graph in vertexsetset`.  It also supports
    all set methods and operators,
    * isdisjoint(), issubset(), issuperset(), union(), intersection(),
      difference(), symmetric_difference(), copy(), update(),
      intersection_update(), difference_update(),
      symmetric_difference_update(), add(), remove(), discard(),
      pop(), clear(),
    * ==, !=, <=, <, >=, >, |, &, -, ^, |=, &=, -=, ^=.

    Examples:
      >>> from graphillion import GraphSet, VertexSetSet

      We assume the set {1, 2, 3, 4, 5} as the universal vertices.
      As shown below, the universe graph of the GraphSet does not have to do
      with the universal vertices, and what only matters is their size.

      >>> dummy_universe = [("foo", "bar"), ("foo", "baz"), ("foo", "foobar"),
                            ("bar", "baz"), ("bar", "foobar")]
      >>> GraphSet.set_universe(dummy_universe)
      >>> universe = [1, 2, 3, 4, 5]
      >>> VertexSetSet.set_universe(universe)

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
    # TODO: 引数に入れられる値の種類を増やす
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
            graphs are stored in the new object).

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
                    d[k] = VertexSetSet._conv_vertices_to_objs(l)
                obj = d
            self._ss = setset(obj)

    def copy(self):
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
        return VertexSetSet(self._ss.union(*[vss._ss for vss in others]))

    def intersection(self, *others):
        return VertexSetSet(self._ss.intersection(*[vss._ss for vss in others]))

    def difference(self, *others):
        return VertexSetSet(self._ss.difference(*[vss._ss for vss in others]))

    def symmetric_difference(self, *others):
        return VertexSetSet(self._ss.symmetric_difference(*[vss._ss for vss in others]))

    def quotient(self, other):
        return VertexSetSet(self._ss.quotient(other._ss))

    def remainder(self, other):
        return VertexSetSet(self._ss.remainder(other._ss))

    def update(self, *others):
        self._ss.update(*[vss._ss for vss in others])
        return self

    def intersection_update(self, *others):
        self._ss.intersection_update(*[vss._ss for vss in others])
        return self

    def difference_update(self, *others):
        self._ss.difference_update(*[vss._ss for vss in others])
        return self

    def symmetric_difference_update(self, *others):
        self._ss.symmetric_difference_update(*[vss._ss for vss in others])
        return self

    def quotient_update(self, other):
        self._ss.quotient_update(other._ss)
        return self

    def remainder_update(self, other):
        self._ss.remainder_update(other._ss)
        return self

    def __invert__(self):
        invert_ss = ~self._ss
        for obj in setset._int2obj[len(VertexSetSet._universe_vertices) + 1:]:
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
        return self._ss.isdisjoint(other._ss)

    def issubset(self, other):
        return self._ss.issubset(other._ss)

    def issuperset(self, other):
        return self._ss.issuperset(other._ss)

    __le__ = issubset
    __ge__ = issuperset

    def __lt__(self, other):
        return self._ss < other._ss

    def __gt__(self, other):
        return self._ss > other._ss

    def __eq__(self, other):
        return self._ss == other._ss

    def __ne__(self, other):
        return self._ss != other._ss

    def __len__(self):
        return len(self._ss)

    def len(self, size=None):
        if size is None:
            return self._ss.len()
        else:
            return self.graph_size(size)

    def __iter__(self):
        for objs in self._ss.__iter__():
            try:
                yield VertexSetSet._sort_vertices(VertexSetSet._conv_objs_to_vertices(objs))
            except StopIteration:
                return

    def rand_iter(self):
        for objs in self._ss.rand_iter():
            try:
                yield VertexSetSet._sort_vertices(VertexSetSet._conv_objs_to_vertices(objs))
            except StopIteration:
                return

    def min_iter(self, weights=None):
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
        type, obj = VertexSetSet._conv_arg(obj)
        if type == "vertex" or type == "vertices":
            return obj in self._ss
        raise TypeError(obj)

    def add(self, vertices_or_vertex):
        type, obj = VertexSetSet._conv_arg(vertices_or_vertex)
        if type == "vertices" or type == "vertex":
            self._ss.add(obj)
        else:
            raise TypeError(obj)

    def remove(self, obj):
        type, obj = VertexSetSet._conv_arg(obj)
        if type == "vertices" or type == "vertex":
            self._ss.remove(obj)
        else:
            raise TypeError(obj)

    def discard(self, obj):
        type, obj = VertexSetSet._conv_arg(obj)
        if type == "vertices" or type == "vertex":
            self._ss.discard(obj)
        else:
            raise TypeError(obj)

    def pop(self):
        return VertexSetSet._conv_objs_to_vertices(self._ss.pop())

    def clear(self):
        return self._ss.clear()

    def flip(self, vertices):
        type, obj = VertexSetSet._conv_arg(vertices)
        if type == "vertex":
            self._ss.flip(obj)
        else:
            raise TypeError(vertices)

    def minimal(self):
        return VertexSetSet(self._ss.minimal())

    def maximal(self):
        return VertexSetSet(self._ss.maximal())

    def blocking(self):
        return VertexSetSet(self._ss.hitting())

    hitting = blocking

    def smaller(self, size):
        return VertexSetSet(self._ss.smaller(size))

    def larger(self, size):
        return VertexSetSet(self._ss.larger(size))

    def graph_size(self, size):
        return VertexSetSet(self._ss.set_size(size))

    def complement(self):
        ss = self._ss.copy()
        for obj in VertexSetSet._obj2vertex:
            ss.flip(obj)
        return VertexSetSet(ss)

    def join(self, other):
        return VertexSetSet(self._ss.join(other._ss))

    def meet(self, other):
        return VertexSetSet(self._ss.meet(other._ss))

    def subgraphs(self, other):
        return VertexSetSet(self._ss.subsets(other._ss))

    def supergraphs(self, other):
        return VertexSetSet(self._ss.supersets(other._ss))

    def non_subgraphs(self, other):
        return VertexSetSet(self._ss.non_subsets(other._ss))

    def non_supergraphs(self, other):
        return VertexSetSet(self._ss.non_supersets(other._ss))

    def including(self, obj):
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
        return VertexSetSet._conv_objs_to_vertices(self._ss.choice())

    def probability(self, probabilities):
        probabilities = {VertexSetSet._vertex2obj[v]: p for v, p in viewitems(probabilities)}
        return self._ss.probability(probabilities)

    def dump(self, fp):
        return self._ss.dump(fp)

    def dumps(self):
        return self._ss.dumps()

    def cost_le(self, costs, cost_bound):
        assert costs.keys() == VertexSetSet._vertex2obj.keys()
        costs = {VertexSetSet._vertex2obj[v]: c for v, c in viewitems(costs)}
        for obj in setset._int2obj[len(VertexSetSet._universe_vertices) + 1:]:
            costs[obj] = 0
        return VertexSetSet(self._ss.cost_le(costs, cost_bound))

    def cost_ge(self, costs, cost_bound):
        assert costs.keys() == VertexSetSet._vertex2obj.keys()
        inv_costs = {VertexSetSet._vertex2obj[v]: -c for v, c in viewitems(costs)}
        for obj in setset._int2obj[len(VertexSetSet._universe_vertices) + 1:]:
            inv_costs[obj] = 0
        return VertexSetSet(self._ss.cost_le(costs=inv_costs, cost_bound=-cost_bound))

    # TODO: rename the argument as their names are almost the same
    def cost_eq(self, costs, cost):
        assert costs.keys() == VertexSetSet._vertex2obj.keys()
        costs = {VertexSetSet._vertex2obj[v]: c for v, c in viewitems(costs)}
        for obj in setset._int2obj[len(VertexSetSet._universe_vertices) + 1:]:
            costs[obj] = 0
        le_ss = self._ss.cost_le(costs=costs, cost_bound=cost)
        lt_ss = self._ss.cost_le(costs=costs, cost_bound=cost - 1)
        return VertexSetSet(le_ss.difference(lt_ss))

    @staticmethod
    def load(fp):
        return VertexSetSet(setset.load(fp))

    @staticmethod
    def loads(fp):
        return VertexSetSet(setset.loads(fp))

    @staticmethod
    # TODO: Add optional erguments "traversal" and "source" as in GraphSet class
    def set_universe(vertices=None):
        if vertices is None: # adopt the vertex set of GraphSet's underlying graph
            # TODO: 実装する
            return

        assert len(vertices) <= len(setset.universe()), \
               f"the universe is too small, which has at least {len(vertices)} elements"

        vertex_num = len(vertices)
        if vertex_num != len(set(vertices)):
            raise ValueError("duplicated elements found")

        VertexSetSet._universe_vertices = []
        VertexSetSet._vertex2id = {}
        VertexSetSet._vertex2obj = {}
        VertexSetSet._obj2vertex = {}
        VertexSetSet._obj2str = {}
        VertexSetSet._obj2weight = {}
        low_level_objs = setset._int2obj[:vertex_num + 1]
        for i, vertex in enumerate(vertices):
            if isinstance(vertex, tuple):
                if len(vertex) == 2:
                    vertex, weight = vertex
                    VertexSetSet._obj2weight[low_level_objs[i + 1]] = weight
                elif len(vertex) == 1:
                    vertex = vertex[0]
                else:
                    raise TypeError(vertex)
            VertexSetSet._universe_vertices.append(vertex)
            VertexSetSet._vertex2id[vertex] = i
            VertexSetSet._vertex2obj[vertex] = low_level_objs[i + 1]
            VertexSetSet._obj2vertex[low_level_objs[i + 1]] = vertex
            VertexSetSet._obj2str[low_level_objs[i + 1]] = str(vertex)

    @staticmethod
    def universe():
        vertices = []
        for v in VertexSetSet._universe_vertices:
            obj = VertexSetSet._vertex2obj[v]
            if obj in VertexSetSet._obj2weight:
                vertices.append((v, VertexSetSet._obj2weight[obj]))
            else:
                vertices.append(v)
        return vertices

    @staticmethod
    def show_messages(flag=True):
        return _graphillion._show_messages(flag)

    @staticmethod
    def _conv_objs_to_vertices(objs):
        return [VertexSetSet._obj2vertex[obj] for obj in objs]

    @staticmethod
    def _conv_vertices_to_objs(vertices):
        return set(VertexSetSet._vertex2obj[vertex] for vertex in vertices)

    @staticmethod
    # NOTE: GraphSet._conv_arg()よりは機能が少ない
    # NOTE: 必要に応じて拡張
    def _conv_arg(obj):
        if isinstance(obj, VertexSetSet):
            return "vertexsetset", obj
        elif isinstance(obj, list):
            return "vertices", VertexSetSet._conv_vertices_to_objs(obj)
        elif obj in VertexSetSet._vertex2obj:
            return "vertex", VertexSetSet._vertex2obj[obj]
        raise KeyError(obj)

    @staticmethod
    def _sort_vertices(obj):
        return (sorted(list(obj), key=lambda x : VertexSetSet._vertex2id[x]))

    _universe_vertices = [] # TODO: listかsetか考える
    _vertex2id = {}
    _vertex2obj = {}
    _obj2vertex = {}
    _obj2str = {}
    _obj2weight = {}