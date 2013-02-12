from graphillion import setset

class GraphSet(setset):

    def __init__(self, graphset_or_constraints=None):
        graphset_or_constraints = GraphSet._conv_arg(graphset_or_constraints)
        setset.__init__(self, graphset_or_constraints)

    def __contains__(self, graph):
        graph = GraphSet._conv_arg(graph)
        return setset.__contains__(self, graph)

    def include(self, edge_or_vertex):
        try:  # if edge
            return setset.include(self, GraphSet._conv_edge(edge_or_vertex))
        except KeyError:  # else
            gs = GraphSet()
            for edge in [e for e in setset.universe() if edge_or_vertex in e]:
                gs |= setset.include(self, edge)
            return gs & self

    def exclude(self, edge_or_vertex):
        try:  # if edge
            return setset.exclude(self, GraphSet._conv_edge(edge_or_vertex))
        except KeyError:  # else
            return self - self.include(edge_or_vertex)

    def add(self, graph_or_edge):
        graph_or_edge = GraphSet._conv_arg(graph_or_edge)
        return setset.add(self, graph_or_edge)

    def remove(self, graph_or_edge):
        graph_or_edge = GraphSet._conv_arg(graph_or_edge)
        return setset.remove(self, graph_or_edge)

    def discard(self, graph_or_edge):
        graph_or_edge = GraphSet._conv_arg(graph_or_edge)
        return setset.discard(self, graph_or_edge)

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
    def _conv_arg(obj):
        if isinstance(obj, list):  # a set of graphs [set+]
            l = []
            for s in obj:
                l.append([GraphSet._conv_edge(e) for e in s])
            return l
        elif obj is None:  # an empty set of graphs
            return []
        elif isinstance(obj, (set, frozenset)):  # a graph
            return set([GraphSet._conv_edge(e) for e in obj])
        elif isinstance(obj, dict):  # constraints
            d = {}
            for k, l in obj.iteritems():
                d[k] = [GraphSet._conv_edge(e) for e in l]
            return d
        else:  # an edge
            return GraphSet._conv_edge(obj)

    @staticmethod
    def _conv_edge(edge):
        if not isinstance(edge, tuple) or len(edge) < 2:
            raise KeyError, edge
        if len(edge) > 2:
            edge = edge[:2]
        if edge in setset._obj2int:
            return edge
        elif (edge[1], edge[0]) in setset._obj2int:
            return (edge[1], edge[0])
        raise KeyError, edge

    _weights = {}
