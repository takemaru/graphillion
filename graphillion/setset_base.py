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

"""Base class for setset, GraphSet, VertexSetSet, and so on.
"""

from builtins import range
import _graphillion


class setset_base(_graphillion.setset):
    """
    The setset_base class is the base class of GraphSet,
    VertexSetset, and so on. This class is not directly
    intended to be used by users.
    """

    def __init__(self, objtable, setset_or_constraints=None):
        obj = setset_or_constraints
        if obj is None:
            obj = []
        elif isinstance(obj, list):  # a set of sets [set+]
            l = []
            for s in obj:
                l.append(set([objtable.conv_elem(e) for e in s]))
            obj = l
        elif isinstance(obj, dict):  # constraints
            d = {}
            for k, l in obj.items():
                d[k] = [objtable.conv_elem(e) for e in l]
            obj = d
        if objtable is None:
            num_elems = 0
        else:
            num_elems = objtable.num_elems()
        _graphillion.setset.__init__(self, obj, num_elems)

    def __repr__(self, objtable):
        name = self.__class__.__name__
        return setset_base._repr(self, objtable, (name + '([', '])'), ('set([', '])'))

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
        for s in setset_base.min_iter(self, objtable, w):
            if no_comma:
                no_comma = False
            else:
                ret += ', '
            if obj_to_str is None:
                ret += inner_braces[0] + str(sorted(list(s)))[1:-1] + inner_braces[1]
            else:
                ret += inner_braces[0] + str(sorted([eval(obj_to_str[tuple(obj)]) for obj in s]))[1:-1] + inner_braces[1]
            if len(ret) > maxchar - 2:
                break
        if len(ret) <= maxchar - 2:
            return ret + outer_braces[1]
        else:
            return ret[:(maxchar - 4)] + ' ...'

    def _invert(self, objtable):
        return _graphillion.setset.complement(self, objtable.num_elems())

    def _contains(self, objtable, set_or_elem):
        set_or_elem = objtable.conv_arg(set_or_elem)
        return _graphillion.setset.__contains__(self, set_or_elem)

    def add(self, objtable, set_or_elem):
        set_or_elem = objtable.conv_arg(set_or_elem)
        return _graphillion.setset.add(self, set_or_elem)

    def remove(self, objtable, set_or_elem):
        set_or_elem = objtable.conv_arg(set_or_elem)
        return _graphillion.setset.remove(self, set_or_elem)

    def discard(self, objtable, set_or_elem):
        set_or_elem = objtable.conv_arg(set_or_elem)
        return _graphillion.setset.discard(self, set_or_elem)

    def pop(self, objtable):
        set = _graphillion.setset.pop(self)
        return objtable.conv_ret(set)

    def hitting(self, objtable):
        return _graphillion.setset.hitting(self, objtable.num_elems())

    def flip(self, objtable, elem=None):
        if elem is not None:
            elem = objtable.conv_elem(elem)
            return _graphillion.setset.flip(self, elem)
        else:
            return _graphillion.setset.flip_all(self, objtable.num_elems())

    def _iter(self, objtable):
        i = _graphillion.setset.iter(self)
        while (True):
            try:
                yield objtable.conv_ret(next(i))
            except StopIteration:
                return

    def rand_iter(self, objtable):
        i = _graphillion.setset.rand_iter(self)
        while (True):
            try:
                yield objtable.conv_ret(next(i))
            except StopIteration:
                return

    def min_iter(self, objtable, weights=None, default=1):
        return self._optimize(objtable, weights, default, _graphillion.setset.min_iter)

    def max_iter(self, objtable, weights=None, default=1):
        return self._optimize(objtable, weights, default, _graphillion.setset.max_iter)

    def _optimize(self, objtable, weights, default, generator):
        ws = [default] * (objtable.num_elems() + 1)
        if weights:
            universe = objtable.universe()
            for e, w in weights.items():
                if e in universe:
                    i = objtable.obj2int[e]
                    ws[i] = w
        i = generator(self, ws)
        while (True):
            try:
                yield objtable.conv_ret(next(i))
            except StopIteration:
                return

    def supersets(self, objtable, obj):
        if (not isinstance(obj, setset_base)):
            obj = objtable.conv_elem(obj)
        return _graphillion.setset.supersets(self, obj)

    def non_supersets(self, objtable, obj):
        if (not isinstance(obj, setset_base)):
            obj = objtable.conv_elem(obj)
        return _graphillion.setset.non_supersets(self, obj)

    def choice(self, objtable):
        set = _graphillion.setset.choice(self)
        return objtable.conv_ret(set)

    def probability(self, objtable, probabilities):
        ps = [-1] * (objtable.num_elems() + 1)
        for e, p in probabilities.items():
            i = objtable.obj2int[e]
            ps[i] = p
        assert len([p for p in ps[1:] if p < 0 or 1 < p]) == 0
        return _graphillion.setset.probability(self, objtable.num_elems(), ps)

    def cost_le(self, objtable, costs, cost_bound):
        cs = [-1] * (objtable.num_elems() + 1)
        for e, c in costs.items():
            i = objtable.obj2int[e]
            cs[i] = c
        # Each cost must be in the range of 32 bit signed integer
        # due to the implementation of BDDCT class of SAPPOROBDD.
        assert len([c for c in cs[1:] if c < -(1 << 31) or (1 << 31) <= c]) == 0
        return _graphillion.setset.cost_le(self, costs=cs[1:], cost_bound=cost_bound)

    def add_some_element(self, objtable):
        return _graphillion.setset.add_some_element(self, objtable.num_elems())

    def remove_add_some_elements(self, objtable):
        return _graphillion.setset.remove_add_some_elements(self, objtable.num_elems())

    def to_vertexsetset(self, objtable):
        edges_from_top = [list(e) for e in objtable.int2obj[1:]]
        return _graphillion.setset.to_vertexsetset(self, edges_from_top)

    def to_edgevertexsetset(self, objtable):
        edges_from_top = [list(e) for e in objtable.int2obj[1:]]
        return _graphillion.setset.to_edgevertexsetset(self, edges_from_top)

    @staticmethod
    def get_vertices_from_top(objtable):
        edges_from_top = [list(e) for e in objtable.int2obj[1:]]
        return _graphillion._get_vertices_from_top(edges_from_top)

    @staticmethod
    def load(fp):
        return setset_base(None, _graphillion.load(fp))

    @staticmethod
    def loads(s):
        return setset_base(None, _graphillion.loads(s))
