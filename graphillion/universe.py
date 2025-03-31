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

import _graphillion
import heapq

"""
ObjectTable class manages the universe of each class such as GraphSet and VertexSetSet.
"""
class ObjectTable:

    # Objects such as edges and vertices are
    # associated with integers 1,...,n.
    # self.int2obj[0] is dummy.
    # To obtain the set of objects,
    # use the universe() method.
    def __init__(self):
        self.obj2int = {}
        self.int2obj = [None]

    def num_elems(self):
        return len(self.int2obj) - 1

    def check_universe(self):
        for e, i in self.obj2int.items():
            assert e == self.int2obj[i]
        for i in range(1, len(self.int2obj)):
            e = self.int2obj[i]
            assert i == self.obj2int[e]

    def universe(self):
        return self.int2obj[1:]

    def add_elem(self, elem):
        assert elem not in self.obj2int
        if len(self.obj2int) >= _graphillion._elem_limit():
            m = 'too many elements are set, which must be {} or less'.format(_graphillion._elem_limit())
            raise RuntimeError(m)
        i = len(self.int2obj)
        _graphillion.setset([set([i])])
        self.obj2int[elem] = i
        self.int2obj.append(elem)
        assert self.int2obj[i] == elem
        assert self.obj2int[elem] == i

    def conv_elem(self, elem):
        if elem not in self.obj2int:
            self.add_elem(elem)
        return self.obj2int[elem]

    def conv_arg(self, obj):
        if isinstance(obj, (set, frozenset)):  # a set
            return set([self.conv_elem(e) for e in obj])
        else:  # an element
            return self.conv_elem(obj)

    def conv_ret(self, obj):
        if isinstance(obj, (set, frozenset)):  # a set
            ret = set()
            for e in obj:
                ret.add(self.int2obj[e])
            return ret
        raise TypeError(obj)

class Universe:
    """Manager class of universes.

    The universes of setset and DiGraphSet classes are managed in their
    respective classes.
    """

    vertices = set()
    isolated_vertices = set()
    weights = {}
    edge_vertex_kind = {}

    e_objtable = ObjectTable()
    v_objtable = ObjectTable()
    ev_objtable = ObjectTable()

    @staticmethod
    def edge_universe():
        """Returns the current universe.

        The list of edges that represents the current universe is
        returned.

        Examples:
          >>> GraphSet.universe()
          [(1, 2, 2.0), (1, 4, -3.0), (2, 3)]

        Returns:
          The universe if no argument is given, or None otherwise.

        See Also:
          set_universe()
        """
        from graphillion import GraphSet  # avoid circular import
        edges = []
        for e in Universe.e_objtable.universe():
            if e in Universe.weights:
                edges.append((e[0], e[1], Universe.weights[e]))
            else:
                edges.append(e)
        return GraphSet.converters['to_graph'](edges)

    @staticmethod
    def vertex_universe():
        """Returns the current universe.

        The list of vertices that represents the current universe is
        returned.

        Examples:
          >>> VertexSetSet.universe()
          [1, 2, (3, 10)]

        Returns:
          The universe of VertexSetSet class.

        See Also:
          set_universe()
        """
        vertices = []

        for v in Universe.v_objtable.int2obj[1:]:
            if v in Universe.weights and Universe.weights[v] != 1:
                vertices.append((v, Universe.weights[v]))
            else:
                vertices.append(v)
        return vertices

    @staticmethod
    def edge_vertex_universe():
        """Returns the current universe.

        The list of edges/vertices that represents the current universe is
        returned. Note that the retutrned list of universe() includes
        edges/vertices while a list of set_universe() passed as arguments
        includes edges (does not include vertices).

        Examples:
          >>> Universe.set_universe([(1, 2), (1, 4), (2, 3)])
          >>> Universe.edge_vertex_universe()
          [(1, 2), (1, 4), 1, 4, (2, 3), 2, 3]

        Returns:
          The universe if no argument is given, or None otherwise.

        See Also:
          set_universe()
        """
        return Universe.ev_objtable.universe()

    @staticmethod
    def set_universe(universe, traversal='greedy', source=None, weights=None, isolated=None):
        """Registers the new universe.

        Examples:
          >>> GraphSet.set_universe([(1, 2, 2.0), (1, 4, -3.0), (2, 3)])

        Args:
          universe: A list of edges that represents the new universe.
            An edge may come along with an edge weight, which can be
            positive as well as negative (or 1.0 if not specified).

          traversal: Optional.  This argument specifies the order of
            edges to be processed in the internal graphset operations.
            The default is 'greedy', best-first search from `source`
            with respect to the number of unused incident edges.
            Other options include 'bfs', the breadth-first search, 
            'dfs', the depth-first search, and 'as-is', the order of
            `universe` list.

          source: Optional.  This argument specifies the starting
            point of the edge traversal.

          weights: Optional.  This argument specifies the weights of
            edges/vertices.  The default is None, which means that
            all edges and vertices have the same weight of 1.0.
            The edge weights specified by the third element of an
            edge tuple take priority.
        
          isolated: Optional.  This argument specifies the list of
            isolated vertices.  The default is None, which means that
            all vertices are not isolated. The vertices in the list
            must not included in 'universe'.

        See Also:
          universe()
        """
        if len(universe) == 0:
            raise ValueError("The universe must not be empty.")
        sorted_edges = []
        indexed_edges = set()
        Universe.vertices = set()
        Universe.weights = {}
        from graphillion import GraphSet  # avoid circular import
        universe = GraphSet.converters['to_edges'](universe)
        for e in universe:
            if e[:2] in indexed_edges or (e[1], e[0]) in indexed_edges:
                raise KeyError(e)
            sorted_edges.append(e[:2])
            indexed_edges.add(e[:2])
            if len(e) > 2:
                Universe.weights[e[:2]] = e[2]
            elif weights is not None and e in weights:
                Universe.weights[e] = weights[e]

        if traversal != 'as-is':
            if source is None:
                source = sorted_edges[0][0]
                for e in sorted_edges:
                    source = min(e[0], e[1], source)
            sorted_edges = Universe._traverse(indexed_edges, traversal, source)
        for u, v in sorted_edges:
            Universe.vertices.add(u)
            Universe.vertices.add(v)
        #setset.set_universe(sorted_edges)
        Universe.e_objtable = ObjectTable()
        for e in sorted_edges:
            Universe.e_objtable.add_elem(e)

        from graphillion.setset_base import setset_base
        vertex_universe = [eval(v) for v in setset_base.get_vertices_from_top(Universe.e_objtable)]
        Universe.set_vertex_universe(vertex_universe, weights)

        Universe._set_ev_universe(universe)

        if isolated is None:
            Universe.isolated_vertices = set()
        else:
            Universe.isolated_vertices = isolated.copy()

    @staticmethod
    def set_vertex_universe(universe, weights=None):
        """Registers the new vertex universe.
        Normally, there is no need to call this method because the universe
        of vertices is automatically set when the universe of edges is set
        using the set_universe method. Only call this method after calling
        the set_universe method if you want to explicitly specify the
        universe of vertices.

        Examples:
          >>> # This example sets the universe as a vertex set {1, 2, 3},
          >>> # with the vertex 3 having a weight of 10.
          >>> VertexSetSet.set_universe([1, (2), (3, 10)])

        Args:
          universe: A list of vertices that represents the new universe.
            A vertex may come along with a vertex weight, which can be
            positive as well as negative (or 1.0 if not specified).
            If a vertex is weighted, it must be given in a list as a tuple of vertex and weight.
            If a vertex is not weighted, it must be given as a tuple of size one or single value.

          weights: Optional.  This argument specifies the weights of
            vertices.  The default is None, which means that
            all vertices have the same weight of 1.0.
            The vertex weights specified by the second element of a
            vertex tuple take priority.

        See Also:
          universe()
        """

        vertex_num = len(universe)
        if vertex_num != len(set(universe)):
            raise ValueError("duplicated elements found")

        Universe.v_objtable = ObjectTable()
        if not Universe.weights:
            Universe.weights = {}

        for i, vertex in enumerate(universe):
            if isinstance(vertex, tuple):
                if len(vertex) == 2:
                    vertex, weight = vertex
                    Universe.weights[vertex] = weight
                elif len(vertex) == 1:
                    vertex = vertex[0]
                    if weights is not None and vertex in weights:
                        Universe.weights[vertex] = weights[vertex]
                    else:
                        Universe.weights[vertex] = 1
                else:
                    raise TypeError(vertex)
            else:
                if weights is not None and vertex in weights:
                    Universe.weights[vertex] = weights[vertex]
                else:
                    Universe.weights[vertex] = 1
            Universe.v_objtable.add_elem(vertex)

    @staticmethod
    def _set_ev_universe(universe):
        visited_vertices = set()
        edges_vertices = []
        Universe.edge_vertex_kind = {}
        for e in reversed(universe):
            for v in [e[1], e[0]]:
                if v not in visited_vertices:
                    visited_vertices.add(v)
                    edges_vertices.append(v)
                    Universe.edge_vertex_kind[v] = False
            # check if a vertex such as "(a, b)" exists
            if e in visited_vertices:
                KeyError("{} cannot be a vertex".format(e))
            else:
                edges_vertices.append(e[:2])
                Universe.edge_vertex_kind[e[:2]] = True
                Universe.edge_vertex_kind[(e[1], e[0])] = True

        edges_vertices.reverse()

        Universe.ev_objtable = ObjectTable()
        for x in edges_vertices:
            Universe.ev_objtable.add_elem(x)

    @staticmethod
    def _traverse(indexed_edges, traversal, source):
        neighbors = {}
        for u, v in indexed_edges:
            if u not in neighbors:
                neighbors[u] = set([v])
            else:
                neighbors[u].add(v)
            if v not in neighbors:
                neighbors[v] = set([u])
            else:
                neighbors[v].add(u)
        assert source in neighbors
        vertices = set(neighbors.keys())

        sorted_edges = []
        visited_vertices = set()
        u = source

        if traversal == 'greedy':
            degree = dict()
            for v in vertices:
                degree[v] = len(neighbors[v])

            heap = []
            while True:
                visited_vertices.add(u)
                for v in sorted(neighbors[u]):
                    degree[v] -= 1
                    if v in visited_vertices:
                        degree[u] -= 1
                        e = (u, v) if (u, v) in indexed_edges else (v, u)
                        sorted_edges.append(e)
                        if degree[v]:
                            for w in sorted(neighbors[v]):
                                if w not in visited_vertices:
                                    heapq.heappush(heap, (degree[v], degree[w], w))
                for v in sorted(neighbors[u]):
                    if v not in visited_vertices:
                         heapq.heappush(heap, (degree[u], degree[v], v))
                if visited_vertices == vertices:
                    break
                while u in visited_vertices:
                    if not heap:
                        u = min(vertices - visited_vertices)
                    else:
                        u = heapq.heappop(heap)[2]
            assert set(indexed_edges) == set(sorted_edges)
            return sorted_edges
        elif traversal == 'bfs' or traversal == 'dfs':
            queue_or_stack = []
            while True:
                visited_vertices.add(u)
                for v in sorted(neighbors[u]):
                    if v in visited_vertices:
                        e = (u, v) if (u, v) in indexed_edges else (v, u)
                        sorted_edges.append(e)
                new_vertices = neighbors[u] - visited_vertices - set(queue_or_stack)
                queue_or_stack.extend(sorted(new_vertices))
                if not queue_or_stack:
                    if visited_vertices == vertices:
                        break
                    else:
                        queue_or_stack.append(min(vertices - visited_vertices))
                if traversal == 'bfs':
                    u, queue_or_stack = queue_or_stack[0], queue_or_stack[1:]
                else:
                    queue_or_stack, u = queue_or_stack[:-1], queue_or_stack[-1]
                assert u not in visited_vertices
            assert set(indexed_edges) == set(sorted_edges)
            return sorted_edges
        else:
            raise ValueError('invalid `traversal`: {}'.format(traversal))
