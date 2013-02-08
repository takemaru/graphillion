import _graphillion


def _add_elem(e):
    assert e not in setset._obj2int
    i = len(setset._int2obj)
    _graphillion.setset(set([i]))
    setset._obj2int[e] = i
    setset._int2obj.append(e)
    assert len(setset._int2obj) == _graphillion.num_elems() + 1
    assert setset._int2obj[i] == e
    assert setset._obj2int[e] == i

def _hook_args(func):
    def wrapper(self, *args, **kwds):
        if args:
            obj = args[0]
            args = [None] + list(args)[1:]
            if obj is None:
                args[0] = None
            elif isinstance(obj, (set, frozenset)):
                args[0] = set([_do_hook_args(e) for e in obj])
            elif isinstance(obj, dict):
                args[0] = {}
                for k, l in obj.iteritems():
                    args[0][k] = [_do_hook_args(e) for e in l]
            elif isinstance(obj, list):
                args[0] = []
                for s in obj:
                    args[0].append(set([_do_hook_args(e) for e in s]))
            else:
                args[0] = _do_hook_args(obj)
        return func(self, *args, **kwds)
    return wrapper

def _do_hook_args(e):
    if e not in setset._obj2int:
        _add_elem(e)
    return setset._obj2int[e]

def _hook_ret(func):
    def wrapper(self, *args, **kwds):
        return _do_hook_ret(func(self, *args, **kwds))
    return wrapper

def _do_hook_ret(obj):
    if isinstance(obj, (set, frozenset)):
        ret = set()
        for e in obj:
            ret.add(setset._int2obj[e])
        return ret
    else:
        return setset._int2obj[obj]


class setset_iterator(object):

    def __init__(self, it):
        self.it = it

    def __iter__(self):
        return self

    @_hook_ret
    def next(self):
        return self.it.next()


class setset(_graphillion.setset):

    @_hook_args
    def __init__(self, obj=None):
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

    @_hook_args
    def __contains__(self, s):
        return _graphillion.setset.__contains__(self, s)

    @_hook_args
    def include(self, e):
        return _graphillion.setset.include(self, e)

    @_hook_args
    def exclude(self, e):
        return _graphillion.setset.exclude(self, e)

    @_hook_args
    def add(self, s):
        _graphillion.setset.add(self, s)
        return self

    @_hook_args
    def remove(self, s):
        _graphillion.setset.remove(self, s)
        return self

    @_hook_args
    def discard(self, s):
        _graphillion.setset.discard(self, s)
        return self

    @_hook_ret
    def pop(self):
        return _graphillion.setset.pop(self)

    def randomize(self):
        i = _graphillion.setset.randomize(self)
        while (True):
            yield _do_hook_ret(i.next())

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
            yield _do_hook_ret(i.next())

    __iter__ = randomize

    @staticmethod
    def universe(universe=None):
        if universe is not None:
            _graphillion.num_elems(0)
            setset._obj2int = {}
            setset._int2obj = [None]
            for e in universe:
                _add_elem(e)
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

    _obj2int = {}
    _int2obj = [None]
