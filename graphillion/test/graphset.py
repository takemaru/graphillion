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
        GraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)])

    def tearDown(self):
        pass

    def test_init(self):
        GraphSet.set_universe([('i', 'ii')])
        self.assertEqual(GraphSet.universe(), [('i', 'ii')])

        GraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)])
        self.assertEqual(GraphSet.universe(),
                         [e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)])

        GraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                              traversal='dfs', source=1)
        self.assertEqual(GraphSet.universe(),
                         [e2 + (-.2,), e4 + (.4,), e1 + (.3,), e3 + (-.2,)])

        self.assertRaises(KeyError, GraphSet.set_universe, [(1,2), (2,1)])

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

        # matching
        dc = {}
        for v in range(1, 7):
            dc[v] = range(0, 2)
        gs = GraphSet.graphs(degree_constraints=dc)
        self.assertEqual(len(gs), 22)
        self.assertTrue([(1, 2), (3, 6)] in gs)
        self.assertTrue([(1, 2), (2, 3), (3, 6)] not in gs)
        for g in gs:
            self.assertTrue(len(g) < 4)

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

        # hamilton paths between 1 and 6
        gs = GraphSet.paths(1, 6, is_hamilton=True)
        self.assertEqual(len(gs), 1)
        self.assertTrue([(1, 4), (2, 3), (2, 5), (3, 6), (4, 5)] in gs)

        # called as instance methods
        gs = GraphSet.graphs(no_loop=True)
        _ = gs.connected_components([1, 3, 5])
        _ = gs.cliques(4)
        _ = gs.trees(1)
        _ = gs.forests([1, 3])
        _ = gs.cycles()
        _ = gs.paths(1, 6)

        # exceptions
        self.assertRaises(KeyError, GraphSet.graphs, vertex_groups=[[7]])
        self.assertRaises(KeyError, GraphSet.graphs, degree_constraints={7: 1})

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

#        v = [g12]
#        gs = GraphSet(u) / GraphSet(v)
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g0, g3, g34]))
#        gs = GraphSet(u).quotient(GraphSet(v))
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g0, g3, g34]))

#        gs = GraphSet(u)
#        gs /= GraphSet(v)
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g0, g3, g34]))
#        gs = GraphSet(u)
#        gs.quotient_update(GraphSet(v))
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g0, g3, g34]))

#        gs = GraphSet(u) % GraphSet(v)
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g0, g1, g134, g14, g4]))
#        gs = GraphSet(u).remainder(GraphSet(v))
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g0, g1, g134, g14, g4]))

#        gs = GraphSet(u)
#        gs %= GraphSet(v)
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g0, g1, g134, g14, g4]))
#        gs = GraphSet(u)
#        gs.remainder_update(GraphSet(v))
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g0, g1, g134, g14, g4]))

        gs = GraphSet(u).complement()
        self.assertEqual(gs, GraphSet([g0, g123, g1234, g2, g23, g234, g34, g4]))

#        v = [g12, g14, g23, g34]
#        gs = GraphSet(u).join(GraphSet(v))
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(
#            gs, GraphSet([g12, g123, g124, g1234, g134, g14, g23, g234, g34]))

#        gs = GraphSet(u).meet(GraphSet(v))
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g0, g1, g12, g14, g2, g23, g3, g34, g4]))

#        v = [g12, g14, g23, g34]
#        gs = GraphSet(u).subgraphs(GraphSet(v))
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g0, g1, g12, g14, g4]))

#        gs = GraphSet(u).supergraphs(GraphSet(v))
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g12, g123, g1234, g134, g14]))

#        gs = GraphSet(u).non_subgraphs(GraphSet(v))
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g123, g1234, g134]))

#        gs = GraphSet(u).non_supergraphs(GraphSet(v))
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g0, g1, g4]))

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
        self.assertTrue(isinstance(gen.next(), list))

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

        f = tempfile.TemporaryFile()
        gs.dump(f)
        f.seek(0)
        gs = GraphSet.load(f)
        self.assertEqual(gs, GraphSet(v))

    def test_networkx(self):
        try:
            import networkx as nx
        except ImportError:
            return

        try:
            GraphSet.converters['to_graph'] = nx.Graph
            GraphSet.converters['to_edges'] = nx.Graph.edges

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
            self.assertTrue(g.edges() == [(v00, v01)] or g.edges() == [(v01, v00)])
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
            GraphSet.converters['to_graph'] = nx.Graph
            GraphSet.converters['to_edges'] = nx.Graph.edges

            g = nx.grid_2d_graph(8, 8)
            v00, v01, v10 = (0,0), (0,1), (1,0)

            GraphSet.set_universe(g)
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
