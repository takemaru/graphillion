from graphillion import setset, VertexSetSet
import tempfile
import unittest


e1 = (1,2)
e2 = (1,3)
e3 = (2,4)
e4 = (3,4)
e5 = (5,6)

v1 = 1
v2 = 2
v3 = 3
v4 = 4

vs0 = []
vs1 = [v1]
vs2 = [v2]
vs3 = [v3]
vs4 = [v4]
vs12 = [v1, v2]
vs13 = [v1, v3]
vs14 = [v1, v4]
vs23 = [v2, v3]
vs24 = [v2, v4]
vs34 = [v3, v4]
vs123 = [v1, v2, v3]
vs124 = [v1, v2, v4]
vs134 = [v1, v3, v4]
vs234 = [v2, v3, v4]
vs1234 = [v1, v2, v3, v4]


class TestGraphSet(unittest.TestCase):

    def setUp(self):
        setset.set_universe([e1 + (.1,), e2 + (-.2,), e3 + (-.3,), e4 + (.4,)])
        VertexSetSet.set_universe([(1, .3), (2, -.2), (3, -.2), (4, .4)])

    def tearDown(self):
        pass

    def test_init(self):
        VertexSetSet.set_universe(["i", "ii"])
        self.assertEqual(VertexSetSet.universe(), ["i", "ii"])

        VertexSetSet.set_universe([1, 2, (3)])
        self.assertEqual(VertexSetSet.universe(), [1, 2, 3])

        VertexSetSet.set_universe([1, (3, 100), 2])
        self.assertEqual(VertexSetSet.universe(), [1, (3, 100), 2])

        self.assertRaises(AssertionError,
                          VertexSetSet.set_universe,
                          ["i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix"])
        self.assertRaises(ValueError, VertexSetSet.set_universe, ["i", "i"])

    def test_constructors(self):
        vss = VertexSetSet()
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(len(vss), 0)

        vss = VertexSetSet([])
        self.assertEqual(len(vss), 0)

        vss = VertexSetSet([vs1, [2]])
        self.assertEqual(len(vss), 2)
        self.assertTrue(vs1 in vss)
        self.assertTrue(vs2 in vss)

        vss = VertexSetSet({})
        self.assertEqual(len(vss), 2**4)

        vss = VertexSetSet({'include': [v1, v2], 'exclude': [v3]})
        self.assertEqual(len(vss), 2)
        self.assertTrue(vs12 in vss)
        self.assertTrue(vs124 in vss)

        self.assertRaises(TypeError, VertexSetSet, [5])
        self.assertRaises(KeyError, VertexSetSet, [[(5)]])
        self.assertRaises(KeyError, VertexSetSet, {'include': [5]})

        # copy constructor
        vss1 = VertexSetSet([vs0, vs12, vs13])
        vss2 = vss1.copy()
        self.assertTrue(isinstance(vss2, VertexSetSet))
        vss1.clear()
        self.assertEqual(vss1, VertexSetSet())
        self.assertEqual(vss2, VertexSetSet([vs0, vs12, vs13]))

        # # repr
        vss = VertexSetSet([vs0, vs12, vs13])
        self.assertEqual(
            repr(vss),
            "VertexSetSet([[], ['1', '2'], ['1', '3']])")

        vss = VertexSetSet({})
        self.assertEqual(
            repr(vss),
            "VertexSetSet([[], ['1'], ['2'], ['3'], ['4'], ['1', '2'], ['1', '3'], ['2',  ...")

    def test_show_messages(self):
        a = VertexSetSet.show_messages()
        b = VertexSetSet.show_messages(True)
        self.assertTrue(b)
        c = VertexSetSet.show_messages(False)
        self.assertTrue(c)
        d = VertexSetSet.show_messages(a)
        self.assertFalse(d)

#     def test_comparison(self):
#         gs = GraphSet([g12])
#         self.assertEqual(gs, GraphSet([g12]))
#         self.assertNotEqual(gs, GraphSet([g13]))

#         # __nonzero__
#         self.assertTrue(gs)
#         self.assertFalse(GraphSet())

#         v = [g0, g12, g13]
#         gs = GraphSet(v)
#         self.assertTrue(gs.isdisjoint(GraphSet([g1, g123])))
#         self.assertFalse(gs.isdisjoint(GraphSet([g1, g12])))

#         self.assertTrue(gs.issubset(GraphSet(v)))
#         self.assertFalse(gs.issubset(GraphSet([g0, g12])))
#         self.assertTrue(gs <= GraphSet(v))
#         self.assertFalse(gs <= GraphSet([g0, g12]))
#         self.assertTrue(gs < GraphSet([g0, g1, g12, g13]))
#         self.assertFalse(gs < GraphSet(v))

#         self.assertTrue(gs.issuperset(GraphSet(v)))
#         self.assertFalse(gs.issuperset(GraphSet([g1, g12])))
#         self.assertTrue(gs >= GraphSet(v))
#         self.assertFalse(gs >= GraphSet([g1, g12]))
#         self.assertTrue(gs > GraphSet([[], g12]))
#         self.assertFalse(gs > GraphSet(v))

#     def test_unary_operators(self):
#         gs = GraphSet([g0, g1, g12, g123, g1234, g134, g14, g4])

#         self.assertTrue(isinstance(~gs, GraphSet))
#         self.assertEqual(~gs, GraphSet([g124, g13, g2, g23, g234, g24, g3, g34]))

#         self.assertTrue(isinstance(gs.smaller(3), GraphSet))
#         self.assertEqual(gs.smaller(3), GraphSet([g0, g1, g12, g14, g4]))
#         self.assertTrue(isinstance(gs.larger(3), GraphSet))
#         self.assertEqual(gs.larger(3), GraphSet([g1234]))
#         self.assertTrue(isinstance(gs.graph_size(3), GraphSet))
#         self.assertEqual(gs.graph_size(3), GraphSet([g123, g134]))
#         self.assertTrue(isinstance(gs.len(3), GraphSet))
#         self.assertEqual(gs.len(3), GraphSet([g123, g134]))

#         gs = GraphSet([g12, g123, g234])
#         self.assertTrue(isinstance(gs.minimal(), GraphSet))
#         self.assertEqual(gs.minimal(), GraphSet([g12, g234]))
#         self.assertTrue(isinstance(gs.maximal(), GraphSet))
#         self.assertEqual(gs.maximal(), GraphSet([g123, g234]))

#         gs = GraphSet([g12, g14, g23, g34])
#         self.assertTrue(isinstance(gs.blocking(), GraphSet))
#         self.assertEqual(
#             gs.blocking(), GraphSet([g123, g1234, g124, g13, g134, g234, g24]))

#     def test_binary_operators(self):
#         u = [g0, g1, g12, g123, g1234, g134, g14, g4]
#         v = [g12, g14, g23, g34]

#         gs = GraphSet(u) | GraphSet(v)
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(
#             gs, GraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))
#         gs = GraphSet(u).union(GraphSet(u), GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(
#             gs, GraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))

#         gs = GraphSet(u)
#         gs |= GraphSet(v)
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(
#             gs, GraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))
#         gs = GraphSet(u)
#         gs.update(GraphSet(u), GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(
#             gs, GraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))

#         gs = GraphSet(u) & GraphSet(v)
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g12, g14]))
#         gs = GraphSet(u).intersection(GraphSet(u), GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g12, g14]))

#         gs = GraphSet(u)
#         gs &= GraphSet(v)
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g12, g14]))
#         gs = GraphSet(u)
#         gs.intersection_update(GraphSet(u), GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g12, g14]))

#         gs = GraphSet(u) - GraphSet(v)
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g4]))
#         gs = GraphSet(u).difference(GraphSet(), GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g4]))

#         gs = GraphSet(u)
#         gs -= GraphSet(v)
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g4]))
#         gs = GraphSet(u)
#         gs.difference_update(GraphSet(), GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g4]))

#         gs = GraphSet(u) ^ GraphSet(v)
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g23, g34, g4]))
#         gs = GraphSet(u).symmetric_difference(GraphSet(), GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g23, g34, g4]))

#         gs = GraphSet(u)
#         gs ^= GraphSet(v)
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g23, g34, g4]))
#         gs = GraphSet(u)
#         gs.symmetric_difference_update(GraphSet(), GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g23, g34, g4]))

#         v = [g12]
#         gs = GraphSet(u) / GraphSet(v)
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g3, g34]))
#         gs = GraphSet(u).quotient(GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g3, g34]))

#         gs = GraphSet(u)
#         gs /= GraphSet(v)
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g3, g34]))
#         gs = GraphSet(u)
#         gs.quotient_update(GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g3, g34]))

#         gs = GraphSet(u) % GraphSet(v)
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g134, g14, g4]))
#         gs = GraphSet(u).remainder(GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g134, g14, g4]))

#         gs = GraphSet(u)
#         gs %= GraphSet(v)
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g134, g14, g4]))
#         gs = GraphSet(u)
#         gs.remainder_update(GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g134, g14, g4]))

#         gs = GraphSet(u).complement()
#         self.assertEqual(gs, GraphSet([g0, g123, g1234, g2, g23, g234, g34, g4]))

#         v = [g12, g14, g23, g34]
#         gs = GraphSet(u).join(GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(
#             gs, GraphSet([g12, g123, g124, g1234, g134, g14, g23, g234, g34]))

#         gs = GraphSet(u).meet(GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g12, g14, g2, g23, g3, g34, g4]))

#         v = [g12, g14, g23, g34]
#         gs = GraphSet(u).subgraphs(GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g12, g14, g4]))

#         gs = GraphSet(u).supergraphs(GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g12, g123, g1234, g134, g14]))

#         gs = GraphSet(u).non_subgraphs(GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g123, g1234, g134]))

#         gs = GraphSet(u).non_supergraphs(GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g4]))

#         gs1 = GraphSet({}) - GraphSet([g1, g34])

#         gs2 = gs1.including(GraphSet([g1, g2]))
#         self.assertTrue(isinstance(gs2, GraphSet))
#         self.assertEqual(len(gs2), 11)

#         gs2 = gs1.including(g1)
#         self.assertTrue(isinstance(gs2, GraphSet))
#         self.assertEqual(len(gs2), 7)

#         gs2 = gs1.including((2,1))
#         self.assertTrue(isinstance(gs2, GraphSet))
#         self.assertEqual(len(gs2), 7)

#         gs2 = gs1.including(1)
#         self.assertTrue(isinstance(gs2, GraphSet))
#         self.assertEqual(len(gs2), 11)

#         self.assertRaises(KeyError, gs1.including, (1, 4))
#         self.assertRaises(KeyError, gs1.including, 5)

#         gs2 = gs1.excluding(GraphSet([g1, g2]))
#         self.assertTrue(isinstance(gs2, GraphSet))
#         self.assertEqual(len(gs2), 3)

#         gs2 = gs1.excluding(g1)
#         self.assertTrue(isinstance(gs2, GraphSet))
#         self.assertEqual(len(gs2), 7)

#         gs2 = gs1.excluding(e2)
#         self.assertTrue(isinstance(gs2, GraphSet))
#         self.assertEqual(len(gs2), 6)

#         gs2 = gs1.excluding(1)
#         self.assertTrue(isinstance(gs2, GraphSet))
#         self.assertEqual(len(gs2), 3)

#         self.assertRaises(KeyError, gs1.excluding, (1, 4))
#         self.assertRaises(KeyError, gs1.excluding, 5)

#         v = [g12, g14, g23, g34]
#         gs = GraphSet(u).included(GraphSet(v))
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g12, g14, g4]))

#         gs = GraphSet(u).included(g12)
#         self.assertTrue(isinstance(gs, GraphSet))
#         self.assertEqual(gs, GraphSet([g0, g1, g12]))

#     def capacity(self):
#         gs = GraphSet()
#         self.assertFalse(gs)

#         gs = GraphSet([g0, g12, g13])
#         self.assertTrue(gs)

#         self.assertEqual(len(gs), 3)
#         self.assertEqual(gs.len(), 3)

#     def test_iterators(self):
#         gs1 = GraphSet([g0, g12, g13])
#         gs2 = GraphSet()
#         for g in gs1:
#             self.assertTrue(isinstance(g, list))
#             gs2 = gs2 | GraphSet([g])
#         self.assertEqual(gs1, GraphSet([g0, g12, g13]))
#         self.assertEqual(gs1, gs2)

#         gs2 = GraphSet()
#         for g in gs1:
#             self.assertTrue(isinstance(g, list))
#             gs2 = gs2 | GraphSet([g])
#         self.assertEqual(gs1, gs2)

#         gs1 = GraphSet([g0, g12, g13])
#         gs2 = GraphSet()
#         for g in gs1.rand_iter():
#             self.assertTrue(isinstance(g, list))
#             gs2 = gs2 | GraphSet([g])
#         self.assertEqual(gs1, gs2)

#         gen = gs1.rand_iter()
#         self.assertTrue(isinstance(next(gen), list))

#         gs = GraphSet([g0, g1, g12, g123, g1234, g134, g14, g4])
#         r = []
#         for g in gs.max_iter():
#             self.assertTrue(isinstance(g, list))
#             r.append(g)
#         self.assertEqual(len(r), 8)
#         self.assertEqual(r[0], g14)
#         self.assertEqual(r[1], g134)
#         self.assertEqual(r[2], g4)

#         r = []
#         for g in gs.max_iter({e1: -.3, e2: .2, e3: .2, e4: -.4}):
#             self.assertTrue(isinstance(g, list))
#             r.append(g)
#         self.assertEqual(len(r), 8)
#         self.assertEqual(r[0], g123)
#         self.assertEqual(r[1], g0)
#         self.assertEqual(r[2], g12)

#         r = []
#         for g in gs.min_iter():
#             self.assertTrue(isinstance(g, list))
#             r.append(g)
#         self.assertEqual(len(r), 8)
#         self.assertEqual(r[0], g123)
#         self.assertEqual(r[1], g0)
#         self.assertEqual(r[2], g12)

#         r = []
#         for g in gs.min_iter({e1: -.3, e2: .2, e3: .2, e4: -.4}):
#             self.assertTrue(isinstance(g, list))
#             r.append(g)
#         self.assertEqual(len(r), 8)
#         self.assertEqual(r[0], g14)
#         self.assertEqual(r[1], g134)
#         self.assertEqual(r[2], g4)

#         gs = GraphSet([[]])
#         self.assertEqual(list(gs.min_iter()), [[]])

#     def test_lookup(self):
#         gs1 = GraphSet([g1, g12])

#         self.assertTrue(g12 in gs1)
#         self.assertTrue(g2 not in gs1)
#         self.assertTrue(e1 in gs1)
#         self.assertTrue(e4 not in gs1)
#         self.assertTrue(1 in gs1)
#         self.assertTrue(4 not in gs1)

#     def test_modifiers(self):
#         v = [g0, g12, g13]
#         gs = GraphSet(v)
#         gs.add(g1)
#         self.assertTrue(g1 in gs)

#         gs.remove(g1)
#         self.assertTrue(g1 not in gs)
#         self.assertRaises(KeyError, gs.remove, g1)

#         gs.add(g0)
#         gs.discard(g0)
#         self.assertTrue(g0 not in gs)
#         gs.discard(g0)  # no exception raised

#         gs = GraphSet(v)
#         gs.add(e2)
#         self.assertEqual(gs, GraphSet([g12, g123, g2]))

#         gs = GraphSet(v)
#         gs.remove(e2)
#         self.assertEqual(gs, GraphSet([g0, g1, g13]))
#         self.assertRaises(KeyError, gs.remove, e4)

#         gs = GraphSet(v)
#         gs.discard(e2)
#         self.assertEqual(gs, GraphSet([g0, g1, g13]))
#         gs.discard(e4)  # no exception raised

#         v = [g1, g12, g13]
#         gs = GraphSet(v)
#         g = gs.pop()
#         self.assertTrue(isinstance(g, list))
#         self.assertTrue(g not in gs)
#         self.assertEqual(gs | GraphSet([g]), GraphSet(v))

#         self.assertTrue(gs)
#         gs.clear()
#         self.assertFalse(gs)

#         self.assertRaises(KeyError, gs.pop)

#         self.assertRaises(KeyError, gs.add, [(1,4)])
#         self.assertRaises(KeyError, gs.remove, [(1,4)])
#         self.assertRaises(KeyError, gs.discard, [(1,4)])

#         self.assertRaises(KeyError, gs.add, (1,4))
#         self.assertRaises(KeyError, gs.remove, (1,4))
#         self.assertRaises(KeyError, gs.discard, (1,4))

#         u = [g0, g1, g12, g123, g1234, g134, g14, g4]
#         gs = GraphSet(u)
#         gs.flip(e1)
#         self.assertEqual(gs, GraphSet([g0, g1, g14, g2, g23, g234, g34, g4]))

#     def test_probability(self):
#         p = {e1: .9, e2: .8, e3: .7, e4: .6}

#         gs = GraphSet()
#         self.assertEqual(gs.probability(p), 0)

#         gs = GraphSet([g0])
#         self.assertAlmostEqual(gs.probability(p), .0024)

#         gs = GraphSet([g1])
#         self.assertAlmostEqual(gs.probability(p), .0216)

#         gs = GraphSet([g2])
#         self.assertAlmostEqual(gs.probability(p), .0096)

#         gs = GraphSet([g12, g13])
#         self.assertAlmostEqual(gs.probability(p), .1368)

#         gs = GraphSet([g1234])
#         self.assertAlmostEqual(gs.probability(p), .3024)

#         gs = GraphSet([g0, g1, g2, g12, g13, g1234])
#         self.assertAlmostEqual(gs.probability(p), .4728)

#     def test_cost_le(self):
#         GraphSet.set_universe([e1, e2, e3, e4])
#         gs = GraphSet([g0, g1, g2, g3, g4, g12, g14, g134, g234, g1234])

#         costs = {e1: 2, e2: 14, e3: 4, e4: 7}
#         cost_bound = 13

#         small_cost_gs = gs.cost_le(costs, cost_bound)
#         self.assertIn(g0, small_cost_gs) # cost: 0
#         self.assertIn(g1, small_cost_gs) # cost: 2
#         self.assertNotIn(g2, small_cost_gs) # cost: 14
#         self.assertIn(g3, small_cost_gs) # cost: 4
#         self.assertIn(g4, small_cost_gs) # cost: 7
#         self.assertNotIn(g12, small_cost_gs) # cost: 16
#         self.assertIn(g14, small_cost_gs) # cost: 9
#         self.assertIn(g134, small_cost_gs) # cost: 13
#         self.assertNotIn(g234, small_cost_gs) # cost: 25
#         self.assertNotIn(g1234, small_cost_gs) # cost: 27

#     def test_cost_ge(self):
#         GraphSet.set_universe([e1, e2, e3, e4])
#         gs = GraphSet([g0, g1, g2, g3, g4, g12, g14, g134, g234, g1234])

#         costs = {e1: 2, e2: 14, e3: 4, e4: 7}
#         cost_bound = 13

#         small_cost_gs = gs.cost_ge(costs, cost_bound)
#         self.assertNotIn(g0, small_cost_gs) # cost: 0
#         self.assertNotIn(g1, small_cost_gs) # cost: 2
#         self.assertIn(g2, small_cost_gs) # cost: 14
#         self.assertNotIn(g3, small_cost_gs) # cost: 4
#         self.assertNotIn(g4, small_cost_gs) # cost: 7
#         self.assertIn(g12, small_cost_gs) # cost: 16
#         self.assertNotIn(g14, small_cost_gs) # cost: 9
#         self.assertIn(g134, small_cost_gs) # cost: 13
#         self.assertIn(g234, small_cost_gs) # cost: 25
#         self.assertIn(g1234, small_cost_gs) # cost: 27

#     def test_cost_eq(self):
#         GraphSet.set_universe([e1, e2, e3, e4])
#         gs = GraphSet([g0, g1, g2, g3, g4, g12, g14, g134, g234, g1234])

#         costs = {e1: 2, e2: 14, e3: 4, e4: 7}
#         cost = 13

#         small_cost_gs = gs.cost_eq(costs, cost)
#         self.assertNotIn(g0, small_cost_gs) # cost: 0
#         self.assertNotIn(g1, small_cost_gs) # cost: 2
#         self.assertNotIn(g2, small_cost_gs) # cost: 14
#         self.assertNotIn(g3, small_cost_gs) # cost: 4
#         self.assertNotIn(g4, small_cost_gs) # cost: 7
#         self.assertNotIn(g12, small_cost_gs) # cost: 16
#         self.assertNotIn(g14, small_cost_gs) # cost: 9
#         self.assertIn(g134, small_cost_gs) # cost: 13
#         self.assertNotIn(g234, small_cost_gs) # cost: 25
#         self.assertNotIn(g1234, small_cost_gs) # cost: 27

#     def test_io(self):
#         gs = GraphSet()
#         st = gs.dumps()
#         self.assertEqual(st, "B\n.\n")
#         gs = GraphSet.loads(st)
#         self.assertEqual(gs, GraphSet())

#         gs = GraphSet([g0])
#         st = gs.dumps()
#         self.assertEqual(st, "T\n.\n")
#         gs = GraphSet.loads(st)
#         self.assertEqual(gs, GraphSet([g0]))

#         v = [g0, g1, g12, g123, g1234, g134, g14, g4]
#         gs = GraphSet(v)
#         st = gs.dumps()
#         gs = GraphSet.loads(st)
#         self.assertEqual(gs, GraphSet(v))

#         # skip this test, becasue string is treated as an element
# #        gs = GraphSet(st)
# #        self.assertEqual(gs, GraphSet(v))

#         with tempfile.TemporaryFile() as f:
#             gs.dump(f)
#             f.seek(0)
#             gs = GraphSet.load(f)
#             self.assertEqual(gs, GraphSet(v))

#     def test_networkx(self):
#         try:
#             import networkx as nx
#         except ImportError:
#             return

#         try:
#             if nx.__version__[0] == "1": # for NetworkX version 1.x
#                 GraphSet.converters['to_graph'] = nx.Graph
#                 GraphSet.converters['to_edges'] = nx.Graph.edges
#             else: # for NetworkX version 2.x
#                 GraphSet.converters['to_graph'] = nx.from_edgelist
#                 GraphSet.converters['to_edges'] = nx.to_edgelist

#             g = nx.grid_2d_graph(3, 3)
#             GraphSet.set_universe(g)
#             g = GraphSet.universe()
#             self.assertTrue(isinstance(g, nx.Graph))
#             self.assertEqual(len(g.edges()), 12)

#             v00, v01, v10 = (0,0), (0,1), (1,0)
#             e1, e2 = (v00, v01), (v00, v10)
#             gs = GraphSet([nx.Graph([e1])])
#             self.assertEqual(len(gs), 1)
#             g = gs.pop()
#             self.assertEqual(len(gs), 0)
#             self.assertTrue(isinstance(g, nx.Graph))
#             self.assertTrue(list(g.edges()) == [(v00, v01)] or list(g.edges()) == [(v01, v00)])
#             gs.add(nx.Graph([e2]))
#             self.assertEqual(len(gs), 1)
#         except:
#             raise
#         finally:
#             GraphSet.converters['to_graph'] = lambda edges: edges
#             GraphSet.converters['to_edges'] = lambda graph: graph

#     def test_large(self):
#         try:
#             import networkx as nx
#         except ImportError:
#             return

#         try:
#             if nx.__version__[0] == "1": # for NetworkX version 1.x
#                 GraphSet.converters['to_graph'] = nx.Graph
#                 GraphSet.converters['to_edges'] = nx.Graph.edges
#             else: # for NetworkX version 2.x
#                 GraphSet.converters['to_graph'] = nx.from_edgelist
#                 GraphSet.converters['to_edges'] = nx.to_edgelist

#             g = nx.grid_2d_graph(8, 8)
#             v00, v01, v10 = (0,0), (0,1), (1,0)

#             GraphSet.set_universe(g, traversal='bfs')
#             self.assertEqual(len(GraphSet.universe().edges()), 112)
# #            self.assertEqual(GraphSet.universe().edges()[:2], [(v00, v01), (v00, v10)])

#             gs = GraphSet({});
#             gs -= GraphSet([nx.Graph([(v00, v01)]),
#                             nx.Graph([(v00, v01), (v00, v10)])])
#             self.assertEqual(gs.len(), 5192296858534827628530496329220094)

#             i = 0
#             for g in gs:
#                 if i > 100: break
#                 i += 1

#             paths = GraphSet.paths((0, 0), (7, 7))
#             self.assertEqual(len(paths), 789360053252)
#         except:
#             raise
#         finally:
#             GraphSet.converters['to_graph'] = lambda edges: edges
#             GraphSet.converters['to_edges'] = lambda graph: graph


if __name__ == '__main__':
    unittest.main()
