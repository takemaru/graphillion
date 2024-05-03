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

"""Module for a set of sets.
"""

from builtins import range
from future.utils import viewitems
import _graphillion


class setset(_graphillion.setset):
    """Represents and manipulates a set of sets.

    A setset object stores a set of sets.  A set element can be any
    hashable object like a number, a text string, and a tuple.

    Like Python set types, setset supports `set in setset`,
    `len(setset)`, and `for set in setset`.  It also supports all set
    methods and operators,
    * isdisjoint(), issubset(), issuperset(), union(), intersection(),
      difference(), symmetric_difference(), copy(), update(),
      intersection_update(), difference_update(),
      symmetric_difference_update(), add(), remove(), discard(),
      pop(), clear(),
    * ==, !=, <=, <, >=, >, |, &, -, ^, |=, &=, -=, ^=.

    Examples:
      >>> from graphillion import setset
      >>> ss = setset([set([1]), set([1,2])])
      >>> len(ss)
      2
      >>> for s in ss:
      ...   s
      set([1])
      set([1, 2])
    """

    def __init__(self, setset_or_constraints=None):
        obj = setset_or_constraints
        if obj is None:
            obj = []
        elif isinstance(obj, list):  # a set of sets [set+]
            l = []
            for s in obj:
                l.append(set([setset._conv_elem(e) for e in s]))
            obj = l
        elif isinstance(obj, dict):  # constraints
            d = {}
            for k, l in viewitems(obj):
                d[k] = [setset._conv_elem(e) for e in l]
            obj = d
        _graphillion.setset.__init__(self, obj)

    def __repr__(self):
        name = self.__class__.__name__
        return self._repr((name + '([', '])'), ('set([', '])'))

    # obj_to_str: dict[tuple, str]
    def _repr(self, outer_braces=('[', ']'), inner_braces=('[', ']'), obj_to_str=None):
        n = _graphillion._num_elems()
        w = {}
        for i in range(1, n + 1):
            e = setset._int2obj[i]
            w[e] = 1 + float(i) / n**2
        ret = outer_braces[0]
        maxchar = 80
        no_comma = True
        for s in setset.min_iter(self, w):
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

    def __contains__(self, set_or_elem):
        set_or_elem = setset._conv_arg(set_or_elem)
        return _graphillion.setset.__contains__(self, set_or_elem)

    def add(self, set_or_elem):
        set_or_elem = setset._conv_arg(set_or_elem)
        return _graphillion.setset.add(self, set_or_elem)

    def remove(self, set_or_elem):
        set_or_elem = setset._conv_arg(set_or_elem)
        return _graphillion.setset.remove(self, set_or_elem)

    def discard(self, set_or_elem):
        set_or_elem = setset._conv_arg(set_or_elem)
        return _graphillion.setset.discard(self, set_or_elem)

    def pop(self):
        set = _graphillion.setset.pop(self)
        return setset._conv_ret(set)

    def flip(self, elem=None):
        if elem is not None:
            elem = setset._conv_elem(elem)
        return _graphillion.setset.flip(self, elem)

    def __iter__(self):
        i = _graphillion.setset.iter(self)
        while (True):
            try:
                yield setset._conv_ret(next(i))
            except StopIteration:
                return

    def rand_iter(self):
        i = _graphillion.setset.rand_iter(self)
        while (True):
            try:
                yield setset._conv_ret(next(i))
            except StopIteration:
                return

    def min_iter(self, weights=None, default=1):
        return self._optimize(weights, default, _graphillion.setset.min_iter)

    def max_iter(self, weights=None, default=1):
        return self._optimize(weights, default, _graphillion.setset.max_iter)

    def _optimize(self, weights, default, generator):
        ws = [default] * (_graphillion._num_elems() + 1)
        if weights:
            for e, w in viewitems(weights):
                i = setset._obj2int[e]
                ws[i] = w
        i = generator(self, ws)
        while (True):
            try:
                yield setset._conv_ret(next(i))
            except StopIteration:
                return

    def supersets(self, obj):
        if (not isinstance(obj, setset)):
            obj = setset._conv_elem(obj)
        return _graphillion.setset.supersets(self, obj)

    def non_supersets(self, obj):
        if (not isinstance(obj, setset)):
            obj = setset._conv_elem(obj)
        return _graphillion.setset.non_supersets(self, obj)

    def choice(self):
        set = _graphillion.setset.choice(self)
        return setset._conv_ret(set)

    def probability(self, probabilities):
        ps = [-1] * (_graphillion._num_elems() + 1)
        for e, p in viewitems(probabilities):
            i = setset._obj2int[e]
            ps[i] = p
        assert len([p for p in ps[1:] if p < 0 or 1 < p]) == 0
        return _graphillion.setset.probability(self, ps)

    def cost_le(self, costs, cost_bound):
        cs = [-1] * (_graphillion._num_elems() + 1)
        for e, c in viewitems(costs):
            i = setset._obj2int[e]
            cs[i] = c
        # Each cost must be in the range of 32 bit signed integer
        # due to the implementation of BDDCT class of SAPPOROBDD.
        assert len([c for c in cs[1:] if c < -(1 << 31) or (1 << 31) <= c]) == 0
        return _graphillion.setset.cost_le(self, costs=cs[1:], cost_bound=cost_bound)

    def to_vertexsetset(self):
        edges_from_top = [list(e) for e in setset._int2obj[1:]]
        return _graphillion.setset.to_vertexsetset(self, edges_from_top)

    @staticmethod
    def get_vertices_from_top():
        edges_from_top = [list(e) for e in setset._int2obj[1:]]
        return _graphillion._get_vertices_from_top(edges_from_top)

    @staticmethod
    def load(fp):
        """Deserialize a file `fp` to `self`.

        This method does not deserialize the universe, which should be
        loaded separately by pickle.

        Args:
          fp: A read-supporting file-like object.

        Examples of dump():
          >>> import pickle
          >>> fp = open('/path/to/setset', 'wb')
          >>> ss.dump(fp)
          >>> fp = open('/path/to/universe' 'wb')
          >>> pickle.dump(setset.universe(), fp)

        Examples of load():
          >>> import pickle
          >>> fp = open('/path/to/universe')
          >>> setset.set_universe(pickle.load(fp))
          >>> fp = open('/path/to/setset')
          >>> ss = setset.load(fp)

        See Also:
          loads()
        """
        return setset(_graphillion.load(fp))

    @staticmethod
    def loads(s):
        """Deserialize `s` to `self`.

        This method does not deserialize the universe, which should be
        loaded separately by pickle.

        Args:
          s: A string instance.

        Examples of dump():
          >>> import pickle
          >>> setset_str = ss.dumps()
          >>> universe_str = pickle.dumps(setset.universe())

        Examples of load():
          >>> import pickle
          >>> setset.set_universe(pickle.loads(universe_str))
          >>> ss = setset.load(graphset_str)

        See Also:
          load()
        """
        return setset(_graphillion.loads(s))

    @staticmethod
    def set_universe(universe):
        if len(universe) != len(set(universe)):
            raise ValueError('duplicated elements found')
        _graphillion._num_elems(0)
        setset._obj2int = {}
        setset._int2obj = [None]
        for e in universe:
            setset._add_elem(e)
        setset._check_universe()

    @staticmethod
    def universe():
        setset._check_universe()
        return setset._int2obj[1:]

    @staticmethod
    def _check_universe():
        assert len(setset._int2obj) == _graphillion._num_elems() + 1
        for e, i in viewitems(setset._obj2int):
            assert e == setset._int2obj[i]
        for i in range(1, len(setset._int2obj)):
            e = setset._int2obj[i]
            assert i == setset._obj2int[e]

    @staticmethod
    def _add_elem(elem):
        assert elem not in setset._obj2int
        if len(setset._obj2int) >= _graphillion._elem_limit():
            m = 'too many elements are set, which must be %d or less' % _graphillion._elem_limit()
            raise RuntimeError(m)
        i = len(setset._int2obj)
        _graphillion.setset([set([i])])
        setset._obj2int[elem] = i
        setset._int2obj.append(elem)
        assert len(setset._int2obj) == _graphillion._num_elems() + 1
        assert setset._int2obj[i] == elem
        assert setset._obj2int[elem] == i

    @staticmethod
    def _conv_elem(elem):
        if elem not in setset._obj2int:
            setset._add_elem(elem)
        return setset._obj2int[elem]

    @staticmethod
    def _conv_arg(obj):
        if isinstance(obj, (set, frozenset)):  # a set
            return set([setset._conv_elem(e) for e in obj])
        else:  # an element
            return setset._conv_elem(obj)

    @staticmethod
    def _conv_ret(obj):
        if isinstance(obj, (set, frozenset)):  # a set
            ret = set()
            for e in obj:
                ret.add(setset._int2obj[e])
            return ret
        raise TypeError(obj)

    _obj2int = {}
    _int2obj = [None]
