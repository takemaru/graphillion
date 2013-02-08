from illion import setset

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
                    args[0].append([_do_hook_args(e) for e in s])
            else:
                args[0] = _do_hook_args(obj)
        return func(self, *args, **kwds)
    return wrapper

def _do_hook_args(e):
    if e in setset._obj2int:
        return e
    elif (e[1], e[0]) in setset._obj2int:
        return (e[1], e[0])
    else:
        raise KeyError, e


class graphset(setset):

    @_hook_args
    def __init__(self, *args, **kwds):
        setset.__init__(self, *args, **kwds)

    @_hook_args
    def __contains__(self, *args, **kwds):
        return setset.__contains__(self, *args, **kwds)

    @_hook_args
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

    @_hook_args
    def exclude(self, *args, **kwds):
        return setset.exclude(self, *args, **kwds)

    @_hook_args
    def add(self, *args, **kwds):
        return setset.add(self, *args, **kwds)

    @_hook_args
    def remove(self, *args, **kwds):
        return setset.remove(self, *args, **kwds)

    @_hook_args
    def discard(self, *args, **kwds):
        return setset.discard(self, *args, **kwds)

    def optimize(self):
        for s in setset.optimize(self, graphset._weights):
            yield s

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
