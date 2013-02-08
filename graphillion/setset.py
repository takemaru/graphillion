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
            if isinstance(obj, (set, frozenset)):
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
    def __init__(self, *args, **kwds):
        _graphillion.setset.__init__(self, *args, **kwds)

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
    def __contains__(self, *args, **kwds):
        return _graphillion.setset.__contains__(self, *args, **kwds)

    @_hook_args
    def include(self, *args, **kwds):
        return _graphillion.setset.include(self, *args, **kwds)

    @_hook_args
    def exclude(self, *args, **kwds):
        return _graphillion.setset.exclude(self, *args, **kwds)

    @_hook_args
    def add(self, *args, **kwds):
        return _graphillion.setset.add(self, *args, **kwds)

    @_hook_args
    def remove(self, *args, **kwds):
        return _graphillion.setset.remove(self, *args, **kwds)

    @_hook_args
    def discard(self, *args, **kwds):
        return _graphillion.setset.discard(self, *args, **kwds)

    @_hook_ret
    def pop(self, *args, **kwds):
        return _graphillion.setset.pop(self, *args, **kwds)

    def randomize(self):
        i = _graphillion.setset.randomize(self)
        while (True):
            yield _do_hook_ret(i.next())

    def maximize(self, *args, **kwds):
        kwds['generator'] = _graphillion.setset.maximize;
        return self._optimize(*args, **kwds)

    def minimize(self, *args, **kwds):
        kwds['generator'] = _graphillion.setset.minimize;
        return self._optimize(*args, **kwds)

    def _optimize(self, *args, **kwds):
        default = kwds['default'] if 'default' in kwds else 1
        weights = [default] * (_graphillion.num_elems() + 1)
        if args:
            for e, w in args[0].iteritems():
                i = setset._obj2int[e]
                weights[i] = w
        i = kwds['generator'](self, weights)
        while (True):
            yield _do_hook_ret(i.next())

    __iter__ = randomize

    @staticmethod
    def universe(*args):
        if args:
            _graphillion.num_elems(0)
            setset._obj2int = {}
            setset._int2obj = [None]
            for e in args[0]:
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
