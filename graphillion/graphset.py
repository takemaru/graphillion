from graphillion import setset

class GraphSet(setset):

    def __init__(self, obj=None):
        obj = GraphSet._conv_obj(obj)
        setset.__init__(self, obj)

    def __contains__(self, s):
        s = GraphSet._conv_obj(s)
        return setset.__contains__(self, s)

    def include(self, obj):
        try:  # if obj is edge
            return setset.include(self, GraphSet._conv_edge(obj))
        except KeyError:  # else obj is vertex
            gs = GraphSet()
            for edge in [e for e in setset.universe() if obj in e]:
                gs |= setset.include(self, edge)
            return gs & self

    def exclude(self, obj):
        try:  # if obj is edge
            return setset.exclude(self, GraphSet._conv_edge(obj))
        except KeyError:  # else obj is vertex
            return self - self.include(obj)

    def add(self, obj):
        obj = GraphSet._conv_obj(obj)
        return setset.add(self, obj)

    def remove(self, obj):
        obj = GraphSet._conv_obj(obj)
        return setset.remove(self, obj)

    def discard(self, obj):
        obj = GraphSet._conv_obj(obj)
        return setset.discard(self, obj)

    def maximize(self):
        for s in setset.maximize(self, GraphSet._weights):
            yield s

    def minimize(self):
        for s in setset.minimize(self, GraphSet._weights):
            yield s

    @staticmethod
    def universe(universe=None, traversal=None, source=None):
        if universe is not None:
            edges = []
            GraphSet._weights = {}
            for e in universe:
                edges.append(e[:2])
                if len(e) > 2:
                    GraphSet._weights[e[:2]] = e[2]
            if traversal:
                if not source:
                    source = edges[0][0]
                    for e in edges:
                        source = min(e[0], e[1], source)
                edges = GraphSet._traverse(edges, traversal, source)
            setset.universe(edges)
        else:
            edges = []
            for e in setset.universe():
                if e in GraphSet._weights:
                    edges.append((e[0], e[1], GraphSet._weights[e]))
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

    @staticmethod
    def _conv_obj(obj):
        if obj is None:
            return None
        elif isinstance(obj, (set, frozenset)):
            return set([GraphSet._conv_edge(e) for e in obj])
        elif isinstance(obj, dict):  # obj is constraints
            d = {}
            for k, l in obj.iteritems():
                d[k] = [GraphSet._conv_edge(e) for e in l]
            return d
        elif isinstance(obj, list):  # obj is [set+]
            l = []
            for s in obj:
                l.append([GraphSet._conv_edge(e) for e in s])
            return l
        else:  # obj is edge
            return GraphSet._conv_edge(obj)

    @staticmethod
    def _conv_edge(e):
        if not isinstance(e, tuple):
            raise KeyError, e
        if e in setset._obj2int:
            return e
        elif (e[1], e[0]) in setset._obj2int:
            return (e[1], e[0])
        raise KeyError, e

    _weights = {}
