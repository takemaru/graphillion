import _illion

class setset(_illion.setset):
    INT_ONLY = False
    obj2int = {}
    int2obj = [None]

    @staticmethod
    def add_elem(e):
        setset.obj2int[e] = len(setset.int2obj)
        setset.int2obj.append(e)

    @staticmethod
    def universe(*args):
        if args:
            setset.obj2int = {}
            setset.int2obj = [None]
            for e in args[0]:
                setset.add_elem(e)
        else:
            return setset.int2obj[1:]

    @staticmethod
    def conv(obj):
        if isinstance(obj, (set, frozenset)):
            s = set()
            for e in obj:
                if e not in setset.obj2int:
                    setset.add_elem(e)
                s.add(setset.obj2int[e])
            return s
        elif isinstance(obj, dict):
            d = {}
            for k, s in obj.iteritems():
                d[k] = setset.conv(s)
            return d
        else:
            raise TypeError('not set or dict')

    def hookarg(func):
        def wrapper(self, *args, **kwds):
            if setset.INT_ONLY or not args:
                return func(self, *args, **kwds)
            else:
                obj = args[0]
                args2 = []
                if isinstance(obj, list):
                    l = []
                    for o in obj:
                        l.append(setset.conv(o))
                    args2.append(l)
                else:
                    args2.append(setset.conv(obj))
                return func(self, *args2, **kwds)
        return wrapper

    def hookret(func):
        def wrapper(self, *args, **kwds):
            s = func(self, *args, **kwds);
            if setset.INT_ONLY or s is None:
                return s
            else:
                ret = set()
                for e in s:
                    ret.add(setset.int2obj[e])
                return ret
        return wrapper

    @hookarg
    def __init__(self, *args, **kwds):
        _illion.setset.__init__(self, *args, **kwds);

    @hookret
    def pop(self):
        return _illion.setset.pop(self)

    def optimize(self, weights):
        i = self.opt_iter(weights)
        while (True):
            yield i.next()

#class graphset(setset):
#    pass
