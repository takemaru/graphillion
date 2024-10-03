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
import _graphillion

class ObjectTable:

    def __init__(self):
        self.obj2int = {}
        self.int2obj = [None]

    def num_elems(self):
        return len(self.int2obj) - 1

    def check_universe(self):
        for e, i in self.obj2int.items():
            assert e == self.int2obj[i]
        for i in range(1, len(self.int2obj)):
            e = self.int2obj[i]
            assert i == self.obj2int[e]

    def universe(self):
        return self.int2obj[1:]

    def add_elem(self, elem):
        assert elem not in self.obj2int
        if len(self.obj2int) >= _graphillion._elem_limit():
            m = 'too many elements are set, which must be %d or less' % _graphillion._elem_limit()
            raise RuntimeError(m)
        i = len(self.int2obj)
        _graphillion.setset([set([i])])
        self.obj2int[elem] = i
        self.int2obj.append(elem)
        assert self.int2obj[i] == elem
        assert self.obj2int[elem] == i

    def conv_elem(self, elem):
        if elem not in self.obj2int:
            self.add_elem(elem)
        return self.obj2int[elem]

    def conv_arg(self, obj):
        if isinstance(obj, (set, frozenset)):  # a set
            return set([self.conv_elem(e) for e in obj])
        else:  # an element
            return self.conv_elem(obj)

    def conv_ret(self, obj):
        if isinstance(obj, (set, frozenset)):  # a set
            ret = set()
            for e in obj:
                ret.add(self.int2obj[e])
            return ret
        raise TypeError(obj)



class setset_base(_graphillion.setset):
    """Represents and manipulates a set of sets.

    A setset_base object stores a set of sets.  A set element can be any
    hashable object like a number, a text string, and a tuple.

    Like Python set types, setset_base supports `set in setset`,
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

    #def __iter__(self):
    def _iter(self, objtable):
        i = _graphillion.setset.iter(self)
        while (True):
            try:
                #yield setset_base._conv_ret(next(i))
                yield objtable.conv_ret(next(i))
            except StopIteration:
                return

    def rand_iter(self, objtable):
        i = _graphillion.setset.rand_iter(self)
        while (True):
            try:
                #yield setset_base._conv_ret(next(i))
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
            for e, w in weights.items():
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

    # num_elems will be removed
    def add_some_element(self, objtable, num_elems = None):
        if num_elems:
            return _graphillion.setset.add_some_element(self, num_elems)
        else:
            return _graphillion.setset.add_some_element(self, objtable.num_elems())

    # num_elems will be removed
    def remove_add_some_elements(self, objtable, num_elems = None):
        if num_elems:
            return _graphillion.setset.remove_add_some_elements(self, num_elems)
        else:
            return _graphillion.setset.remove_add_some_elements(self, objtable.num_elems())

    def to_vertexsetset(self, objtable):
        edges_from_top = [list(e) for e in objtable.int2obj[1:]]
        return _graphillion.setset.to_vertexsetset(self, edges_from_top)

    @staticmethod
    def get_vertices_from_top(objtable):
        edges_from_top = [list(e) for e in objtable.int2obj[1:]]
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
          >>> fp = open('/path/to/setset_base', 'wb')
          >>> ss.dump(fp)
          >>> fp = open('/path/to/universe' 'wb')
          >>> pickle.dump(setset_base.universe(), fp)

        Examples of load():
          >>> import pickle
          >>> fp = open('/path/to/universe')
          >>> setset_base.set_universe(pickle.load(fp))
          >>> fp = open('/path/to/setset_base')
          >>> ss = setset_base.load(fp)

        See Also:
          loads()
        """
        return setset_base(None, _graphillion.load(fp))

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
          >>> universe_str = pickle.dumps(setset_base.universe())

        Examples of load():
          >>> import pickle
          >>> setset_base.set_universe(pickle.loads(universe_str))
          >>> ss = setset_base.load(graphset_str)

        See Also:
          load()
        """
        return setset_base(None, _graphillion.loads(s))

    @staticmethod
    def set_universe(universe):
        if len(universe) != len(set(universe)):
            raise ValueError('duplicated elements found')
        #_graphillion._num_elems(0)
        setset_base._obj2int = {}
        setset_base._int2obj = [None]
        for e in universe:
            setset_base._add_elem(e)
        setset_base._check_universe()

    @staticmethod
    def universe():
        setset_base._check_universe()
        return setset_base._int2obj[1:]

    @staticmethod
    def _check_universe():
        #assert len(setset_base._int2obj) == _graphillion._num_elems() + 1
        for e, i in setset_base._obj2int.items():
            assert e == setset_base._int2obj[i]
        for i in range(1, len(setset_base._int2obj)):
            e = setset_base._int2obj[i]
            assert i == setset_base._obj2int[e]

    @staticmethod
    def _add_elem(elem):
        assert elem not in setset_base._obj2int
        if len(setset_base._obj2int) >= _graphillion._elem_limit():
            m = 'too many elements are set, which must be %d or less' % _graphillion._elem_limit()
            raise RuntimeError(m)
        i = len(setset_base._int2obj)
        _graphillion.setset([set([i])])
        setset_base._obj2int[elem] = i
        setset_base._int2obj.append(elem)
        #assert len(setset_base._int2obj) == _graphillion._num_elems() + 1
        assert setset_base._int2obj[i] == elem
        assert setset_base._obj2int[elem] == i

    @staticmethod
    def _conv_elem(elem):
        if elem not in setset_base._obj2int:
            setset_base._add_elem(elem)
        return setset_base._obj2int[elem]

    @staticmethod
    def _conv_arg(obj):
        if isinstance(obj, (set, frozenset)):  # a set
            return set([setset_base._conv_elem(e) for e in obj])
        else:  # an element
            return setset_base._conv_elem(obj)

    @staticmethod
    def _conv_ret(obj):
        if isinstance(obj, (set, frozenset)):  # a set
            ret = set()
            for e in obj:
                ret.add(setset_base._int2obj[e])
            return ret
        raise TypeError(obj)

    _obj2int = {}
    _int2obj = [None]
