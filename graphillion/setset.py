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
        setset_or_constraints = setset._conv_arg(setset_or_constraints)
        _graphillion.setset.__init__(self, setset_or_constraints)

    def __repr__(self):
        n = _graphillion.num_elems()
        w = {}
        for i in range(1, n + 1):
            e = setset._int2obj[i]
            w[e] = 1 + float(i) / n**2
        ret = self.__class__.__name__ + '(['
        maxchar = 78
        no_comma = True
        for s in setset.minimize(self, w):
            if no_comma:
                no_comma = False
            else:
                ret += ', '
            ret += str(s)
            if len(ret) > maxchar - 2:
                break
        if len(ret) <= maxchar - 2:
            return ret + '])'
        else:
            return ret[:(maxchar - 4)] + ' ...'

    def __contains__(self, set):
        set = setset._conv_arg(set)
        return _graphillion.setset.__contains__(self, set)

    def include(self, elem):
        elem = setset._conv_elem(elem)
        return _graphillion.setset.include(self, elem)

    def exclude(self, elem):
        elem = setset._conv_elem(elem)
        return _graphillion.setset.exclude(self, elem)

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

    def randomize(self):
        i = _graphillion.setset.randomize(self)
        while (True):
            yield setset._conv_ret(i.next())

    __iter__ = randomize

    def minimize(self, weights=None, default=1):
        return self._optimize(weights, default, _graphillion.setset.minimize)

    def maximize(self, weights=None, default=1):
        return self._optimize(weights, default, _graphillion.setset.maximize)

    def _optimize(self, weights, default, generator):
        ws = [default] * (_graphillion.num_elems() + 1)
        if weights:
            for e, w in weights.iteritems():
                i = setset._obj2int[e]
                ws[i] = w
        i = generator(self, ws)
        while (True):
            yield setset._conv_ret(i.next())

    @staticmethod
    def set_universe(universe):
        _graphillion.num_elems(0)
        setset._obj2int = {}
        setset._int2obj = [None]
        for e in universe:
            setset._add_elem(e)
        setset._check_universe()

    @staticmethod
    def get_universe():
        setset._check_universe()
        return setset._int2obj[1:]

    @staticmethod
    def _check_universe():
        assert len(setset._int2obj) == _graphillion.num_elems() + 1
        for e, i in setset._obj2int.iteritems():
            assert e == setset._int2obj[i]
        for i in xrange(1, len(setset._int2obj)):
            e = setset._int2obj[i]
            assert i == setset._obj2int[e]

    @staticmethod
    def _add_elem(elem):
        assert elem not in setset._obj2int
        i = len(setset._int2obj)
        _graphillion.setset(set([i]))
        setset._obj2int[elem] = i
        setset._int2obj.append(elem)
        assert len(setset._int2obj) == _graphillion.num_elems() + 1
        assert setset._int2obj[i] == elem
        assert setset._obj2int[elem] == i

    @staticmethod
    def _conv_elem(elem):
        if elem not in setset._obj2int:
            setset._add_elem(elem)
        return setset._obj2int[elem]

    @staticmethod
    def _conv_arg(obj):
        if isinstance(obj, list):  # a set of sets [set+]
            l = []
            for s in obj:
                l.append(set([setset._conv_elem(e) for e in s]))
            return l
        elif obj is None:  # an empty set of sets []
            return []
        elif isinstance(obj, (set, frozenset)):  # an inner set
            return set([setset._conv_elem(e) for e in obj])
        elif isinstance(obj, dict):  # constraints
            d = {}
            for k, l in obj.iteritems():
                d[k] = [setset._conv_elem(e) for e in l]
            return d
        else:  # an element
            return setset._conv_elem(obj)

    @staticmethod
    def _conv_ret(obj):
        assert isinstance(obj, (set, frozenset))
        ret = set()
        for e in obj:
            ret.add(setset._int2obj[e])
        return ret

    _obj2int = {}
    _int2obj = [None]
