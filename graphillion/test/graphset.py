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

from builtins import range
from graphillion import GraphSet
import tempfile
import unittest


e1 = (1,2)
e2 = (1,3)
e3 = (2,4)
e4 = (3,4)

g0 = []
g1 = [e1]
g2 = [e2]
g3 = [e3]
g4 = [e4]
g12 = [e1, e2]
g13 = [e1, e3]
g14 = [e1, e4]
g23 = [e2, e3]
g24 = [e2, e4]
g34 = [e3, e4]
g123 = [e1, e2, e3]
g124 = [e1, e2, e4]
g134 = [e1, e3, e4]
g234 = [e2, e3, e4]
g1234 = [e1, e2, e3, e4]


class TestGraphSet(unittest.TestCase):

    def setUp(self):
        GraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                              traversal='bfs', source=1)

    def tearDown(self):
        pass

    def test_init(self):
        GraphSet.set_universe([('i', 'ii')])
        self.assertEqual(GraphSet.universe(), [('i', 'ii')])

        GraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                              traversal='bfs', source=1)
        self.assertEqual(GraphSet.universe(),
                         [e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)])

        GraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                              traversal='dfs', source=1)
        self.assertEqual(GraphSet.universe(),
                         [e2 + (-.2,), e4 + (.4,), e1 + (.3,), e3 + (-.2,)])

        GraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                              traversal='greedy', source=3)
        self.assertEqual(GraphSet.universe(),
                         [e2 + (-.2,), e1 + (.3,), e3 + (-.2,), e4 + (.4,)])

        self.assertRaises(KeyError, GraphSet.set_universe, [(1,2), (2,1)])

        GraphSet.set_universe([(1,2), (3,4)])  # disconnected graph
        self.assertEqual(GraphSet.universe(), [(1,2), (3,4)])

    def test_constructors(self):
        gs = GraphSet()
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(len(gs), 0)

        gs = GraphSet([])
        self.assertEqual(len(gs), 0)

        gs = GraphSet([g1, [(3,1)]])
        self.assertEqual(len(gs), 2)
        self.assertTrue(g1 in gs)
        self.assertTrue(g2 in gs)

        gs = GraphSet({})
        self.assertEqual(len(gs), 2**4)

        gs = GraphSet({'include': [e1, e2], 'exclude': [(4,3)]})
        self.assertEqual(len(gs), 2)
        self.assertTrue(g12 in gs)
        self.assertTrue(g123 in gs)

        self.assertRaises(KeyError, GraphSet, [(1,4)])
        self.assertRaises(KeyError, GraphSet, [[(1,4)]])
        self.assertRaises(KeyError, GraphSet, {'include': [(1,4)]})

        # copy constructor
        gs1 = GraphSet([g0, g12, g13])
        gs2 = gs1.copy()
        self.assertTrue(isinstance(gs2, GraphSet))
        gs1.clear()
        self.assertEqual(gs1, GraphSet())
        self.assertEqual(gs2, GraphSet([g0, g12, g13]))

        # repr
        gs = GraphSet([g0, g12, g13])
        self.assertEqual(
            repr(gs),
            "GraphSet([[], [(1, 2), (1, 3)], [(1, 2), (2, 4)]])")

        gs = GraphSet({})
        self.assertEqual(
            repr(gs),
            "GraphSet([[], [(1, 2)], [(1, 3)], [(2, 4)], [(3, 4)], [(1, 2), (1, 3)], [(1, ...")

    def test_graphs(self):
        GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5),
                               (5, 6)])

        # any subgraph
        gs = GraphSet.graphs()
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(len(gs), 2**7)
        self.assertTrue([(1, 2)] in gs)

        # subgraphs separating [1, 5] and [2]
        gs = GraphSet.graphs(vertex_groups=[[1, 5], [2]])
        self.assertEqual(len(gs), 6)
        self.assertTrue([(1, 4), (4, 5)] in gs)
        self.assertTrue([(1, 2), (1, 4), (4, 5)] not in gs)

        gs = GraphSet.graphs(vertex_groups=[[1], [2], [3], [4], [5], [6]])
        self.assertEqual(len(gs), 1)
        self.assertTrue([] in gs)
        self.assertTrue([(1, 2)] not in gs)

        # matching
        gs = GraphSet.matchings()
        self.assertEqual(len(gs), 22)
        self.assertTrue([(1, 2), (3, 6)] in gs)
        self.assertTrue([(1, 2), (2, 3), (3, 6)] not in gs)
        for g in gs:
            self.assertTrue(len(g) < 4)

        # k-matching
        gs = GraphSet.k_matchings(k=2)
        self.assertEqual(len(gs), 100)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (3, 6), (4, 5)] in gs)
        self.assertTrue([(1, 4), (2, 3), (2, 5), (3, 6), (4, 5), (5, 6)] not in gs)
        gs = GraphSet.k_matchings(k=3)
        self.assertEqual(gs, GraphSet({})) # power set

        # b-matching
        b = {}
        for v in GraphSet._vertices:
            if v != 1:
                b[v] = v % 2 + 1
        gs = GraphSet.b_matchings(b)
        self.assertEqual(len(gs), 17)
        self.assertTrue([(2, 5), (3, 6), (4, 5)] in gs)
        self.assertTrue([(1, 2), (2, 5), (3, 6), (4, 5)] not in gs)

        # k-factor
        gs = GraphSet.k_factors(k=1)
        # A 1-factor is equal to a perfect matching.
        self.assertEqual(gs, GraphSet.perfect_matchings())
        gs = GraphSet.k_factors(k=2)
        self.assertEqual(gs, GraphSet([[(1, 2), (1, 4), (2, 3),
                                        (3, 6), (4, 5), (5, 6)]]))
        gs = GraphSet.k_factors(k=3)
        self.assertEqual(len(gs), 0)

        # f-factor
        f = {}
        for v in GraphSet._vertices:
            if v == 2 or v == 5:
                f[v] = 2
            else:
                f[v] = 1
        gs = GraphSet.f_factors(f)
        g1 = [(1, 2), (2, 3), (4, 5), (5, 6)]
        g2 = [(1, 2), (2, 5), (3, 6), (4, 5)]
        g3 = [(1, 4), (2, 3), (2, 5), (5, 6)]
        self.assertEqual(gs, GraphSet([g1, g2, g3]))

        # subgraphs with 1 or 2 edges
        gs = GraphSet.graphs(num_edges=range(1, 3))
        self.assertEqual(len(gs), 28)
        for g in gs:
            self.assertTrue(1 <= len(g) and len(g) < 3)

        # single connected component and vertex islands
        gs = GraphSet.graphs(vertex_groups=[[]])
        self.assertEqual(len(gs), 80)
        self.assertTrue([(1, 2), (2, 3)] in gs)
        self.assertTrue([(1, 2), (2, 3), (4, 5)] not in gs)

        # any forest
        gs = GraphSet.graphs(no_loop=True)
        self.assertEqual(len(gs), 112)
        self.assertTrue([(1, 2), (1, 4), (2, 5)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (4, 5)] not in gs)
        for g in gs:
            self.assertTrue(len(g) < 6)

        # constrained by GraphSet
        gs = GraphSet.graphs(no_loop=True)
        gs = gs.graphs(vertex_groups=[[]])
        self.assertEqual(len(gs), 66)
        self.assertTrue([(1, 2), (1, 4), (2, 5)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (4, 5)] not in gs)

        # single connected components across 1, 3, and 5
        gs = GraphSet.connected_components([1, 3, 5])
        self.assertEqual(len(gs), 35)
        self.assertTrue([(1, 2), (2, 3), (2, 5)] in gs)
        self.assertTrue([(1, 2), (2, 3), (5, 6)] not in gs)

        GraphSet.set_universe([(1, 2), (1, 3), (1, 4), (1, 5), (2, 3), (2, 4),
                               (2, 5), (3, 4), (3, 5), (4, 5)])

        # cliques with 4 vertices
        gs = GraphSet.cliques(4)
        self.assertEqual(len(gs), 5)
        self.assertTrue([(1, 2), (1, 3), (1, 4), (2, 3), (2, 4), (3, 4)] in gs)
        self.assertTrue([(1, 2), (1, 3), (1, 4), (2, 3), (2, 4), (3, 5)] not in gs)

        GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5),
                               (5, 6)])

        # trees rooted at 1
        gs = GraphSet.trees(1)
        self.assertEqual(len(gs), 45)
        self.assertTrue([] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (4, 5)] not in gs)

        # spanning trees
        gs = GraphSet.trees(is_spanning=True)
        self.assertEqual(len(gs), 15)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5), (4, 5)] not in gs)
        for g in gs:
            self.assertEqual(len(g), 5)

        # forests rooted at 1 and 3
        gs = GraphSet.forests([1, 3])
        self.assertEqual(len(gs), 54)
        self.assertTrue([] in gs)
        self.assertTrue([(1, 2), (2, 3)] not in gs)

        # spanning forests rooted at 1 and 3
        gs = GraphSet.forests([1, 3], is_spanning=True)
        self.assertEqual(len(gs), 20)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (3, 6)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5)] not in gs)
        for g in gs:
            self.assertEqual(len(g), 4)

        # cycles
        gs = GraphSet.cycles()
        self.assertEqual(len(gs), 3)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (4, 5)] in gs)
        self.assertTrue([] not in gs)

        # hamilton cycles
        gs = GraphSet.cycles(is_hamilton=True)
        self.assertEqual(len(gs), 1)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (3, 6), (4, 5), (5, 6)] in gs)

        # paths between 1 and 6
        gs = GraphSet.paths(1, 6)
        self.assertEqual(len(gs), 4)
        self.assertTrue([(1, 2), (2, 3), (3, 6)] in gs)
        self.assertTrue([(1, 2), (2, 3), (5, 6)] not in gs)

        # paths not specifying endpoints
        gs = GraphSet.paths()
        self.assertEqual(len(gs), 49)
        self.assertTrue([(1, 4), (2, 3), (3, 6), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 4), (2, 5), (3, 6), (4, 5), (5, 6)] not in gs)

        gs = GraphSet.paths(1)
        self.assertEqual(len(gs), 17)
        self.assertTrue([(1, 4), (2, 3), (3, 6), (4, 5), (5, 6)] in gs)
        self.assertTrue([(2, 3), (3, 6), (4, 5), (5, 6)] not in gs)
        self.assertTrue([(1, 4), (2, 5), (3, 6), (4, 5), (5, 6)] not in gs)

        # hamilton paths between 1 and 6
        gs = GraphSet.paths(1, 6, is_hamilton=True)
        self.assertEqual(len(gs), 1)
        self.assertTrue([(1, 4), (2, 3), (2, 5), (3, 6), (4, 5)] in gs)

        # hamilton paths not specifying endpoints
        gs = GraphSet.paths(is_hamilton=True)
        self.assertEqual(len(gs), 8)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (3, 6), (4, 5)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (4, 5)] not in gs)

        gs = GraphSet.paths(1, is_hamilton=True)
        self.assertEqual(len(gs), 3)
        self.assertTrue([(1, 4), (2, 3), (3, 6), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (3, 6), (4, 5)] not in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (4, 5)] not in gs)

        # perfect matchings
        gs = GraphSet.perfect_matchings()
        self.assertEqual(len(gs), 3)
        self.assertTrue([(1, 2), (3, 6), (4, 5)] in gs)
        self.assertTrue([(1, 4), (2, 3), (5, 6)] in gs)
        self.assertTrue([(1, 4), (2, 5), (3, 6)] in gs)
        
        gs = GraphSet.perfect_matchings(graphset=GraphSet.forests(roots=[1, 2, 3]))
        self.assertEqual(len(gs), 1)
        self.assertTrue([(1, 4), (2, 5), (3, 6)] in gs)

        # called as instance methods
        gs = GraphSet.graphs(no_loop=True)
        _ = gs.connected_components([1, 3, 5])
        _ = gs.cliques(4)
        _ = gs.trees(1)
        _ = gs.forests([1, 3])
        _ = gs.cycles()
        _ = gs.paths(1, 6)
        _ = gs.perfect_matchings()

        # exceptions
        self.assertRaises(KeyError, GraphSet.graphs, vertex_groups=[[7]])
        self.assertRaises(KeyError, GraphSet.graphs, degree_constraints={7: 1})

    def test_combined_cases(self):
        GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5),
                               (5, 6)])

        # spanning tree = (forests) & (spanning)
        gs1 = GraphSet.forests([1])
        gs2 = GraphSet.connected_components([1, 2, 3, 4, 5, 6])
        gs3 = GraphSet.trees(is_spanning=True)
        self.assertEqual(gs3, gs1 & gs2)

        # all hamilton paths are spanning connected components
        gs1 = GraphSet.paths(1, 6, is_hamilton=True)
        gs2 = GraphSet.connected_components([1, 2, 3, 4, 5, 6])
        self.assertTrue(gs1.issubset(gs2))

    def test_linear_constraints(self):
        GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5),
                               (5, 6)])

        gs = GraphSet.graphs(linear_constraints=[([(2, 3)], (2, 1))])
        self.assertEqual(gs, GraphSet())

        gs = GraphSet.graphs(linear_constraints=[([], (1, 2))])
        self.assertEqual(gs, GraphSet())

        gs = GraphSet.graphs(linear_constraints=[([(5, 6, -1.5)], (-1.5, 0))])
        self.assertEqual(gs, GraphSet.graphs())

        gs = GraphSet.graphs(linear_constraints=[([(1, 2, 3.14)], (0, float("inf")))])
        self.assertEqual(gs, GraphSet.graphs())

        gs = GraphSet.graphs(linear_constraints=[(GraphSet.universe(), (0, float("inf")))])
        self.assertEqual(gs, GraphSet.graphs())

        gs = GraphSet.graphs(linear_constraints=[(GraphSet.universe(), (7, 7))])
        self.assertEqual(gs, GraphSet([GraphSet.universe()]))

        gs = GraphSet.graphs(linear_constraints=[(GraphSet.universe(), (1, 7))])
        self.assertEqual(gs, GraphSet.graphs() - GraphSet([[]]))

        gs = GraphSet.graphs(linear_constraints=[([(2, 3, -1), (2, 5, 10), (4, 1),
                                                   (3, 6), (4, 5, -1), (5, 6)],
                                                  (10, 100))])
        self.assertEqual(len(gs), 52)

        gs = GraphSet.graphs(linear_constraints=[([(2, 3), (2, 5, -10), (4, 1, -1),
                                                   (3, 6, -1), (4, 5), (5, 6, -1)],
                                                  (-100, -10))])
        self.assertEqual(len(gs), 52)

        gs = GraphSet.graphs(linear_constraints=[([(1, 2)], (1, 1)),
                                                 ([(1, 4)], (1, 2)),
                                                 ([(2, 3)], (1, 3)),
                                                 ([(2, 5)], (1, 4)),
                                                 ([(4, 5)], (1, 6)),
                                                 ([(5, 6)], (1, 7))])
        self.assertEqual(len(gs), 2)

    def test_show_messages(self):
        a = GraphSet.show_messages()
        b = GraphSet.show_messages(True)
        self.assertTrue(b)
        c = GraphSet.show_messages(False)
        self.assertTrue(c)
        d = GraphSet.show_messages(a)
        self.assertFalse(d)

    def test_bicliques(self):
        GraphSet.set_universe([(1, 2), (1, 3), (1, 4), (2, 3), (2, 4), (3, 4)])

        gs = GraphSet.bicliques(2, 2)
        self.assertEqual(len(gs), 3)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (3, 4)] in gs)
        self.assertTrue([(1, 2)] not in gs)

    def test_regular_graphs(self):
        GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5),
                               (5, 6)])

        gs = GraphSet.regular_graphs(is_connected=False)
        self.assertEqual(len(gs), 24)
        self.assertTrue([(2, 3), (2, 5), (5, 6), (3, 6)] in gs)
        self.assertTrue([(2, 5), (1, 4), (3, 6)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (4, 5), (3, 6)] not in gs)

        gs = GraphSet.regular_graphs(is_connected=True)
        self.assertEqual(len(gs), 10)
        self.assertTrue([(2, 3), (2, 5), (5, 6), (3, 6)] in gs)
        self.assertTrue([(2, 5), (1, 4), (3, 6)] not in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (4, 5), (3, 6)] not in gs)

        universe = []
        for i in range(1, 6):
            for j in range(i + 1, 6):
                universe.append((i, j))
        GraphSet.set_universe(universe)
        gs = GraphSet.regular_graphs(degree=(2, 3), is_connected=True)
        self.assertEqual(len(gs), 42)
        self.assertTrue([(2, 4), (2, 5), (3, 4), (3, 5)] in gs)
        self.assertTrue([(1, 2)] not in gs)

    def test_regular_bipartite_graphs(self):
        GraphSet.set_universe([(1, 2), (1, 4), (1, 5), (2, 3), (2, 5),
                               (3, 6), (4, 5), (5, 6)])

        gs = GraphSet.regular_bipartite_graphs(is_connected=False)
        self.assertEqual(len(gs), 27)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (3, 6), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 4), (2, 5), (3, 6)] in gs)
        self.assertTrue([(1, 2), (1, 5), (2, 3), (3, 6), (5, 6)] not in gs)

        gs = GraphSet.regular_bipartite_graphs(is_connected=True)
        self.assertEqual(len(gs), 11)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (3, 6), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 4), (2, 5), (3, 6)] not in gs)
        self.assertTrue([(1, 2), (1, 5), (2, 3), (3, 6), (5, 6)] not in gs)

    def test_steiner(self):
        GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (2, 5), (4, 5), (4, 7),
                                (3, 6), (5, 6), (5, 8), (6, 9), (7, 8), (8, 9)])
        terminals = [1, 3, 8]

        gs = GraphSet.steiner_subgraphs(terminals=terminals)
        self.assertEqual(len(gs), 830)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5), (4, 5), (5, 8)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5), (4, 5)] not in gs)

        gs = GraphSet.steiner_trees(terminals=terminals)
        self.assertEqual(len(gs), 438)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (4, 7), (7, 8), (8, 9)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5), (4, 5), (5, 8)] not in gs)

        gs = GraphSet.steiner_cycles(terminals=terminals)
        self.assertEqual(len(gs), 3)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (3, 6), (4, 7), (5, 6), (5, 8), (7, 8)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (4, 7), (7, 8), (8, 9)] not in gs)

        gs = GraphSet.steiner_paths(terminals=terminals)
        self.assertEqual(len(gs), 73)
        self.assertTrue([(1, 2), (2, 3), (3, 6), (5, 6), (5, 8), (7, 8)] in gs)
        self.assertTrue([(1, 2), (2, 3), (3, 6), (5, 6), (5, 8), (7, 8), (8, 9)] not in gs)


    def test_partitions(self):
        GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5),
                               (5, 6)])

        # lower bound and upper bound
        gs = GraphSet.partitions(num_comp_lb=2, num_comp_ub=3)
        self.assertEqual(len(gs), 44)
        self.assertTrue([(1, 2), (2, 3), (2, 5)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5), (4, 5)] in gs)
        self.assertTrue([(1, 2), (2, 3)] not in gs)

        gs = GraphSet.partitions(num_comp_lb=1, num_comp_ub=1)
        self.assertEqual(len(gs), 1)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (4, 5), (3, 6)] not in gs)
        self.assertTrue([(1, 2), (3, 6), (4, 5), (5, 6)] not in gs)

        gs = GraphSet.partitions(num_comp_lb=2, num_comp_ub=2)
        self.assertEqual(len(gs), 15)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (4, 5), (3, 6)] in gs)
        self.assertTrue([(1, 2), (3, 6), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5), (5, 6)] not in gs)

        # only upper bound
        gs = GraphSet.partitions(num_comp_ub=2)
        self.assertEqual(len(gs), 1 + 15)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (4, 5), (3, 6)] in gs)
        self.assertTrue([(1, 2), (3, 6), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 4), (2, 5), (3, 6)] not in gs)

        # only lower bound
        gs = GraphSet.partitions(num_comp_lb=2)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (4, 5), (3, 6)] in gs)
        self.assertTrue([(1, 2), (3, 6), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5), (5, 6)] not in gs)
        self.assertTrue([(1, 4), (2, 5), (3, 6)] in gs)

    def test_balanced_partitions(self):
        GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5),
                               (5, 6)])

        gs = GraphSet.balanced_partitions(ratio=1.0, num_comps=3)
        self.assertEqual(len(gs), 3)
        self.assertTrue([(1, 4), (2, 3), (5, 6)] in gs)

        gs = GraphSet.balanced_partitions(ratio=1.0)
        self.assertEqual(len(gs), 8)
        self.assertTrue([] in gs)
        self.assertTrue([(1, 2), (1, 4), (3, 6), (5, 6)] in gs)

        gs = GraphSet.balanced_partitions(ratio=1.0, lower=2, upper=3)
        self.assertEqual(len(gs), 6)
        self.assertTrue([] not in gs)

        gs = GraphSet.balanced_partitions(ratio=2.0)
        self.assertTrue([(2, 3), (5, 6)] in gs)
        self.assertTrue([(1, 2), (2, 3)] not in gs)

        gs = GraphSet.balanced_partitions(lower=1, upper=2, num_comps=4)
        self.assertTrue([(1, 2), (3, 6)] in gs)
        self.assertTrue([(1, 2), (2, 3)] not in gs)

        wl = {}
        for v in range(1, 7):
            if v % 2:
                wl[v] = 1
            else:
                wl[v] = 2

        gs = GraphSet.balanced_partitions(weight_list=wl, ratio=2)
        self.assertTrue([(2, 3), (5, 6)] not in gs)
        self.assertTrue([(1, 2), (2, 3), (5, 6)] in gs)

        gs = GraphSet.balanced_partitions(weight_list=wl, lower=4)
        self.assertTrue([(1, 2), (1, 4), (3, 6), (5, 6)] in gs)
        self.assertTrue([(1, 4), (2, 5), (3, 6)] not in gs)

        gs = GraphSet.balanced_partitions(weight_list=wl, upper=3)
        self.assertTrue([(1, 4), (2, 5), (3, 6)] in gs)
        self.assertTrue([(1, 2), (1, 4), (3, 6), (5, 6)] not in gs)

        gs = GraphSet.balanced_partitions(weight_list=wl, lower=3, upper=4)
        self.assertTrue([(1, 4), (2, 5), (3, 6)] in gs)
        self.assertTrue([(1, 2), (1, 4), (3, 6), (5, 6)] not in gs)
    def test_reliability(self):
        GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5),
                               (5, 6)])

        # calculated by hand
        probabilities = {(1, 2): 0.5}
        terminals = [1]
        reliability = GraphSet.reliability(probabilities, terminals)
        self.assertTrue(abs(1.0 - reliability) < 1e-9)

        probabilities = {(1, 2): 0.5, (1, 4): 0.5}
        terminals = [1, 2, 3, 4, 5, 6]
        reliability = GraphSet.reliability(probabilities, terminals)
        self.assertTrue(abs(0.75 - reliability) < 1e-9)

        # calculated by brute-force search
        probabilities = {(1, 2): 0.5, (1, 4): 0.5, (2, 3): 0.5,
                         (2, 5): 0.5, (3, 6): 0.5, (4, 5): 0.5, (5, 6): 0.5}
        terminals = [1, 2, 3, 4, 5, 6]
        reliability = GraphSet.reliability(probabilities, terminals)
        self.assertTrue(abs(0.1796875 - reliability) < 1e-9)

        probabilities = {(1, 2): 0.2, (1, 4): 0.3, (2, 3): 0.4, (2, 5): 0.5,
                         (3, 6): 0.6, (4, 5): 0.7, (5, 6): 0.8}
        terminals = [1, 2, 3, 4, 5, 6]
        reliability = GraphSet.reliability(probabilities, terminals)
        self.assertTrue(abs(0.1479680 - reliability) < 1e-9)

        reliability = GraphSet.reliability(None, None)
        self.assertEqual(reliability, 1.0)

        terminals = []
        reliability = GraphSet.reliability(None, terminals)
        self.assertEqual(reliability, 1.0)

        probabilities = {}
        reliability = GraphSet.reliability(probabilities, None)
        self.assertEqual(reliability, 1.0)
    def test_induced_graphs(self):
        GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5),
                               (5, 6)])

        gs = GraphSet.induced_graphs()

        self.assertEqual(len(gs), 34)  # 1 + 6 + 10 + 10 + 7 + 0

        self.assertTrue([(3, 6)] in gs)

        self.assertTrue([(1, 2), (2, 3)] in gs)
        self.assertTrue([(1, 2), (3, 6)] not in gs)

        self.assertTrue([(2, 3), (2, 5), (3, 6), (5, 6)] in gs)
        self.assertTrue([(1, 2), (2, 3), (4, 5), (5, 6)] not in gs)

        self.assertTrue([(1, 2), (1, 4), (2, 3), (3, 6)] in gs)
        self.assertTrue([(1, 2), (2, 3), (3, 6), (5, 6)] not in gs)

        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5),
                         (3, 6), (4, 5), (5, 6)] in gs)

    def test_weighted_induced_graphs(self):
        GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5),
                               (5, 6)])

        gs = GraphSet.weighted_induced_graphs()
        self.assertEqual(len(gs), 34)

        # induced graphs with more than or equal to 5 vertices
        gs = GraphSet.weighted_induced_graphs(lower=5)
        self.assertEqual(len(gs), 7)
        self.assertTrue([(1, 4), (4, 5), (5, 6), (3, 6)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 2), (2, 3)] not in gs)

        gs = GraphSet.weighted_induced_graphs(upper=2)
        self.assertEqual(len(gs), 7)

        gs = GraphSet.weighted_induced_graphs(lower=3, upper=4)
        self.assertEqual(len(gs), 20)

        wl = {}
        for v in range(1, 7):
            wl[v] = v

        gs = GraphSet.weighted_induced_graphs(weight_list=wl)
        self.assertTrue(len(gs), 34)

        gs = GraphSet.weighted_induced_graphs(weight_list=wl, lower=18)
        self.assertEqual(len(gs), 5)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 2), (2, 3), (2, 5), (3, 6), (5, 6)] not in gs)

        gs = GraphSet.weighted_induced_graphs(weight_list=wl, upper=9)
        self.assertEqual(len(gs), 9) # 6 + 3
        self.assertTrue([(1, 2), (2, 3)] in gs)
        self.assertTrue([(1, 2), (2, 5)] in gs)
        self.assertTrue([(1, 2), (1, 4)] in gs)
        self.assertTrue([(2, 3), (2, 5)] not in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (4, 5), (5, 6)] not in gs)

        gs = GraphSet.weighted_induced_graphs(weight_list=wl, lower=10, upper=17)
        self.assertEqual(len(gs), 20) # 34 - 5 - 9

    # def test_chordal_graphs(self):
    #     # the number of chordal labeled graphs: https://oeis.org/A058862

    #     # K3
    #     GraphSet.set_universe([(1, 2), (1, 3), (2, 3)])
    #     gs = GraphSet.chordal_graphs()
    #     self.assertEqual(len(gs), 8)

    #     # K4
    #     GraphSet.set_universe([(1, 2), (1, 3), (1, 4), (2, 3), (2, 4), (3, 4)])
    #     gs = GraphSet.chordal_graphs()
    #     self.assertEqual(len(gs), 61)
    #     self.assertTrue([(1, 2), (1, 4), (2, 3), (3, 4)] not in gs)
    #     self.assertTrue([(1, 2), (1, 3), (1, 4), (2, 3), (3, 4)] in gs)

    #     # K6
    #     es = []
    #     for i in range(1, 8):
    #         for j in range(i + 1, 8):
    #             es.append((i, j))
    #     GraphSet.set_universe(es)
    #     gs = GraphSet.chordal_graphs()
    #     self.assertEqual(len(gs), 617675)
    #     self.assertTrue([(2, 3), (2, 5), (3, 4), (4, 5)] not in gs)
    #     self.assertTrue([(3, 4), (3, 5), (3, 6), (4, 5), (5, 6)] in gs)

    def test_bipartite_graphs(self):
        GraphSet.set_universe([(1, 2), (1, 3), (2, 3), (3, 4)])
        """
        1 --- 2
        |  /
        3 --- 4
        """
        gs = GraphSet.bipartite_graphs(is_connected=False)
        self.assertEqual(len(gs), 2 ** 4 - 2)
        self.assertTrue([(1, 2), (2, 3), (3, 4)] in gs)
        self.assertTrue([(1, 2), (3, 4)] in gs)
        self.assertTrue([] in gs)
        self.assertTrue([(1, 2), (1, 3), (2, 3)] not in gs)
        self.assertTrue([(1, 2), (1, 3), (2, 3), (3, 4)] not in gs)

        gs = GraphSet.bipartite_graphs(is_connected=True)
        self.assertEqual(len(gs), 13)
        self.assertTrue([(1, 2), (2, 3), (3, 4)] in gs)
        self.assertTrue([] in gs)
        self.assertTrue([(1, 2), (3, 4)] not in gs)
        self.assertTrue([(1, 2), (1, 3), (2, 3)] not in gs)
        self.assertTrue([(1, 2), (1, 3), (2, 3), (3, 4)] not in gs)

        graphset = GraphSet(
            [[(1, 2), (1, 3), (2, 3)], [(1, 2), (2, 3), (3, 4)]])
        gs = GraphSet.bipartite_graphs(is_connected=False, graphset=graphset)
        self.assertTrue([(1, 2), (2, 3), (3, 4)] in gs)
        self.assertTrue([(1, 2), (1, 3), (3, 4)] not in gs)

        GraphSet.set_universe([(1, 2), (2, 3), (3, 4), (4, 5)])
        """
        1 --- 2 --- 3 --- 4 --- 5
        """
        gs = GraphSet.bipartite_graphs(is_connected=False)
        self.assertEqual(len(gs), 16)

    def test_degree_distribution_graphs(self):
        GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5),
                               (5, 6)])

        deg_dist1 = {0: GraphSet.DegreeDistribution_Any, 1: 2, 2: 1}
        gs1 = GraphSet.degree_distribution_graphs(deg_dist1, False)

        self.assertEqual(len(gs1), 10)
        self.assertTrue([(1, 2), (1, 4)] in gs1)
        self.assertTrue([(1, 2), (3, 6)] not in gs1)

        gs2 = GraphSet.degree_distribution_graphs(deg_dist1, True)
        # same as the case of not connected

        self.assertEqual(len(gs2), 10)
        self.assertTrue([(1, 2), (1, 4)] in gs2)
        self.assertTrue([(1, 2), (3, 6)] not in gs2)

        deg_dist2 = {0: GraphSet.DegreeDistribution_Any, 1: 4,
                     3: GraphSet.DegreeDistribution_Any} # 2: 0
        gs3 = GraphSet.degree_distribution_graphs(deg_dist2, False)

        self.assertEqual(len(gs3), 12)

        self.assertTrue([(1, 2), (2, 3), (2, 5), (4, 5), (5, 6)] in gs3)
        self.assertTrue([(1, 2), (3, 6)] in gs3)
        self.assertTrue([(1, 2), (1, 4)] not in gs3)

        gs4 = GraphSet.degree_distribution_graphs(deg_dist2, True)

        self.assertEqual(len(gs4), 1)

        self.assertTrue([(1, 2), (2, 3), (2, 5), (4, 5), (5, 6)] in gs4)
        self.assertTrue([(1, 2), (3, 6)] not in gs4)
        self.assertTrue([(1, 2), (1, 4)] not in gs4)

    def test_letter_P_graphs(self):
        GraphSet.set_universe([(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5),
                               (5, 6)])

        gs = GraphSet.letter_P_graphs()

        self.assertEqual(len(gs), 8)

        self.assertTrue([(1, 2), (1, 4), (2, 3), (2, 5), (4, 5)] in gs)
        self.assertTrue([(1, 2), (1, 4), (2, 3), (3, 6)] not in gs)

    def test_comparison(self):
        gs = GraphSet([g12])
        self.assertEqual(gs, GraphSet([g12]))
        self.assertNotEqual(gs, GraphSet([g13]))

        # __nonzero__
        self.assertTrue(gs)
        self.assertFalse(GraphSet())

        v = [g0, g12, g13]
        gs = GraphSet(v)
        self.assertTrue(gs.isdisjoint(GraphSet([g1, g123])))
        self.assertFalse(gs.isdisjoint(GraphSet([g1, g12])))

        self.assertTrue(gs.issubset(GraphSet(v)))
        self.assertFalse(gs.issubset(GraphSet([g0, g12])))
        self.assertTrue(gs <= GraphSet(v))
        self.assertFalse(gs <= GraphSet([g0, g12]))
        self.assertTrue(gs < GraphSet([g0, g1, g12, g13]))
        self.assertFalse(gs < GraphSet(v))

        self.assertTrue(gs.issuperset(GraphSet(v)))
        self.assertFalse(gs.issuperset(GraphSet([g1, g12])))
        self.assertTrue(gs >= GraphSet(v))
        self.assertFalse(gs >= GraphSet([g1, g12]))
        self.assertTrue(gs > GraphSet([[], g12]))
        self.assertFalse(gs > GraphSet(v))

    def test_unary_operators(self):
        gs = GraphSet([g0, g1, g12, g123, g1234, g134, g14, g4])

        self.assertTrue(isinstance(~gs, GraphSet))
        self.assertEqual(~gs, GraphSet([g124, g13, g2, g23, g234, g24, g3, g34]))

        self.assertTrue(isinstance(gs.smaller(3), GraphSet))
        self.assertEqual(gs.smaller(3), GraphSet([g0, g1, g12, g14, g4]))
        self.assertTrue(isinstance(gs.larger(3), GraphSet))
        self.assertEqual(gs.larger(3), GraphSet([g1234]))
        self.assertTrue(isinstance(gs.graph_size(3), GraphSet))
        self.assertEqual(gs.graph_size(3), GraphSet([g123, g134]))
        self.assertTrue(isinstance(gs.len(3), GraphSet))
        self.assertEqual(gs.len(3), GraphSet([g123, g134]))

        gs = GraphSet([g12, g123, g234])
        self.assertTrue(isinstance(gs.minimal(), GraphSet))
        self.assertEqual(gs.minimal(), GraphSet([g12, g234]))
        self.assertTrue(isinstance(gs.maximal(), GraphSet))
        self.assertEqual(gs.maximal(), GraphSet([g123, g234]))

        gs = GraphSet([g12, g14, g23, g34])
        self.assertTrue(isinstance(gs.blocking(), GraphSet))
        self.assertEqual(
            gs.blocking(), GraphSet([g123, g1234, g124, g13, g134, g234, g24]))

    def test_binary_operators(self):
        u = [g0, g1, g12, g123, g1234, g134, g14, g4]
        v = [g12, g14, g23, g34]

        gs = GraphSet(u) | GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(
            gs, GraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))
        gs = GraphSet(u).union(GraphSet(u), GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(
            gs, GraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))

        gs = GraphSet(u)
        gs |= GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(
            gs, GraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))
        gs = GraphSet(u)
        gs.update(GraphSet(u), GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(
            gs, GraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))

        gs = GraphSet(u) & GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g12, g14]))
        gs = GraphSet(u).intersection(GraphSet(u), GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g12, g14]))

        gs = GraphSet(u)
        gs &= GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g12, g14]))
        gs = GraphSet(u)
        gs.intersection_update(GraphSet(u), GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g12, g14]))

        gs = GraphSet(u) - GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g4]))
        gs = GraphSet(u).difference(GraphSet(), GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g4]))

        gs = GraphSet(u)
        gs -= GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g4]))
        gs = GraphSet(u)
        gs.difference_update(GraphSet(), GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g4]))

        gs = GraphSet(u) ^ GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g23, g34, g4]))
        gs = GraphSet(u).symmetric_difference(GraphSet(), GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g23, g34, g4]))

        gs = GraphSet(u)
        gs ^= GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g23, g34, g4]))
        gs = GraphSet(u)
        gs.symmetric_difference_update(GraphSet(), GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g23, g34, g4]))

        v = [g12]
        gs = GraphSet(u) / GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g3, g34]))
        gs = GraphSet(u).quotient(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g3, g34]))

        gs = GraphSet(u)
        gs /= GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g3, g34]))
        gs = GraphSet(u)
        gs.quotient_update(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g3, g34]))

        gs = GraphSet(u) % GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g134, g14, g4]))
        gs = GraphSet(u).remainder(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g134, g14, g4]))

        gs = GraphSet(u)
        gs %= GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g134, g14, g4]))
        gs = GraphSet(u)
        gs.remainder_update(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g134, g14, g4]))

        gs = GraphSet(u).complement()
        self.assertEqual(gs, GraphSet([g0, g123, g1234, g2, g23, g234, g34, g4]))

        v = [g12, g14, g23, g34]
        gs = GraphSet(u).join(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(
            gs, GraphSet([g12, g123, g124, g1234, g134, g14, g23, g234, g34]))

        gs = GraphSet(u).meet(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g12, g14, g2, g23, g3, g34, g4]))

        v = [g12, g14, g23, g34]
        gs = GraphSet(u).subgraphs(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g12, g14, g4]))

        gs = GraphSet(u).supergraphs(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g12, g123, g1234, g134, g14]))

        gs = GraphSet(u).non_subgraphs(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g123, g1234, g134]))

        gs = GraphSet(u).non_supergraphs(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g4]))

        gs1 = GraphSet({}) - GraphSet([g1, g34])

        gs2 = gs1.including(GraphSet([g1, g2]))
        self.assertTrue(isinstance(gs2, GraphSet))
        self.assertEqual(len(gs2), 11)

        gs2 = gs1.including(g1)
        self.assertTrue(isinstance(gs2, GraphSet))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.including((2,1))
        self.assertTrue(isinstance(gs2, GraphSet))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.including(1)
        self.assertTrue(isinstance(gs2, GraphSet))
        self.assertEqual(len(gs2), 11)

        self.assertRaises(KeyError, gs1.including, (1, 4))
        self.assertRaises(KeyError, gs1.including, 5)

        gs2 = gs1.excluding(GraphSet([g1, g2]))
        self.assertTrue(isinstance(gs2, GraphSet))
        self.assertEqual(len(gs2), 3)

        gs2 = gs1.excluding(g1)
        self.assertTrue(isinstance(gs2, GraphSet))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.excluding(e2)
        self.assertTrue(isinstance(gs2, GraphSet))
        self.assertEqual(len(gs2), 6)

        gs2 = gs1.excluding(1)
        self.assertTrue(isinstance(gs2, GraphSet))
        self.assertEqual(len(gs2), 3)

        self.assertRaises(KeyError, gs1.excluding, (1, 4))
        self.assertRaises(KeyError, gs1.excluding, 5)

        v = [g12, g14, g23, g34]
        gs = GraphSet(u).included(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g12, g14, g4]))

        gs = GraphSet(u).included(g12)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g12]))

    def capacity(self):
        gs = GraphSet()
        self.assertFalse(gs)

        gs = GraphSet([g0, g12, g13])
        self.assertTrue(gs)

        self.assertEqual(len(gs), 3)
        self.assertEqual(gs.len(), 3)

    def test_iterators(self):
        gs1 = GraphSet([g0, g12, g13])
        gs2 = GraphSet()
        for g in gs1:
            self.assertTrue(isinstance(g, list))
            gs2 = gs2 | GraphSet([g])
        self.assertEqual(gs1, GraphSet([g0, g12, g13]))
        self.assertEqual(gs1, gs2)

        gs2 = GraphSet()
        for g in gs1:
            self.assertTrue(isinstance(g, list))
            gs2 = gs2 | GraphSet([g])
        self.assertEqual(gs1, gs2)

        gs1 = GraphSet([g0, g12, g13])
        gs2 = GraphSet()
        for g in gs1.rand_iter():
            self.assertTrue(isinstance(g, list))
            gs2 = gs2 | GraphSet([g])
        self.assertEqual(gs1, gs2)

        gen = gs1.rand_iter()
        self.assertTrue(isinstance(next(gen), list))

        gs = GraphSet([g0, g1, g12, g123, g1234, g134, g14, g4])
        r = []
        for g in gs.max_iter():
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], g14)
        self.assertEqual(r[1], g134)
        self.assertEqual(r[2], g4)

        r = []
        for g in gs.max_iter({e1: -.3, e2: .2, e3: .2, e4: -.4}):
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], g123)
        self.assertEqual(r[1], g0)
        self.assertEqual(r[2], g12)

        r = []
        for g in gs.min_iter():
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], g123)
        self.assertEqual(r[1], g0)
        self.assertEqual(r[2], g12)

        r = []
        for g in gs.min_iter({e1: -.3, e2: .2, e3: .2, e4: -.4}):
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], g14)
        self.assertEqual(r[1], g134)
        self.assertEqual(r[2], g4)

        gs = GraphSet([[]])
        self.assertEqual(list(gs.min_iter()), [[]])

    def test_lookup(self):
        gs1 = GraphSet([g1, g12])

        self.assertTrue(g12 in gs1)
        self.assertTrue(g2 not in gs1)
        self.assertTrue(e1 in gs1)
        self.assertTrue(e4 not in gs1)
        self.assertTrue(1 in gs1)
        self.assertTrue(4 not in gs1)

    def test_modifiers(self):
        v = [g0, g12, g13]
        gs = GraphSet(v)
        gs.add(g1)
        self.assertTrue(g1 in gs)

        gs.remove(g1)
        self.assertTrue(g1 not in gs)
        self.assertRaises(KeyError, gs.remove, g1)

        gs.add(g0)
        gs.discard(g0)
        self.assertTrue(g0 not in gs)
        gs.discard(g0)  # no exception raised

        gs = GraphSet(v)
        gs.add(e2)
        self.assertEqual(gs, GraphSet([g12, g123, g2]))

        gs = GraphSet(v)
        gs.remove(e2)
        self.assertEqual(gs, GraphSet([g0, g1, g13]))
        self.assertRaises(KeyError, gs.remove, e4)

        gs = GraphSet(v)
        gs.discard(e2)
        self.assertEqual(gs, GraphSet([g0, g1, g13]))
        gs.discard(e4)  # no exception raised

        v = [g1, g12, g13]
        gs = GraphSet(v)
        g = gs.pop()
        self.assertTrue(isinstance(g, list))
        self.assertTrue(g not in gs)
        self.assertEqual(gs | GraphSet([g]), GraphSet(v))

        self.assertTrue(gs)
        gs.clear()
        self.assertFalse(gs)

        self.assertRaises(KeyError, gs.pop)

        self.assertRaises(KeyError, gs.add, [(1,4)])
        self.assertRaises(KeyError, gs.remove, [(1,4)])
        self.assertRaises(KeyError, gs.discard, [(1,4)])

        self.assertRaises(KeyError, gs.add, (1,4))
        self.assertRaises(KeyError, gs.remove, (1,4))
        self.assertRaises(KeyError, gs.discard, (1,4))

        u = [g0, g1, g12, g123, g1234, g134, g14, g4]
        gs = GraphSet(u)
        gs.flip(e1)
        self.assertEqual(gs, GraphSet([g0, g1, g14, g2, g23, g234, g34, g4]))

    def test_probability(self):
        p = {e1: .9, e2: .8, e3: .7, e4: .6}

        gs = GraphSet()
        self.assertEqual(gs.probability(p), 0)

        gs = GraphSet([g0])
        self.assertAlmostEqual(gs.probability(p), .0024)

        gs = GraphSet([g1])
        self.assertAlmostEqual(gs.probability(p), .0216)

        gs = GraphSet([g2])
        self.assertAlmostEqual(gs.probability(p), .0096)

        gs = GraphSet([g12, g13])
        self.assertAlmostEqual(gs.probability(p), .1368)

        gs = GraphSet([g1234])
        self.assertAlmostEqual(gs.probability(p), .3024)

        gs = GraphSet([g0, g1, g2, g12, g13, g1234])
        self.assertAlmostEqual(gs.probability(p), .4728)

    def test_cost_le(self):
        GraphSet.set_universe([e1, e2, e3, e4])
        gs = GraphSet([g0, g1, g2, g3, g4, g12, g14, g134, g234, g1234])

        costs = {e1: 2, e2: 14, e3: 4, e4: 7}
        cost_bound = 13

        small_cost_gs = gs.cost_le(costs, cost_bound)
        self.assertIn(g0, small_cost_gs) # cost: 0
        self.assertIn(g1, small_cost_gs) # cost: 2
        self.assertNotIn(g2, small_cost_gs) # cost: 14
        self.assertIn(g3, small_cost_gs) # cost: 4
        self.assertIn(g4, small_cost_gs) # cost: 7
        self.assertNotIn(g12, small_cost_gs) # cost: 16
        self.assertIn(g14, small_cost_gs) # cost: 9
        self.assertIn(g134, small_cost_gs) # cost: 13
        self.assertNotIn(g234, small_cost_gs) # cost: 25
        self.assertNotIn(g1234, small_cost_gs) # cost: 27

    def test_cost_ge(self):
        GraphSet.set_universe([e1, e2, e3, e4])
        gs = GraphSet([g0, g1, g2, g3, g4, g12, g14, g134, g234, g1234])

        costs = {e1: 2, e2: 14, e3: 4, e4: 7}
        cost_bound = 13

        large_cost_gs = gs.cost_ge(costs, cost_bound)
        self.assertNotIn(g0, large_cost_gs) # cost: 0
        self.assertNotIn(g1, large_cost_gs) # cost: 2
        self.assertIn(g2, large_cost_gs) # cost: 14
        self.assertNotIn(g3, large_cost_gs) # cost: 4
        self.assertNotIn(g4, large_cost_gs) # cost: 7
        self.assertIn(g12, large_cost_gs) # cost: 16
        self.assertNotIn(g14, large_cost_gs) # cost: 9
        self.assertIn(g134, large_cost_gs) # cost: 13
        self.assertIn(g234, large_cost_gs) # cost: 25
        self.assertIn(g1234, large_cost_gs) # cost: 27

    def test_cost_eq(self):
        GraphSet.set_universe([e1, e2, e3, e4])
        gs = GraphSet([g0, g1, g2, g3, g4, g12, g14, g134, g234, g1234])

        costs = {e1: 2, e2: 14, e3: 4, e4: 7}
        cost_bound = 13

        equal_cost_gs = gs.cost_eq(costs, cost_bound)
        self.assertNotIn(g0, equal_cost_gs) # cost: 0
        self.assertNotIn(g1, equal_cost_gs) # cost: 2
        self.assertNotIn(g2, equal_cost_gs) # cost: 14
        self.assertNotIn(g3, equal_cost_gs) # cost: 4
        self.assertNotIn(g4, equal_cost_gs) # cost: 7
        self.assertNotIn(g12, equal_cost_gs) # cost: 16
        self.assertNotIn(g14, equal_cost_gs) # cost: 9
        self.assertIn(g134, equal_cost_gs) # cost: 13
        self.assertNotIn(g234, equal_cost_gs) # cost: 25
        self.assertNotIn(g1234, equal_cost_gs) # cost: 27

    def test_remove_some_edge(self):

        gs = GraphSet([])
        self.assertEqual(gs.remove_some_edge(), GraphSet())

        gs = GraphSet([g0])
        self.assertEqual(gs.remove_some_edge(), GraphSet())

        gs = GraphSet([g1])
        self.assertEqual(gs.remove_some_edge(), GraphSet([g0]))

        gs = GraphSet([g0, g1])
        self.assertEqual(gs.remove_some_edge(), GraphSet([g0]))

        gs = GraphSet([g1, g12])
        self.assertEqual(gs.remove_some_edge(), GraphSet([g0, g1, g2]))

        gs1 = GraphSet([g0, g4, g12, g234])
        gs2 = GraphSet([g0, g1, g2, g23, g24, g34])
        self.assertEqual(gs1.remove_some_edge(), gs2)

    def test_add_some_edge(self):

        gs = GraphSet([])
        self.assertEqual(gs.add_some_edge(), GraphSet())

        gs = GraphSet([g0])
        self.assertEqual(gs.add_some_edge(), GraphSet([g1, g2, g3, g4]))

        gs = GraphSet([g1])
        self.assertEqual(gs.add_some_edge(), GraphSet([g12, g13, g14]))

        gs = GraphSet([g0, g1])
        self.assertEqual(gs.add_some_edge(),
                         GraphSet([g1, g2, g3, g4, g12, g13, g14]))

        gs = GraphSet([g1, g12])
        self.assertEqual(gs.add_some_edge(),
                         GraphSet([g12, g13, g14, g123, g124]))

        gs1 = GraphSet([g0, g4, g12, g234])
        gs2 = GraphSet([g1, g2, g3, g4, g14, g24, g34, g123, g124, g1234])
        self.assertEqual(gs1.add_some_edge(), gs2)

    def test_remove_add_some_edges(self):

        gs = GraphSet([])
        self.assertEqual(gs.remove_add_some_edges(), GraphSet())

        gs = GraphSet([g0])
        self.assertEqual(gs.remove_add_some_edges(), GraphSet())

        gs = GraphSet([g1])
        self.assertEqual(gs.remove_add_some_edges(),
                         GraphSet([g2, g3, g4]))

        gs = GraphSet([g0, g1])
        self.assertEqual(gs.remove_add_some_edges(),
                         GraphSet([g2, g3, g4]))

        gs = GraphSet([g1, g12])
        self.assertEqual(gs.remove_add_some_edges(),
                         GraphSet([g2, g3, g4, g13, g14, g23, g24]))

        gs1 = GraphSet([g0, g4, g12, g234])
        gs2 = GraphSet([g1, g2, g3, g13, g14, g23, g24, g123, g124, g134])
        self.assertEqual(gs1.remove_add_some_edges(), gs2)

    def test_io(self):
        gs = GraphSet()
        st = gs.dumps()
        self.assertEqual(st, "B\n.\n")
        gs = GraphSet.loads(st)
        self.assertEqual(gs, GraphSet())

        gs = GraphSet([g0])
        st = gs.dumps()
        self.assertEqual(st, "T\n.\n")
        gs = GraphSet.loads(st)
        self.assertEqual(gs, GraphSet([g0]))

        v = [g0, g1, g12, g123, g1234, g134, g14, g4]
        gs = GraphSet(v)
        st = gs.dumps()
        gs = GraphSet.loads(st)
        self.assertEqual(gs, GraphSet(v))

        # skip this test, becasue string is treated as an element
#        gs = GraphSet(st)
#        self.assertEqual(gs, GraphSet(v))

        with tempfile.TemporaryFile() as f:
            gs.dump(f)
            f.seek(0)
            gs = GraphSet.load(f)
            self.assertEqual(gs, GraphSet(v))

        for gs1 in [GraphSet([]), GraphSet([g0]), GraphSet([g1])]:
            with tempfile.TemporaryFile() as f:
                gs1.dump(f)
                f.seek(0)
                gs2 = GraphSet.load(f)
                self.assertEqual(gs1, gs2)

    def test_networkx(self):
        try:
            import networkx as nx
        except ImportError:
            return

        try:
            if nx.__version__[0] == "1": # for NetworkX version 1.x
                GraphSet.converters['to_graph'] = nx.Graph
                GraphSet.converters['to_edges'] = nx.Graph.edges
            else: # for NetworkX version 2.x
                GraphSet.converters['to_graph'] = nx.from_edgelist
                GraphSet.converters['to_edges'] = nx.to_edgelist

            g = nx.grid_2d_graph(3, 3)
            GraphSet.set_universe(g)
            g = GraphSet.universe()
            self.assertTrue(isinstance(g, nx.Graph))
            self.assertEqual(len(g.edges()), 12)

            v00, v01, v10 = (0,0), (0,1), (1,0)
            e1, e2 = (v00, v01), (v00, v10)
            gs = GraphSet([nx.Graph([e1])])
            self.assertEqual(len(gs), 1)
            g = gs.pop()
            self.assertEqual(len(gs), 0)
            self.assertTrue(isinstance(g, nx.Graph))
            self.assertTrue(list(g.edges()) == [(v00, v01)] or list(g.edges()) == [(v01, v00)])
            gs.add(nx.Graph([e2]))
            self.assertEqual(len(gs), 1)
        except:
            raise
        finally:
            GraphSet.converters['to_graph'] = lambda edges: edges
            GraphSet.converters['to_edges'] = lambda graph: graph

    def test_large(self):
        try:
            import networkx as nx
        except ImportError:
            return

        try:
            if nx.__version__[0] == "1": # for NetworkX version 1.x
                GraphSet.converters['to_graph'] = nx.Graph
                GraphSet.converters['to_edges'] = nx.Graph.edges
            else: # for NetworkX version 2.x
                GraphSet.converters['to_graph'] = nx.from_edgelist
                GraphSet.converters['to_edges'] = nx.to_edgelist

            g = nx.grid_2d_graph(8, 8)
            v00, v01, v10 = (0,0), (0,1), (1,0)

            GraphSet.set_universe(g, traversal='bfs')
            self.assertEqual(len(GraphSet.universe().edges()), 112)
#            self.assertEqual(GraphSet.universe().edges()[:2], [(v00, v01), (v00, v10)])

            gs = GraphSet({});
            gs -= GraphSet([nx.Graph([(v00, v01)]),
                            nx.Graph([(v00, v01), (v00, v10)])])
            self.assertEqual(gs.len(), 5192296858534827628530496329220094)

            i = 0
            for g in gs:
                if i > 100: break
                i += 1

            paths = GraphSet.paths((0, 0), (7, 7))
            self.assertEqual(len(paths), 789360053252)
        except:
            raise
        finally:
            GraphSet.converters['to_graph'] = lambda edges: edges
            GraphSet.converters['to_edges'] = lambda graph: graph


if __name__ == '__main__':
    unittest.main()
