import _illion


def add_elem(e):
    assert e not in setset._obj2int
    i = len(setset._int2obj)
    _illion.setset(set([i]))
    setset._obj2int[e] = i
    setset._int2obj.append(e)
    assert len(setset._int2obj) == _illion.num_elems() + 1
    assert setset._int2obj[i] == e
    assert setset._obj2int[e] == i

def conv_elem(e):
    if e not in setset._obj2int:
        add_elem(e)
    return setset._obj2int[e]

def conv_arg(func):
    def wrapper(self, *args, **kwds):
        if not args:
            return func(self, *args, **kwds)
        else:
            obj = args[0]
            args = [None] + list(args)[1:]
            if isinstance(obj, (set, frozenset)):
                args[0] = set([conv_elem(e) for e in obj])
            elif isinstance(obj, dict):
                d = {}
                for k, l in obj.iteritems():
                    d[k] = [conv_elem(e) for e in l]
                args[0] = d
            elif isinstance(obj, list):
                l = []
                for s in obj:
                    l.append(set([conv_elem(e) for e in s]))
                args[0] = l
            else:
                args[0] = conv_elem(obj)
            return func(self, *args, **kwds)
    return wrapper

def do_conv_ret(obj):
    if isinstance(obj, (set, frozenset)):
        ret = set()
        for e in obj:
            ret.add(setset._int2obj[e])
        return ret
    else:
        return setset._int2obj[obj]

def conv_ret(func):
    def wrapper(self, *args, **kwds):
        return do_conv_ret(func(self, *args, **kwds))
    return wrapper

def do_check_arg(l):
    for e in l:
        if e not in setset._obj2int:
            raise KeyError, e

def check_arg(func):
    def wrapper(self, *args, **kwds):
        if args:
            obj = args[0]
            if isinstance(obj, (set, frozenset)):
                do_check_arg(obj)
            elif isinstance(obj, dict):
                for k, l in obj.iteritems():
                    do_check_arg(l)
            elif isinstance(obj, list):
                for s in obj:
                    do_check_arg(s)
        return func(self, *args, **kwds)
    return wrapper


class setset_iterator(object):

    def __init__(self, it):
        self.it = it

    def __iter__(self):
        return self

    @conv_ret
    def next(self):
        return self.it.next()


class setset(_illion.setset):

    @conv_arg
    def __init__(self, *args, **kwds):
        _illion.setset.__init__(self, *args, **kwds)

    def __repr__(self):
        n = _illion.num_elems()
        w = {}
        for i in range(1, n + 1):
            e = setset._int2obj[i]
            w[e] = - (1 + float(i) / n**2)
        ret = self.__class__.__name__ + '(['
        i = 1
        for s in self.optimize(w):
            if i >= 2:
                ret += ', '
            ret += str(s)
            if i >= 4:
                i = -1
                break
            else:
                i += 1
        if i < 0:
            ret += ', ...'
        return ret + '])'

    @conv_arg
    def __contains__(self, *args, **kwds):
        return _illion.setset.__contains__(self, *args, **kwds)

    @conv_arg
    def include(self, *args, **kwds):
        return _illion.setset.include(self, *args, **kwds)

    @conv_arg
    def exclude(self, *args, **kwds):
        return _illion.setset.exclude(self, *args, **kwds)

    @conv_arg
    def add(self, *args, **kwds):
        return _illion.setset.add(self, *args, **kwds)

    @conv_arg
    def remove(self, *args, **kwds):
        return _illion.setset.remove(self, *args, **kwds)

    @conv_arg
    def discard(self, *args, **kwds):
        return _illion.setset.discard(self, *args, **kwds)

    @conv_ret
    def pop(self, *args, **kwds):
        return _illion.setset.pop(self, *args, **kwds)

    def __iter__(self):
        return setset_iterator(self.rand_iter())

    def randomize(self):
        i = self.rand_iter()
        while (True):
            yield do_conv_ret(i.next())

    def optimize(self, weights_arg):
        weights = [1] * (_illion.num_elems() + 1)
        for e, w in weights_arg.iteritems():
            i = setset._obj2int[e]
            weights[i] = w
        i = self.opt_iter(weights)
        while (True):
            yield do_conv_ret(i.next())

    @staticmethod
    def universe(*args):
        if args:
            _illion.num_elems(0)
            setset._obj2int = {}
            setset._int2obj = [None]
            for e in args[0]:
                add_elem(e)
            setset._check_universe()
        else:
            setset._check_universe()
            return setset._int2obj[1:]

    @staticmethod
    def _check_universe():
        assert len(setset._int2obj) == _illion.num_elems() + 1
        for e, i in setset._obj2int.iteritems():
            assert e == setset._int2obj[i]
        for i in xrange(1, len(setset._int2obj)):
            e = setset._int2obj[i]
            assert i == setset._obj2int[e]

    _obj2int = {}
    _int2obj = [None]


class graphset(setset):

    @check_arg
    def __init__(self, *args, **kwds):
        setset.__init__(self, *args, **kwds)

    def paths(self, u, v):
        assert False, 'not implemented'

    def cycles(self):
        assert False, 'not implemented'

    def trees(self):
        assert False, 'not implemented'

    def forests(self):
        assert False, 'not implemented'

    @check_arg
    def __contains__(self, *args, **kwds):
        return setset.__contains__(self, *args, **kwds)

    @check_arg
    def include(self, *args, **kwds):
        return setset.include(self, *args, **kwds)

    def include_edge(self, *args, **kwds):
        return graphset.include(self, *args, **kwds)

    def exclude_edge(self, *args, **kwds):
        return graphset.exclude(self, *args, **kwds)

    def include_vertex(self, v):
        gs = graphset()
        for edge in [e for e in setset.universe() if v in e]:
            gs |= self.include(edge)
        return gs & self

    def exclude_vertex(self, v):
        return self - self.include_vertex(v)

    @check_arg
    def exclude(self, *args, **kwds):
        return setset.exclude(self, *args, **kwds)

    @check_arg
    def add(self, *args, **kwds):
        return setset.add(self, *args, **kwds)

    @check_arg
    def remove(self, *args, **kwds):
        return setset.remove(self, *args, **kwds)

    @check_arg
    def discard(self, *args, **kwds):
        return setset.discard(self, *args, **kwds)

    def optimize(self):
        for i in setset.optimize(self, graphset._weights):
            yield i

    @staticmethod
    def universe(*args, **kwds):
        if args:
            edges = []
            graphset._weights = {}
            for e in args[0]:
                edges.append(e[:2])
                if len(e) > 2:
                    graphset._weights[e[:2]] = e[2]
            if 'traversal' in kwds:
                if 'source' in kwds:
                    source = kwds['source']
                else:
                    source = edges[0][0]
                    for e in edges:
                        source = min(e[0], e[1], source)
                edges = graphset._traverse(edges, kwds['traversal'], source)
            setset.universe(edges)
        else:
            edges = []
            for e in setset.universe():
                if e in graphset._weights:
                    edges.append((e[0], e[1], graphset._weights[e]))
                else:
                    edges.append(e)
            return edges

    @staticmethod
    def _traverse(edges, traversal, source):
        neighbors = {}
        for u, v in edges:
            if u not in neighbors:
                neighbors[u] = set([v])
            else:
                neighbors[u].add(v)
            if v not in neighbors:
                neighbors[v] = set([u])
            else:
                neighbors[v].add(u)
        assert source in neighbors

        sorted_edges = []
        queue_or_stack = []
        visited_vertices = set()
        u = source
        while True:
            if u in visited_vertices:
                continue
            visited_vertices.add(u)
            for v in sorted(neighbors[u]):
                if v in visited_vertices:
                    e = (u, v) if (u, v) in edges else (v, u)
                    sorted_edges.append(e)
            new_vertices = neighbors[u] - visited_vertices - set(queue_or_stack)
            queue_or_stack.extend(new_vertices)
            if not queue_or_stack:
                break
            if traversal == 'bfs':
                u, queue_or_stack = queue_or_stack[0], queue_or_stack[1:]
            else:
                queue_or_stack, u = queue_or_stack[:-1], queue_or_stack[-1]
        assert set(edges) == set(sorted_edges)
        return sorted_edges

    _weights = {}
