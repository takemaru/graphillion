from future.utils import viewitems

from graphillion import GraphSet
from graphillion import setset

class VertexSetSet(object):
    # TODO: 引数に入れられる値の種類を増やす
    def __init__(self, vertex_setset_or_constraints=None):
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
                            raise ValueError("invalid vertex:", vertex)

                l = []
                for vertices in obj:
                    l.append([VertexSetSet._vertex2obj[vertex] for vertex in vertices])
                obj = l
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
                yield VertexSetSet._conv_objs_to_vertices(objs)
            except StopIteration:
                return

    def rand_iter(self):
        for objs in self._ss.rand_iter():
            try:
                yield VertexSetSet._conv_objs_to_vertices(objs)
            except StopIteration:
                return

    def min_iter(self, weights=None):
        if weights is None:
            weights = VertexSetSet._weights
        weights = {VertexSetSet._vertex2obj[vertex]: weight for vertex, weight in weights.items()}
        for objs in self._ss.min_iter(weights):
            try:
                yield VertexSetSet._conv_objs_to_vertices(objs)
            except StopIteration:
                return

    def max_iter(self, weights=None):
        if weights is None:
            weights = VertexSetSet._weights
        weights = {VertexSetSet._vertex2obj[vertex]: weight for vertex, weight in weights.items()}
        for objs in self._ss.max_iter(weights):
            try:
                yield VertexSetSet._conv_objs_to_vertices(objs)
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
        raise TypeError(obj)

    def remove(self, obj):
        type, obj = VertexSetSet._conv_arg(obj)
        if type == "vertices" or type == "vertex":
            self._ss.remove(obj)
        raise TypeError(obj)

    def discard(self, obj):
        type, obj = VertexSetSet._conv_arg(obj)
        if type == "vertices" or type == "vertex":
            self._ss.discard(obj)
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
            return self.excluding(VertexSetSet([obj]))
        elif type == "vertex":
            return VertexSetSet(self._ss.non_supersets(obj))
        else:
            return self.excluding(VertexSetSet([set([e]) for e in obj]))

    def included(self, obj):
        type, obj = VertexSetSet._conv_arg(obj)
        if type == "vertexsetset":
            return VertexSetSet(self._ss.subsets(obj.ss))
        elif type == "graph":
            return self.included(VertexSetSet([obj]))
        else:
            raise TypeError(obj)

    def choice(self):
        return VertexSetSet._conv_objs_to_vertices(self._ss.choice())

    def probability(self, probabilities):
        probabilities = {VertexSetSet._vertex2obj[v]: p for v, p in probabilities.viewitems()}
        return self._ss.probability(probabilities)

    def dump(self, fp):
        return self._ss.dump(fp)

    def dumps(self):
        return self._ss.dumps()

    def cost_le(self, costs, cost_bound):
        assert costs.keys() == VertexSetSet._vertex2obj.keys()
        costs = {VertexSetSet._vertex2obj[v]: c for v, c in costs.viewitems()}
        for obj in setset._int2obj[len(VertexSetSet._universe_vertices) + 1:]:
            costs[obj] = 0
        return VertexSetSet(self._ss.cost_le(costs, cost_bound))

    def cost_ge(self, costs, cost_bound):
        assert costs.keys() == VertexSetSet._vertex2obj.keys()
        inv_costs = {VertexSetSet._vertex2obj[v]: -c for v, c in costs.viewitems()}
        for obj in setset._int2obj[len(VertexSetSet._universe_vertices) + 1:]:
            inv_costs[obj] = 0
        return VertexSetSet(self._ss.cost_le(costs=inv_costs, cost_bound=-cost_bound))

    # TODO: rename the argument as their names are almost the same
    def cost_eq(self, costs, cost):
        assert costs.keys() == VertexSetSet._vertex2obj.keys()
        costs = {VertexSetSet._vertex2obj[v]: c for v, c in costs.viewitems()}
        for obj in setset._int2obj[len(VertexSetSet._universe_vertices) + 1:]:
            costs[obj] = 0
        le_ss = self._ss.cost_le(costs=costs, cost_bound=cost)
        lt_ss = self._ss.cost_le(costs=costs, cost_bound=cost - 1)
        return GraphSet(le_ss.difference(lt_ss))

    @staticmethod
    # TODO: _weightsも設定できるようにする
    def set_universe(vertices=None):
        if vertices is None: # adopt the vertex set of GraphSet's underlying graph
            # TODO: 実装する
            return

        assert len(vertices) <= len(setset.universe()), \
               f"the universe is too small, which has at least {len(vertices)} elements"

        vertex_num = len(vertices)
        if vertex_num != len(set(vertices)):
            raise ValueError("duplicated elements found")

        VertexSetSet._universe_vertices = set(vertices)

        low_level_objs = setset._int2obj[:vertex_num + 1]
        # TODO: 内包表記に書き換え
        for i, vertex in enumerate(vertices):
            VertexSetSet._vertex2obj[vertex] = low_level_objs[i + 1]
            VertexSetSet._obj2vertex[low_level_objs[i + 1]] = vertex
            VertexSetSet._obj2str[low_level_objs[i + 1]] = str(vertex)

        # VertexSetSet._universe_vertices = GraphSet._vertices
        # vertex_num = len(VertexSetSet._universe_vertices)


        # VertexSetSet._edges = GraphSet.universe()

        # VertexSetSet._int2edge = setset._int2obj[:vertex_num + 1]
        # vertex_iter = iter(VertexSetSet._universe_vertices)
        # for edge in VertexSetSet._int2edge[1:]:
        #     VertexSetSet._edge2vertex[edge] = next(vertex_iter)

    _universe_vertices = set() # TODO: listかsetか考える
    _vertex2obj = {}
    _obj2vertex = {}
    _obj2str = {}
    _weights = {}

    @staticmethod
    def _conv_objs_to_vertices(objs):
        return [VertexSetSet._obj2vertex[obj] for obj in objs]

    @staticmethod
    def _conv_vertices_to_objs(vertices):
        return [VertexSetSet._vertex2obj[vertex] for vertex in vertices]

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