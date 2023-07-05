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

    def graph_size(self, size):
        return VertexSetSet(self._ss.set_size(size))

    @staticmethod
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

    # _edges = set()
    # _edge2vertex = {}
    # _edge2int = {}
    # _int2edge = [None]

    @staticmethod
    def _conv_objs_to_vertices(obj_list):
        return [VertexSetSet._obj2vertex[obj] for obj in obj_list]