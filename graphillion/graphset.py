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

from graphillion import setset


class GraphSet(setset):
    """Represents and manipulates a set of graphs.

    A GraphSet object stores a set of graphs.  A graph stored is a
    subgraph of the universal graph, and it is represented by a set of
    edges in the universal graph.  An edge is a tuple of two vertices,
    and a vertex can be any hashable object like a number, a text
    string, and a tuple.  Currently, GraphSet only supports undirected
    graphs without edge labels.

    The universal graph must be defined before creating GraphSet
    objects by `GraphSet.universe()` method.

    Like Python set types, GraphSet supports `graph in graphset`,
    `len(graphset)`, and `for graph in graphset`.

    Examples:
      >>> from graphillion import GraphSet

      We assume the following graph and register the edge list as the
      universe.

      1 --- 2 --- 3
      |     |     |
      4 --- 5 --- 6

      >>> GraphSet.universe([(1,2), (1,4), (2,3), (2,5), (3,6), (4,5), (5,6)])

      Find all paths from 1 to 6 and count them.

      >>> paths = GraphSet.path(1, 6)
      >>> len(paths)
      3

      Give constraints in which edge 1-4 must not be passed but 2 must
      be passed, and show the paths that meet the constraints.

      >>> paths = paths.exclude((1,4)).include(2)
      >>> for path in paths:
      ...   path
      set([(1, 2), (2, 3), (3, 6)])
      set([(1, 2), (2, 5), (5, 6)])
    """

    def __init__(self, graphset_or_constraints=None):
        """Initializes a GraphSet object with a set of graphs or constraints.

        Examples:
          >>> gs = GraphSet([set([]), set([(1,2), (2,3)])])
          >>> gs
          GraphSet([set([]), set([(1,2), (2,3)])])
          >>> gs = GraphSet({'include': [(1,2), (2,3)], 'exclude': [(1,4)]})
          >>> gs
          GraphSet([set([(1,2), (2,3)]), set([(1,2), (2,3), (2,5)]), ...

        Args:
          graphset_or_constraints: A set of graphs represented by a
            list of edge sets:

            [set([]), set([(1,2), (2,3)])]

            Or constraints represented by a dict of included or
            excluded edge lists (not-specified edges are not cared):

            {'include': [(1,2), (2,3)], 'exclude': [(1,4)]}

            If no argument is given, it is treated as an empty list
            `[]` and an empty GraphSet is returned.  An empty dict
            `{}` means that no constraint is specified, and so a
            GraphSet including all possible graphs in the universe is
            returned (let N the number of edges in the universe, 2^N
            graphs are stored).

        Raises:
          KeyError: If given edges are not found in the universe.
        """
        graphset_or_constraints = GraphSet._conv_arg(graphset_or_constraints)
        setset.__init__(self, graphset_or_constraints)

    def __contains__(self, graph):
        """Returns True if `graph` is in the `self` GraphSet, False otherwise.

        Use the expression `graph in graphset`.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> set([(2,3)]) in gs
          True

        Args:
          graph: A graph (a set of edges) in the universe.

        Returns:
          True or False.

        Raises:
          KeyError: If the given graph is not found in the universe.
        """
        graph = GraphSet._conv_arg(graph)
        return setset.__contains__(self, graph)

    def include(self, edge_or_vertex):
        """Returns a new set of graphs that include a given edge or vertex.

        The graphs stored in the new GraphSet are selected from the
        `self` GraphSet.  The `self` GraphSet is not changed.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> gs = gs.include(4)
          >>> gs
          GraphSet([set([(1,2), (1,4)])])

        Args:
          edge_or_vertex: An edge or a vertex in the universe.

        Returns:
          A new GraphSet object.

        Raises:
          KeyError: If a given edge or vertex is not found in the universe.
        """
        try:  # if edge
            return setset.include(self, GraphSet._conv_edge(edge_or_vertex))
        except KeyError:  # else
            gs = GraphSet()
            for edge in [e for e in setset.universe() if edge_or_vertex in e]:
                gs |= setset.include(self, edge)
            return gs & self

    def exclude(self, edge_or_vertex):
        """Returns a new set of graphs that don't include a given edge or vertex.

        The graphs stored in the new GraphSet are selected from `self`
        GraphSet.  The `self` GraphSet is not changed.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> gs = gs.exclude(4)
          >>> gs
          GraphSet([set([(2,3)])])

        Args:
          edge_or_vertex: An edge or a vertex in the universe.

        Returns:
          A new GraphSet object.

        Raises:
          KeyError: If a given edge or vertex is not found in the universe.
        """
        try:  # if edge
            return setset.exclude(self, GraphSet._conv_edge(edge_or_vertex))
        except KeyError:  # else
            return self - self.include(edge_or_vertex)

    def add(self, graph_or_edge):
        """Adds a given graph or edge to `self` GraphSet.

        If a graph is given, the graph is just added to `self`
        GraphSet.  If an edge is given, the edge is added to all the
        graphs in `self` GraphSet.

        The `self` GraphSet is changed.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> gs.add((1,2))
          >>> gs
          GraphSet([set([(1,2), (1,4)]), set([(1,2), (2,3)])])

        Args:
          graph_or_edge: A graph (a set of edges) or an edge in the universe.

        Returns:
          None.

        Raises:
          KeyError: If given edges are not found in the universe.
        """
        graph_or_edge = GraphSet._conv_arg(graph_or_edge)
        return setset.add(self, graph_or_edge)

    def remove(self, graph_or_edge):
        """Removes a given graph or edge from `self` GraphSet.

        If a graph is given, the graph is just removed from `self`
        GraphSet.  If an edge is given, the edge is removed from all
        the graphs in `self` GraphSet.

        The `self` GraphSet is changed.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> gs.remove((1,2))
          >>> gs
          GraphSet([set([(1,4)]), set([(2,3)])])

        Args:
          graph_or_edge: A graph (a set of edges) or an edge in the universe.

        Returns:
          None.

        Raises:
          KeyError: If given edges are not found in the universe, or
            if the given graph is not stored in `self` GraphSet.
        """
        graph_or_edge = GraphSet._conv_arg(graph_or_edge)
        return setset.remove(self, graph_or_edge)

    def discard(self, graph_or_edge):
        """Removes a given graph or edge from `self` GraphSet.

        If a graph is given, the graph is just removed from `self`
        GraphSet.  If an edge is given, the edge is removed from all
        the graphs in `self` GraphSet.

        The `self` GraphSet is changed.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> gs.discard((1,2))
          >>> gs
          GraphSet([set([(1,4)]), set([(2,3)])])

        Args:
          graph_or_edge: A graph (a set of edges) or an edge in the universe.

        Returns:
          None.

        Raises:
          KeyError: If given edges are not found in the universe.
        """
        graph_or_edge = GraphSet._conv_arg(graph_or_edge)
        return setset.discard(self, graph_or_edge)

    def invert(self, edge):
        """Returns a new set of graphs by inverting a given edge.

        If a graph in `self` GraphSet includes the given edge, the
        edge is removed from the graph.  If a graph in `self` GraphSet
        does not include the given edge, the edge is added to the
        graph.

        The `self` GraphSet is not changed.

        Examples:
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> gs = gs.invert((1,2))
          >>> gs
          GraphSet([set([(1,4)]), set([(1,2), (2,3)])])

        Args:
          edge: An edge in the universe.

        Returns:
          A new GraphSet object.

        Raises:
          KeyError: If a given edge is not found in the universe.
        """
        edge = GraphSet._conv_edge(edge)
        return setset.invert(self, edge)

    def maximize(self):
        """Iterates over graphs in the descending order of weights.

        Returns a generator that iterates over graphs in `self`
        GraphSet.  The graphs are selected in the descending order of
        weights, which are specified with the universe (or 1 if
        omitted).

        Examples:
          >>> GraphSet.universe([(1,2, 2.0), (1,4, 3.0), (2,3, 4.0])
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> for g in gs.maximize():
          ...   g
          set([(1, 2), (1, 4)])
          set([(2, 3)])

        Returns:
          A generator.

        Yields:
          A graph.
        """
        for s in setset.maximize(self, GraphSet._weights):
            yield s

    def minimize(self):
        """Iterates over graphs in the ascending order of weights.

        Returns a generator that iterates over graphs in `self`
        GraphSet.  The graphs are selected in the ascending order of
        weights, which are specified with the universe (or 1 if
        omitted).

        Examples:
          >>> GraphSet.universe([(1,2, 2.0), (1,4, 3.0), (2,3, 4.0])
          >>> gs = GraphSet([set([(1,2), (1,4)]), set([(2,3)])])
          >>> for g in gs.minimize():
          ...   g
          set([(2, 3)])
          set([(1, 2), (1, 4)])

        Returns:
          A generator.

        Yields:
          A graph.
        """
        for s in setset.minimize(self, GraphSet._weights):
            yield s

    @staticmethod
    def universe(universe=None, traversal=None, source=None):
        """Registers or returns the universe.

        If `universe` is given, it is registered as a new universe.
        Otherwise, the list of edges that represents the current
        universe is returned.

        Args:
          universe: Optional.  The list of edges that represents the
            current universe.

          traversal: Optional.  This argument specifies the order of
            edges to be processed in the internal graphset operations.
            The default is 'bfs', the breadth-first search from
            `source`.  Other options include 'dfs', the depth-first
            search, and 'as-is', the order of `universe` list.

          source: Optional.  This argument specifies the starting
            point of the edge traversal.

        Returns:
          The universe if no argument is given, or None otherwise.
        """
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
