import _graphillion


class setset(_graphillion.setset):

    def __init__(self, obj=None):
        obj = setset._conv_arg(obj)
        _graphillion.setset.__init__(self, obj)

    def __repr__(self):
        n = _graphillion.num_elems()
        w = {}
        for i in range(1, n + 1):
            e = setset._int2obj[i]
            w[e] = 1 + float(i) / n**2
        ret = self.__class__.__name__ + '(['
        no_comma = True
        for s in setset.minimize(self, w):
            if no_comma:
                no_comma = False
            else:
                ret += ', '
            ret += str(s)
            if len(ret) > 78: break
        return ret + '])' if len(ret) <= 78 else ret[:76] + ' ...'

    def __contains__(self, s):
        s = setset._conv_arg(s)
        return _graphillion.setset.__contains__(self, s)

    def include(self, e):
        e = setset._conv_elem(e)
        return _graphillion.setset.include(self, e)

    def exclude(self, e):
        e = setset._conv_elem(e)
        return _graphillion.setset.exclude(self, e)

    def add(self, obj):
        obj = setset._conv_arg(obj)
        return _graphillion.setset.add(self, obj)

    def remove(self, obj):
        obj = setset._conv_arg(obj)
        return _graphillion.setset.remove(self, obj)

    def discard(self, obj):
        obj = setset._conv_arg(obj)
        return _graphillion.setset.discard(self, obj)

    def pop(self):
        s = _graphillion.setset.pop(self)
        return setset._conv_ret(s)

    def invert(self, e):
        e = setset._conv_elem(e)
        return _graphillion.setset.invert(self, e)

    def randomize(self):
        i = _graphillion.setset.randomize(self)
        while (True):
            yield setset._conv_ret(i.next())

    def maximize(self, weights=None, default=1):
        return self._optimize(weights, default, _graphillion.setset.maximize)

    def minimize(self, weights=None, default=1):
        return self._optimize(weights, default, _graphillion.setset.minimize)

    def _optimize(self, weights, default, generator):
        ws = [default] * (_graphillion.num_elems() + 1)
        if weights:
            for e, w in weights.iteritems():
                i = setset._obj2int[e]
                ws[i] = w
        i = generator(self, ws)
        while (True):
            yield setset._conv_ret(i.next())

    __iter__ = randomize

    @staticmethod
    def universe(universe=None):
        if universe is not None:
            _graphillion.num_elems(0)
            setset._obj2int = {}
            setset._int2obj = [None]
            for e in universe:
                setset._add_elem(e)
            setset._check_universe()
        else:
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
    def _add_elem(e):
        assert e not in setset._obj2int
        i = len(setset._int2obj)
        _graphillion.setset(set([i]))
        setset._obj2int[e] = i
        setset._int2obj.append(e)
        assert len(setset._int2obj) == _graphillion.num_elems() + 1
        assert setset._int2obj[i] == e
        assert setset._obj2int[e] == i

    @staticmethod
    def _conv_elem(e):
        if e not in setset._obj2int:
            setset._add_elem(e)
        return setset._obj2int[e]

    @staticmethod
    def _conv_arg(obj):
        if obj is None:
            return None
        elif isinstance(obj, (set, frozenset)):
            return set([setset._conv_elem(e) for e in obj])
        elif isinstance(obj, dict):  # obj is constraints
            d = {}
            for k, l in obj.iteritems():
                d[k] = [setset._conv_elem(e) for e in l]
            return d
        elif isinstance(obj, list):  # obj is [set+]
            l = []
            for s in obj:
                l.append(set([setset._conv_elem(e) for e in s]))
            return l
        else:  # obj is element
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
