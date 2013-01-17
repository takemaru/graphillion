import _illion

def add_elem(e):
    setset._obj2int[e] = len(setset._int2obj)
    setset._int2obj.append(e)

def conv_arg(obj):
    if isinstance(obj, (set, frozenset)):
        s = set()
        for e in obj:
            if e not in setset._obj2int:
                add_elem(e)
            s.add(setset._obj2int[e])
        return s
    elif isinstance(obj, dict):
        d = {}
        for k, s in obj.iteritems():
            d[k] = conv_arg(s)
        return d
    else:
        e = obj
        if e not in setset._obj2int:
            add_elem(e)
        return setset._obj2int[e]

def conv_ret(s):
    if setset.INT_ELEM_ONLY or s is None:
        return s
    elif isinstance(s, (set, frozenset)):
        ret = set()
        for e in s:
            ret.add(setset._int2obj[e])
        return ret
    else:
        raise TypeError('not set')

def hook_arg(func):
    def wrapper(self, *args, **kwds):
        if setset.INT_ELEM_ONLY or not args:
            return func(self, *args, **kwds)
        else:
            obj = args[0]
            args2 = []
            if isinstance(obj, list):
                l = []
                for o in obj:
                    l.append(conv_arg(o))
                args2.append(l)
            else:
                args2.append(conv_arg(obj))
            return func(self, *args2, **kwds)
    return wrapper

def hook_ret(func):
    def wrapper(self, *args, **kwds):
        return conv_ret(func(self, *args, **kwds))
    return wrapper


class setset_iterator(object):

    def __init__(self, it):
        self.it = it

    def __iter__(self):
        return self

    @hook_ret
    def next(self):
        return self.it.next()


class setset(_illion.setset):

    @hook_arg
    def __init__(self, *args, **kwds):
        _illion.setset.__init__(self, *args, **kwds);

    @hook_arg
    def __contains__(self, *args, **kwds):
        return _illion.setset.__contains__(self, *args, **kwds);

    @hook_arg
    def find(self, *args, **kwds):
        return _illion.setset.find(self, *args, **kwds);

    @hook_arg
    def not_find(self, *args, **kwds):
        return _illion.setset.not_find(self, *args, **kwds);

    @hook_arg
    def add(self, *args, **kwds):
        return _illion.setset.add(self, *args, **kwds)

    @hook_arg
    def remove(self, *args, **kwds):
        return _illion.setset.remove(self, *args, **kwds)

    @hook_arg
    def discard(self, *args, **kwds):
        return _illion.setset.discard(self, *args, **kwds)

    @hook_ret
    def pop(self, *args, **kwds):
        return _illion.setset.pop(self, *args, **kwds)

    def __iter__(self):
        return setset_iterator(self.rand_iter())

    def randomize(self):
        i = self.rand_iter()
        while (True):
            yield conv_ret(i.next())

    def optimize(self, weights_arg):
        weights = [0] * (len(setset.universe()) + 1)
        for o, w in weights_arg.iteritems():
            i = setset._obj2int[o]
            weights[i] = w
        i = self.opt_iter(weights)
        while (True):
            yield conv_ret(i.next())

    @staticmethod
    def universe(*args):
        if setset.INT_ELEM_ONLY:
            if args:
                pass
            else:
                return range(1, _illion.num_elems() + 1)
        else:
            if args:
                setset._obj2int = {}
                setset._int2obj = [None]
                for e in args[0]:
                    add_elem(e)
            else:
                return setset._int2obj[1:]

    INT_ELEM_ONLY = False
    _obj2int = {}
    _int2obj = [None]

#class graphset(setset):
#    pass
