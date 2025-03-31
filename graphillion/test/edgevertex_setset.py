#!/usr/bin/python3
# -*- coding: utf-8 -*-

from graphillion.universe import Universe
from graphillion import EdgeVertexSetSet
import unittest
import tempfile

e1 = (1,2)
e2 = (1,3)
e3 = (2,4)
e4 = (3,4)
e5 = (5,6)
e6 = (6,7)

ev0 = []
ev1 = [e1, 1, 2]
ev2 = [e2, 1, 3]
ev3 = [e3, 2, 4]
ev4 = [e4, 3, 4]
ev12 = [e1, e2, 1, 2, 3]
ev13 = [e1, e3, 1, 2, 4]
ev14 = [e1, e4, 1, 2, 3, 4]
ev23 = [e2, e3, 1, 2, 3, 4]
ev24 = [e2, e4, 1, 3, 4]
ev34 = [e3, e4, 2, 3, 4]
ev123 = [e1, e2, e3, 1, 2, 3, 4]
ev124 = [e1, e2, e4, 1, 2, 3, 4]
ev134 = [e1, e3, e4, 1, 2, 3, 4]
ev234 = [e2, e3, e4, 1, 2, 3, 4]
ev1234 = [e1, e2, e3, e4, 1, 2, 3, 4]

class TestEdgeVertexSetSet(unittest.TestCase):

    def setUp(self):
        #Universe.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
        #                      traversal='bfs', source=1,
        #                      weights={e1: -.3, e2: .2, e3: .2, e4: -.4, 1: -.2, 2: .4, 3: .6, 4: -.5})
        Universe.set_universe([e1 + (-.3,), e2 + (.2,), e3 + (.2,), e4 + (-.4,)],
                              traversal='bfs', source=1,
                              weights={e1: 2, e2: 5, e3: -4, e4: 8, 1: -.2, 2: .4, 3: .6, 4: -.5})
        # The edge weights in 'weights' argument should be ignored.

    def tearDown(self):
        pass

    def test_init(self):
        Universe.set_universe([('i', 'ii')])
        self.assertEqual(Universe.edge_vertex_universe(), [('i', 'ii'), 'i', 'ii'])

        self.assertRaises(KeyError, Universe.set_universe, [(1,2), (2,1)])

        Universe.set_universe([(1,2), (3,4)])  # disconnected graph
        self.assertEqual(Universe.edge_vertex_universe(), [(1,2), 1, 2, (3,4), 3, 4])

    def test_constructors(self):
        gs = EdgeVertexSetSet()
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(len(gs), 0)

        gs = EdgeVertexSetSet([])
        self.assertEqual(len(gs), 0)

        gs = EdgeVertexSetSet([ev1, [(3,1), 3, 1]])
        self.assertEqual(len(gs), 2)
        self.assertTrue(ev1 in gs)
        self.assertTrue(ev2 in gs)
        self.assertFalse(ev2 + [1, 2, 3] in gs)
        self.assertTrue((1, 2) in gs)
        self.assertFalse((2, 4) in gs)
        self.assertTrue(1 in gs)
        self.assertFalse(4 in gs)

        gs = EdgeVertexSetSet({})
        self.assertEqual(len(gs), 2**4)

        self.assertRaises(KeyError, EdgeVertexSetSet, [(1,4)])
        self.assertRaises(KeyError, EdgeVertexSetSet, [[(1,4)]])

        # copy constructor
        gs1 = EdgeVertexSetSet([ev0, ev12, ev13])
        gs2 = gs1.copy()
        self.assertTrue(isinstance(gs2, EdgeVertexSetSet))
        gs1.clear()
        self.assertEqual(gs1, EdgeVertexSetSet())
        self.assertEqual(gs2, EdgeVertexSetSet([ev0, ev12, ev13]))

        # repr
        gs = EdgeVertexSetSet([ev0, ev12, ev13])
        self.assertEqual(
            repr(gs),
            "EdgeVertexSetSet([[], [(1, 2), (1, 3), 1, 2, 3], [(1, 2), (2, 4), 1, 2, 4]])")

        gs = EdgeVertexSetSet({})
        self.assertEqual(
            repr(gs),
            "EdgeVertexSetSet([[], [(1, 2), 1, 2], [(1, 3), 1, 3], [(2, 4), 2, 4], [(3, 4 ...")

    def test_show_messages(self):
        a = EdgeVertexSetSet.show_messages()
        b = EdgeVertexSetSet.show_messages(True)
        self.assertTrue(b)
        c = EdgeVertexSetSet.show_messages(False)
        self.assertTrue(c)
        d = EdgeVertexSetSet.show_messages(a)
        self.assertFalse(d)

    def test_comparison(self):
        gs = EdgeVertexSetSet([ev12])
        self.assertEqual(gs, EdgeVertexSetSet([ev12]))
        self.assertNotEqual(gs, EdgeVertexSetSet([ev12 + [1, 2, 3, 4]]))
        self.assertNotEqual(gs, EdgeVertexSetSet([ev13]))

        # __bool__
        self.assertTrue(gs)
        self.assertFalse(EdgeVertexSetSet())

        v = [ev0, ev12, ev13]
        gs = EdgeVertexSetSet(v)
        self.assertTrue(gs.isdisjoint(EdgeVertexSetSet([ev1, ev123])))
        self.assertFalse(gs.isdisjoint(EdgeVertexSetSet([ev1, ev12])))

        self.assertTrue(gs.issubset(EdgeVertexSetSet(v)))
        self.assertFalse(gs.issubset(EdgeVertexSetSet([ev0, ev12])))
        self.assertTrue(gs <= EdgeVertexSetSet(v))
        self.assertFalse(gs <= EdgeVertexSetSet([ev0, ev12]))
        self.assertTrue(gs < EdgeVertexSetSet([ev0, ev1, ev12, ev13]))
        self.assertFalse(gs < EdgeVertexSetSet(v))

        self.assertTrue(gs.issuperset(EdgeVertexSetSet(v)))
        self.assertFalse(gs.issuperset(EdgeVertexSetSet([ev1, ev12])))
        self.assertTrue(gs >= EdgeVertexSetSet(v))
        self.assertFalse(gs >= EdgeVertexSetSet([ev1, ev12]))
        self.assertTrue(gs > EdgeVertexSetSet([[], ev12]))
        self.assertFalse(gs > EdgeVertexSetSet(v))

    def test_unary_operators(self):
        gs = EdgeVertexSetSet([ev0, ev1, ev12, ev24, ev123, ev1234, ev134, ev14, ev4])

        self.assertTrue(isinstance(gs.smaller(5), EdgeVertexSetSet))
        self.assertEqual(gs.smaller(5), EdgeVertexSetSet([ev0, ev1, ev4]))
        self.assertTrue(isinstance(gs.larger(5), EdgeVertexSetSet))
        self.assertEqual(gs.larger(5), EdgeVertexSetSet([ev123, ev1234, ev134, ev14]))
        self.assertTrue(isinstance(gs.graph_size(5), EdgeVertexSetSet))
        self.assertEqual(gs.graph_size(5), EdgeVertexSetSet([ev12, ev24]))
        self.assertTrue(isinstance(gs.len(5), EdgeVertexSetSet))
        self.assertEqual(gs.len(5), EdgeVertexSetSet([ev12, ev24]))

        gs = EdgeVertexSetSet([ev12, ev123, ev234])
        self.assertTrue(isinstance(gs.minimal(), EdgeVertexSetSet))
        self.assertEqual(gs.minimal(), EdgeVertexSetSet([ev12, ev234]))
        self.assertTrue(isinstance(gs.maximal(), EdgeVertexSetSet))
        self.assertEqual(gs.maximal(), EdgeVertexSetSet([ev123, ev234]))

    def test_binary_operators(self):
        u = [ev0, ev1, ev12, ev123, ev1234, ev134, ev14, ev4]
        v = [ev12, ev14, ev23, ev34]

        gs = EdgeVertexSetSet(u) | EdgeVertexSetSet(v)
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(
            gs, EdgeVertexSetSet([ev0, ev1, ev12, ev123, ev1234, ev134, ev14, ev23, ev34, ev4]))
        gs = EdgeVertexSetSet(u).union(EdgeVertexSetSet(u), EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(
            gs, EdgeVertexSetSet([ev0, ev1, ev12, ev123, ev1234, ev134, ev14, ev23, ev34, ev4]))

        gs = EdgeVertexSetSet(u)
        gs |= EdgeVertexSetSet(v)
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(
            gs, EdgeVertexSetSet([ev0, ev1, ev12, ev123, ev1234, ev134, ev14, ev23, ev34, ev4]))
        gs = EdgeVertexSetSet(u)
        gs.update(EdgeVertexSetSet(u), EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(
            gs, EdgeVertexSetSet([ev0, ev1, ev12, ev123, ev1234, ev134, ev14, ev23, ev34, ev4]))

        gs = EdgeVertexSetSet(u) & EdgeVertexSetSet(v)
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev12, ev14]))
        gs = EdgeVertexSetSet(u).intersection(EdgeVertexSetSet(u), EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev12, ev14]))

        gs = EdgeVertexSetSet(u)
        gs &= EdgeVertexSetSet(v)
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev12, ev14]))
        gs = EdgeVertexSetSet(u)
        gs.intersection_update(EdgeVertexSetSet(u), EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev12, ev14]))

        gs = EdgeVertexSetSet(u) - EdgeVertexSetSet(v)
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev0, ev1, ev123, ev1234, ev134, ev4]))
        gs = EdgeVertexSetSet(u).difference(EdgeVertexSetSet(), EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev0, ev1, ev123, ev1234, ev134, ev4]))

        gs = EdgeVertexSetSet(u)
        gs -= EdgeVertexSetSet(v)
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev0, ev1, ev123, ev1234, ev134, ev4]))
        gs = EdgeVertexSetSet(u)
        gs.difference_update(EdgeVertexSetSet(), EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev0, ev1, ev123, ev1234, ev134, ev4]))

        gs = EdgeVertexSetSet(u) ^ EdgeVertexSetSet(v)
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev0, ev1, ev123, ev1234, ev134, ev23, ev34, ev4]))
        gs = EdgeVertexSetSet(u).symmetric_difference(EdgeVertexSetSet(), EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev0, ev1, ev123, ev1234, ev134, ev23, ev34, ev4]))

        gs = EdgeVertexSetSet(u)
        gs ^= EdgeVertexSetSet(v)
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev0, ev1, ev123, ev1234, ev134, ev23, ev34, ev4]))
        gs = EdgeVertexSetSet(u)
        gs.symmetric_difference_update(EdgeVertexSetSet(), EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev0, ev1, ev123, ev1234, ev134, ev23, ev34, ev4]))

        v = [ev12, ev14, ev23, ev34]
        gs = EdgeVertexSetSet(u).join(EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(
            gs, EdgeVertexSetSet([ev12, ev123, ev124, ev1234, ev134, ev14, ev23, ev234, ev34]))

        gs1 = EdgeVertexSetSet(u).meet(EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        meets = set()
        for i in u:
            for j in v:
                meets.add(frozenset(set(i) & set(j)))
        gs2 = EdgeVertexSetSet(meets)
        self.assertEqual(gs1, gs2)

        v = [ev12, ev14, ev23, ev34]
        gs = EdgeVertexSetSet(u).subgraphs(EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev0, ev1, ev12, ev14, ev4]))

        gs = EdgeVertexSetSet(u).supergraphs(EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev12, ev123, ev1234, ev134, ev14]))

        gs = EdgeVertexSetSet(u).non_subgraphs(EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev123, ev1234, ev134]))

        gs = EdgeVertexSetSet(u).non_supergraphs(EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev0, ev1, ev4]))

        gs1 = EdgeVertexSetSet({}) - EdgeVertexSetSet([ev1, ev34])

        gs2 = gs1.including(EdgeVertexSetSet([ev1, ev2]))
        self.assertTrue(isinstance(gs2, EdgeVertexSetSet))
        self.assertEqual(len(gs2), 11)

        gs2 = gs1.including(ev1)
        self.assertTrue(isinstance(gs2, EdgeVertexSetSet))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.including((2,1))
        self.assertTrue(isinstance(gs2, EdgeVertexSetSet))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.including(1)
        self.assertTrue(isinstance(gs2, EdgeVertexSetSet))
        self.assertEqual(len(gs2), 11)

        self.assertRaises(KeyError, gs1.including, (1, 4))
        self.assertRaises(KeyError, gs1.including, 5)

        gs2 = gs1.excluding(EdgeVertexSetSet([ev1, ev2]))
        self.assertTrue(isinstance(gs2, EdgeVertexSetSet))
        self.assertEqual(len(gs2), 3)

        gs2 = gs1.excluding(ev1)
        self.assertTrue(isinstance(gs2, EdgeVertexSetSet))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.excluding(e2)
        self.assertTrue(isinstance(gs2, EdgeVertexSetSet))
        self.assertEqual(len(gs2), 6)

        gs2 = gs1.excluding(1)
        self.assertTrue(isinstance(gs2, EdgeVertexSetSet))
        self.assertEqual(len(gs2), 3)

        self.assertRaises(KeyError, gs1.excluding, (1, 4))
        self.assertRaises(KeyError, gs1.excluding, 5)

        v = [ev12, ev14, ev23, ev34]
        gs = EdgeVertexSetSet(u).included(EdgeVertexSetSet(v))
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev0, ev1, ev12, ev14, ev4]))

        gs = EdgeVertexSetSet(u).included(ev12)
        self.assertTrue(isinstance(gs, EdgeVertexSetSet))
        self.assertEqual(gs, EdgeVertexSetSet([ev0, ev1, ev12]))

    def capacity(self):
        gs = EdgeVertexSetSet()
        self.assertFalse(gs)

        gs = EdgeVertexSetSet([ev0, ev12, ev13])
        self.assertTrue(gs)

        self.assertEqual(len(gs), 3)
        self.assertEqual(gs.len(), 3)

    def test_iterators(self):
        gs1 = EdgeVertexSetSet([ev0, ev12, ev13])
        gs2 = EdgeVertexSetSet()
        for g in gs1:
            self.assertTrue(isinstance(g, list))
            gs2 = gs2 | EdgeVertexSetSet([g])
        self.assertEqual(gs1, EdgeVertexSetSet([ev0, ev12, ev13]))
        self.assertEqual(gs1, gs2)

        gs2 = EdgeVertexSetSet()
        for g in gs1:
            self.assertTrue(isinstance(g, list))
            gs2 = gs2 | EdgeVertexSetSet([g])
        self.assertEqual(gs1, gs2)

        gs1 = EdgeVertexSetSet([ev0, ev12, ev13])
        gs2 = EdgeVertexSetSet()
        for g in gs1.rand_iter():
            self.assertTrue(isinstance(g, list))
            gs2 = gs2 | EdgeVertexSetSet([g])
        self.assertEqual(gs1, gs2)

        gen = gs1.rand_iter()
        self.assertTrue(isinstance(next(gen), list))

        gs = EdgeVertexSetSet([ev0, ev1, ev12, ev123, ev1234, ev134, ev14, ev4])
        r = []
        for g in gs.max_iter():
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], ev12)
        self.assertEqual(r[1], ev123)
        self.assertTrue(r[2] == ev1234 or r[2] == ev0) # same weight

        r = []
        for g in gs.max_iter({e1: -.3, e2: .2, e3: .2, e4: -.4, 1: -.3, 2: .5, 3: .1, 4: -.4}):
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], ev12)
        self.assertTrue((r[1] == ev0 and r[2] == ev123) or (r[1] == ev123 and r[2] == ev0)) # same weight

        r = []
        for g in gs.min_iter():
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], ev14)
        self.assertEqual(r[1], ev4)
        self.assertEqual(r[2], ev134)

        r = []
        for g in gs.min_iter({e1: -.3, e2: .2, e3: .2, e4: -.4, 1: -.3, 2: .5, 3: .1, 4: -.4}):
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], ev14)
        self.assertEqual(r[1], ev4)
        self.assertEqual(r[2], ev134)

        gs = EdgeVertexSetSet([[]])
        self.assertEqual(list(gs.min_iter()), [[]])

    def test_lookup(self):
        gs1 = EdgeVertexSetSet([ev1, ev12])

        self.assertTrue(ev12 in gs1)
        self.assertTrue(ev2 not in gs1)
        self.assertTrue(e1 in gs1)
        self.assertTrue(e4 not in gs1)
        self.assertTrue(1 in gs1)
        self.assertTrue(4 not in gs1)

    def test_modifiers(self):
        v = [ev0, ev12, ev13]
        gs = EdgeVertexSetSet(v)
        gs.add(ev1)
        self.assertTrue(ev1 in gs)

        gs.remove(ev1)
        self.assertTrue(ev1 not in gs)
        self.assertRaises(KeyError, gs.remove, ev1)

        gs.add(ev0)
        gs.discard(ev0)
        self.assertTrue(ev0 not in gs)
        gs.discard(ev0)  # no exception raised

        gs = EdgeVertexSetSet(v)
        gs.add_vertex(4)
        self.assertEqual(gs, EdgeVertexSetSet([[4], ev12 + [4], ev13]))

        v = [ev1, ev12, ev13]
        gs = EdgeVertexSetSet(v)
        g = gs.pop()
        self.assertTrue(isinstance(g, list))
        self.assertTrue(g not in gs)
        self.assertEqual(gs | EdgeVertexSetSet([g]), EdgeVertexSetSet(v))

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

    def test_probability(self):
        p = {e1: .9, e2: .8, e3: .7, e4: .6, 1:.5, 2:.4, 3:.3, 4:.2}

        gs = EdgeVertexSetSet()
        self.assertEqual(gs.probability(p), 0)

        gs = EdgeVertexSetSet([ev0])
        self.assertAlmostEqual(gs.probability(p), .0004032)

        gs = EdgeVertexSetSet([ev1])
        self.assertAlmostEqual(gs.probability(p), .0024192)

        gs = EdgeVertexSetSet([ev2])
        self.assertAlmostEqual(gs.probability(p), .0006912)

        gs = EdgeVertexSetSet([ev12, ev13])
        self.assertAlmostEqual(gs.probability(p), .0055584)

        gs = EdgeVertexSetSet([ev1234])
        self.assertAlmostEqual(gs.probability(p), .0036288)

        gs = EdgeVertexSetSet([ev0, ev1, ev2, ev12, ev13, ev1234])
        self.assertAlmostEqual(gs.probability(p), .0127008)

    def test_cost_le(self):
        Universe.set_universe([e1, e2, e3, e4])
        gs = EdgeVertexSetSet([ev0, ev1, ev2, ev3, ev4, ev12, ev14, ev134, ev234, ev1234])

        costs = {e1: 2, e2: 14, e3: 4, e4: 7, 1: 3, 2: 1, 3: 4, 4: 3}
        cost_bound = 24

        small_cost_gs = gs.cost_le(costs, cost_bound)
        self.assertIn(ev0, small_cost_gs) # cost: 0
        self.assertIn(ev1, small_cost_gs) # cost: 6
        self.assertIn(ev2, small_cost_gs) # cost: 21
        self.assertIn(ev3, small_cost_gs) # cost: 8
        self.assertIn(ev4, small_cost_gs) # cost: 14
        self.assertIn(ev12, small_cost_gs) # cost: 24
        self.assertIn(ev14, small_cost_gs) # cost: 20
        self.assertIn(ev134, small_cost_gs) # cost: 24
        self.assertNotIn(ev234, small_cost_gs) # cost: 36
        self.assertNotIn(ev1234, small_cost_gs) # cost: 38

    def test_cost_ge(self):
        Universe.set_universe([e1, e2, e3, e4])
        gs = EdgeVertexSetSet([ev0, ev1, ev2, ev3, ev4, ev12, ev14, ev134, ev234, ev1234])

        costs = {e1: 2, e2: 14, e3: 4, e4: 7, 1: 3, 2: 1, 3: 4, 4: 3}
        cost_bound = 24

        large_cost_gs = gs.cost_ge(costs, cost_bound)
        self.assertNotIn(ev0, large_cost_gs) # cost: 0
        self.assertNotIn(ev1, large_cost_gs) # cost: 6
        self.assertNotIn(ev2, large_cost_gs) # cost: 21
        self.assertNotIn(ev3, large_cost_gs) # cost: 8
        self.assertNotIn(ev4, large_cost_gs) # cost: 14
        self.assertIn(ev12, large_cost_gs) # cost: 24
        self.assertNotIn(ev14, large_cost_gs) # cost: 20
        self.assertIn(ev134, large_cost_gs) # cost: 24
        self.assertIn(ev234, large_cost_gs) # cost: 36
        self.assertIn(ev1234, large_cost_gs) # cost: 38

    def test_cost_eq(self):
        Universe.set_universe([e1, e2, e3, e4])
        gs = EdgeVertexSetSet([ev0, ev1, ev2, ev3, ev4, ev12, ev14, ev134, ev234, ev1234])

        costs = {e1: 2, e2: 14, e3: 4, e4: 7, 1: 3, 2: 1, 3: 4, 4: 3}
        cost_bound = 24

        equal_cost_gs = gs.cost_eq(costs, cost_bound)
        self.assertNotIn(ev0, equal_cost_gs) # cost: 0
        self.assertNotIn(ev1, equal_cost_gs) # cost: 6
        self.assertNotIn(ev2, equal_cost_gs) # cost: 21
        self.assertNotIn(ev3, equal_cost_gs) # cost: 8
        self.assertNotIn(ev4, equal_cost_gs) # cost: 14
        self.assertIn(ev12, equal_cost_gs) # cost: 24
        self.assertNotIn(ev14, equal_cost_gs) # cost: 20
        self.assertIn(ev134, equal_cost_gs) # cost: 24
        self.assertNotIn(ev234, equal_cost_gs) # cost: 36
        self.assertNotIn(ev1234, equal_cost_gs) # cost: 38

    def test_io(self):
        gs = EdgeVertexSetSet()
        st = gs.dumps()
        self.assertEqual(st, "B\n.\n")
        gs = EdgeVertexSetSet.loads(st)
        self.assertEqual(gs, EdgeVertexSetSet())

        gs = EdgeVertexSetSet([ev0])
        st = gs.dumps()
        self.assertEqual(st, "T\n.\n")
        gs = EdgeVertexSetSet.loads(st)
        self.assertEqual(gs, EdgeVertexSetSet([ev0]))

        v = [ev0, ev1, ev12, ev123, ev1234, ev134, ev14, ev4]
        gs = EdgeVertexSetSet(v)
        st = gs.dumps()
        gs = EdgeVertexSetSet.loads(st)
        self.assertEqual(gs, EdgeVertexSetSet(v))

        with tempfile.TemporaryFile() as f:
            gs.dump(f)
            f.seek(0)
            gs = EdgeVertexSetSet.load(f)
            self.assertEqual(gs, EdgeVertexSetSet(v))

        for gs1 in [EdgeVertexSetSet([]), EdgeVertexSetSet([ev0]), EdgeVertexSetSet([ev1])]:
            with tempfile.TemporaryFile() as f:
                gs1.dump(f)
                f.seek(0)
                gs2 = EdgeVertexSetSet.load(f)
                self.assertEqual(gs1, gs2)


if __name__ == '__main__':
    unittest.main()
