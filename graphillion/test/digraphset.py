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

from graphillion import GraphSet, DiGraphSet
import tempfile
import unittest


e1 = (1, 2)
e2 = (1, 3)
e3 = (2, 4)
e4 = (3, 4)

e1r = (2, 1)

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

g1r = [e1r]
g1r2 = [e1r, e2]
g1r4 = [e1r, e4]

"""
1 <-> 2 <-> 3
^     ^     ^
|     |     |
v     v     v
4 <-> 5 <-> 6
"""

f1 = (1, 2)
f2 = (1, 4)
f3 = (2, 3)
f4 = (2, 5)
f5 = (3, 6)
f6 = (4, 5)
f7 = (5, 6)
f8 = (2, 1)
f9 = (4, 1)
f10 = (3, 2)
f11 = (5, 2)
f12 = (6, 3)
f13 = (5, 4)
f14 = (6, 5)
universe_edges = [f1, f2, f3, f4, f5, f6, f7, f8, f9,
                  f10, f11, f12, f13, f14]

class TestDiGraphSet(unittest.TestCase):

    def setUp(self):
        DiGraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                                traversal='bfs', source=1)

    def tearDown(self):
        pass

    def test_init(self):
        DiGraphSet.set_universe([('i', 'ii')])
        self.assertEqual(DiGraphSet.universe(), [('i', 'ii')])

        DiGraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                                traversal='bfs', source=1)
        self.assertEqual(DiGraphSet.universe(),
                         [e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)])

        DiGraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                                traversal='dfs', source=1)
        self.assertEqual(DiGraphSet.universe(),
                         [e2 + (-.2,), e4 + (.4,), e1 + (.3,), e3 + (-.2,)])

        DiGraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                                traversal='greedy', source=3)
        self.assertEqual(DiGraphSet.universe(),
                         [e2 + (-.2,), e1 + (.3,), e3 + (-.2,), e4 + (.4,)])

#        self.assertRaises(KeyError, DiGraphSet.set_universe, [(1, 2), (2, 1)])

        DiGraphSet.set_universe([(1, 2), (3, 4)])  # disconnected graph
        self.assertEqual(DiGraphSet.universe(), [(1, 2), (3, 4)])

    def test_constructors(self):
        self.setUp()
        gs = DiGraphSet()
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(len(gs), 0)

        gs = DiGraphSet([])
        self.assertEqual(len(gs), 0)

        gs = DiGraphSet([g1, [(3, 1)]])
        self.assertEqual(len(gs), 2)
        self.assertTrue(g1 in gs)
        self.assertTrue(g2 in gs)

        gs = DiGraphSet({})
        self.assertEqual(len(gs), 2**4)

        gs = DiGraphSet({'include': [e1, e2], 'exclude': [(4, 3)]})
        self.assertEqual(len(gs), 2)
        self.assertTrue(g12 in gs)
        self.assertTrue(g123 in gs)

        self.assertRaises(KeyError, DiGraphSet, [(1, 4)])
        self.assertRaises(KeyError, DiGraphSet, [[(1, 4)]])
        self.assertRaises(KeyError, DiGraphSet, {'include': [(1, 4)]})

        # copy constructor
        gs1 = DiGraphSet([g0, g12, g13])
        gs2 = gs1.copy()
        self.assertTrue(isinstance(gs2, DiGraphSet))
        gs1.clear()
        self.assertEqual(gs1, DiGraphSet())
        self.assertEqual(gs2, DiGraphSet([g0, g12, g13]))

        # repr
        gs = DiGraphSet([g0, g12, g13])
        self.assertEqual(
            repr(gs),
            "DiGraphSet([[], [(1, 2), (1, 3)], [(1, 2), (2, 4)]])")

        gs = DiGraphSet({})
        self.assertEqual(
            repr(gs),
            "DiGraphSet([[], [(1, 2)], [(1, 3)], [(2, 4)], [(3, 4)], [(1, 2), (1, 3)], [( ...")

# def test_graphs(self):

#    def test_linear_constraints(self):

    def test_show_messages(self):
        a = DiGraphSet.show_messages()
        b = DiGraphSet.show_messages(True)
        self.assertTrue(b)
        c = DiGraphSet.show_messages(False)
        self.assertTrue(c)
        d = DiGraphSet.show_messages(a)
        self.assertFalse(d)

    def test_comparison(self):
        self.setUp()
        gs = DiGraphSet([g12])
        self.assertEqual(gs, DiGraphSet([g12]))
        self.assertNotEqual(gs, DiGraphSet([g13]))

        # __bool__
        self.assertTrue(gs)
        self.assertFalse(DiGraphSet())

        v = [g0, g12, g13]
        gs = DiGraphSet(v)
        self.assertTrue(gs.isdisjoint(DiGraphSet([g1, g123])))
        self.assertFalse(gs.isdisjoint(DiGraphSet([g1, g12])))

        self.assertTrue(gs.issubset(DiGraphSet(v)))
        self.assertFalse(gs.issubset(DiGraphSet([g0, g12])))
        self.assertTrue(gs <= DiGraphSet(v))
        self.assertFalse(gs <= DiGraphSet([g0, g12]))
        self.assertTrue(gs < DiGraphSet([g0, g1, g12, g13]))
        self.assertFalse(gs < DiGraphSet(v))

        self.assertTrue(gs.issuperset(DiGraphSet(v)))
        self.assertFalse(gs.issuperset(DiGraphSet([g1, g12])))
        self.assertTrue(gs >= DiGraphSet(v))
        self.assertFalse(gs >= DiGraphSet([g1, g12]))
        self.assertTrue(gs > DiGraphSet([[], g12]))
        self.assertFalse(gs > DiGraphSet(v))

    def test_unary_operators(self):
        self.setUp()
        gs = DiGraphSet([g0, g1, g12, g123, g1234, g134, g14, g4])

        self.assertTrue(isinstance(~gs, DiGraphSet))
        self.assertEqual(~gs, DiGraphSet(
            [g124, g13, g2, g23, g234, g24, g3, g34]))

        self.assertTrue(isinstance(gs.smaller(3), DiGraphSet))
        self.assertEqual(gs.smaller(3), DiGraphSet([g0, g1, g12, g14, g4]))
        self.assertTrue(isinstance(gs.larger(3), DiGraphSet))
        self.assertEqual(gs.larger(3), DiGraphSet([g1234]))
        self.assertTrue(isinstance(gs.graph_size(3), DiGraphSet))
        self.assertEqual(gs.graph_size(3), DiGraphSet([g123, g134]))
        self.assertTrue(isinstance(gs.len(3), DiGraphSet))
        self.assertEqual(gs.len(3), DiGraphSet([g123, g134]))

        gs = DiGraphSet([g12, g123, g234])
        self.assertTrue(isinstance(gs.minimal(), DiGraphSet))
        self.assertEqual(gs.minimal(), DiGraphSet([g12, g234]))
        self.assertTrue(isinstance(gs.maximal(), DiGraphSet))
        self.assertEqual(gs.maximal(), DiGraphSet([g123, g234]))

        gs = DiGraphSet([g12, g14, g23, g34])
        self.assertTrue(isinstance(gs.blocking(), DiGraphSet))
        self.assertEqual(
            gs.blocking(), DiGraphSet([g123, g1234, g124, g13, g134, g234, g24]))

    def test_binary_operators(self):
        self.setUp()
        u = [g0, g1, g12, g123, g1234, g134, g14, g4]
        v = [g12, g14, g23, g34]

        gs = DiGraphSet(u) | DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(
            gs, DiGraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))
        gs = DiGraphSet(u).union(DiGraphSet(u), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(
            gs, DiGraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))

        gs = DiGraphSet(u)
        gs |= DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(
            gs, DiGraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))
        gs = DiGraphSet(u)
        gs.update(DiGraphSet(u), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(
            gs, DiGraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))

        gs = DiGraphSet(u) & DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g12, g14]))
        gs = DiGraphSet(u).intersection(DiGraphSet(u), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g12, g14]))

        gs = DiGraphSet(u)
        gs &= DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g12, g14]))
        gs = DiGraphSet(u)
        gs.intersection_update(DiGraphSet(u), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g12, g14]))

        gs = DiGraphSet(u) - DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g123, g1234, g134, g4]))
        gs = DiGraphSet(u).difference(DiGraphSet(), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g123, g1234, g134, g4]))

        gs = DiGraphSet(u)
        gs -= DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g123, g1234, g134, g4]))
        gs = DiGraphSet(u)
        gs.difference_update(DiGraphSet(), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g123, g1234, g134, g4]))

        gs = DiGraphSet(u) ^ DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet(
            [g0, g1, g123, g1234, g134, g23, g34, g4]))
        gs = DiGraphSet(u).symmetric_difference(DiGraphSet(), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet(
            [g0, g1, g123, g1234, g134, g23, g34, g4]))

        gs = DiGraphSet(u)
        gs ^= DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet(
            [g0, g1, g123, g1234, g134, g23, g34, g4]))
        gs = DiGraphSet(u)
        gs.symmetric_difference_update(DiGraphSet(), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet(
            [g0, g1, g123, g1234, g134, g23, g34, g4]))

        v = [g12]
        gs = DiGraphSet(u) / DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g3, g34]))
        gs = DiGraphSet(u).quotient(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g3, g34]))

        gs = DiGraphSet(u)
        gs /= DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g3, g34]))
        gs = DiGraphSet(u)
        gs.quotient_update(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g3, g34]))

        gs = DiGraphSet(u) % DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g134, g14, g4]))
        gs = DiGraphSet(u).remainder(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g134, g14, g4]))

        gs = DiGraphSet(u)
        gs %= DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g134, g14, g4]))
        gs = DiGraphSet(u)
        gs.remainder_update(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g134, g14, g4]))

        gs = DiGraphSet(u).complement()
        self.assertEqual(gs, DiGraphSet(
            [g0, g123, g1234, g2, g23, g234, g34, g4]))

        v = [g12, g14, g23, g34]
        gs = DiGraphSet(u).join(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(
            gs, DiGraphSet([g12, g123, g124, g1234, g134, g14, g23, g234, g34]))

        gs = DiGraphSet(u).meet(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet(
            [g0, g1, g12, g14, g2, g23, g3, g34, g4]))

        v = [g12, g14, g23, g34]
        gs = DiGraphSet(u).subgraphs(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g12, g14, g4]))

        gs = DiGraphSet(u).supergraphs(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g12, g123, g1234, g134, g14]))

        gs = DiGraphSet(u).non_subgraphs(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g123, g1234, g134]))

        gs = DiGraphSet(u).non_supergraphs(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g4]))

        gs1 = DiGraphSet({}) - DiGraphSet([g1, g34])

        gs2 = gs1.including(DiGraphSet([g1, g2]))
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 11)

        gs2 = gs1.including(g1)
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.including((2, 1))
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.including(1)
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 11)

        self.assertRaises(KeyError, gs1.including, (1, 4))
        self.assertRaises(KeyError, gs1.including, 5)

        gs2 = gs1.excluding(DiGraphSet([g1, g2]))
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 3)

        gs2 = gs1.excluding(g1)
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.excluding(e2)
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 6)

        gs2 = gs1.excluding(1)
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 3)

        self.assertRaises(KeyError, gs1.excluding, (1, 4))
        self.assertRaises(KeyError, gs1.excluding, 5)

        v = [g12, g14, g23, g34]
        gs = DiGraphSet(u).included(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g12, g14, g4]))

        gs = DiGraphSet(u).included(g12)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g12]))

    def capacity(self):
        self.setUp()
        gs = DiGraphSet()
        self.assertFalse(gs)

        gs = DiGraphSet([g0, g12, g13])
        self.assertTrue(gs)

        self.assertEqual(len(gs), 3)
        self.assertEqual(gs.len(), 3)

    def test_iterators(self):
        self.setUp()
        gs1 = DiGraphSet([g0, g12, g13])
        gs2 = DiGraphSet()
        for g in gs1:
            self.assertTrue(isinstance(g, list))
            gs2 = gs2 | DiGraphSet([g])
        self.assertEqual(gs1, DiGraphSet([g0, g12, g13]))
        self.assertEqual(gs1, gs2)

        gs2 = DiGraphSet()
        for g in gs1:
            self.assertTrue(isinstance(g, list))
            gs2 = gs2 | DiGraphSet([g])
        self.assertEqual(gs1, gs2)

        gs1 = DiGraphSet([g0, g12, g13])
        gs2 = DiGraphSet()
        for g in gs1.rand_iter():
            self.assertTrue(isinstance(g, list))
            gs2 = gs2 | DiGraphSet([g])
        self.assertEqual(gs1, gs2)

        gen = gs1.rand_iter()
        self.assertTrue(isinstance(next(gen), list))

        gs = DiGraphSet([g0, g1, g12, g123, g1234, g134, g14, g4])
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

        gs = DiGraphSet([[]])
        self.assertEqual(list(gs.min_iter()), [[]])

    def test_lookup(self):
        self.setUp()
        gs1 = DiGraphSet([g1, g12])

        self.assertTrue(g12 in gs1)
        self.assertTrue(g2 not in gs1)
        self.assertTrue(e1 in gs1)
        self.assertTrue(e4 not in gs1)
        self.assertTrue(1 in gs1)
        self.assertTrue(4 not in gs1)

    def test_modifiers(self):
        self.setUp()
        v = [g0, g12, g13]
        gs = DiGraphSet(v)
        gs.add(g1)
        self.assertTrue(g1 in gs)

        gs.remove(g1)
        self.assertTrue(g1 not in gs)
        self.assertRaises(KeyError, gs.remove, g1)

        gs.add(g0)
        gs.discard(g0)
        self.assertTrue(g0 not in gs)
        gs.discard(g0)  # no exception raised

        gs = DiGraphSet(v)
        gs.add(e2)
        self.assertEqual(gs, DiGraphSet([g12, g123, g2]))

        gs = DiGraphSet(v)
        gs.remove(e2)
        self.assertEqual(gs, DiGraphSet([g0, g1, g13]))
        self.assertRaises(KeyError, gs.remove, e4)

        gs = DiGraphSet(v)
        gs.discard(e2)
        self.assertEqual(gs, DiGraphSet([g0, g1, g13]))
        gs.discard(e4)  # no exception raised

        v = [g1, g12, g13]
        gs = DiGraphSet(v)
        g = gs.pop()
        self.assertTrue(isinstance(g, list))
        self.assertTrue(g not in gs)
        self.assertEqual(gs | DiGraphSet([g]), DiGraphSet(v))

        self.assertTrue(gs)
        gs.clear()
        self.assertFalse(gs)

        self.assertRaises(KeyError, gs.pop)

        self.assertRaises(KeyError, gs.add, [(1, 4)])
        self.assertRaises(KeyError, gs.remove, [(1, 4)])
        self.assertRaises(KeyError, gs.discard, [(1, 4)])

        self.assertRaises(KeyError, gs.add, (1, 4))
        self.assertRaises(KeyError, gs.remove, (1, 4))
        self.assertRaises(KeyError, gs.discard, (1, 4))

        u = [g0, g1, g12, g123, g1234, g134, g14, g4]
        gs = DiGraphSet(u)
        gs.flip(e1)
        self.assertEqual(gs, DiGraphSet([g0, g1, g14, g2, g23, g234, g34, g4]))

#    def test_probability(self):

    def test_cost_le(self):
        DiGraphSet.set_universe([e1, e1r, e2, e3, e4])
        gs = DiGraphSet([g0, g1, g2, g3, g4, g12, g14, g134, g234, g1234, g1r, g1r2, g1r4])

        costs = {e1: 2, e1r: 6, e2: 14, e3: 4, e4: 7}
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
        self.assertIn(g1r, small_cost_gs) # cost: 6
        self.assertNotIn(g1r2, small_cost_gs) # cost: 20
        self.assertIn(g1r4, small_cost_gs) # cost: 13

    def test_cost_ge(self):
        DiGraphSet.set_universe([e1, e1r, e2, e3, e4])
        gs = DiGraphSet([g0, g1, g2, g3, g4, g12, g14, g134, g234, g1234, g1r, g1r2, g1r4])

        costs = {e1: 2, e1r: 6, e2: 14, e3: 4, e4: 7}
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
        self.assertNotIn(g1r, large_cost_gs) # cost: 6
        self.assertIn(g1r2, large_cost_gs) # cost: 20
        self.assertIn(g1r4, large_cost_gs) # cost: 13

    def test_cost_eq(self):
        DiGraphSet.set_universe([e1, e1r, e2, e3, e4])
        gs = DiGraphSet([g0, g1, g2, g3, g4, g12, g14, g134, g234, g1234, g1r, g1r2, g1r4])

        costs = {e1: 2, e1r: 6, e2: 14, e3: 4, e4: 7}
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
        self.assertNotIn(g1r, equal_cost_gs) # cost: 6
        self.assertNotIn(g1r2, equal_cost_gs) # cost: 20
        self.assertIn(g1r4, equal_cost_gs) # cost: 13

    def test_remove_some_edge(self):
        DiGraphSet.set_universe([e1, e1r, e2, e3, e4])

        gs = DiGraphSet([])
        self.assertEqual(gs.remove_some_edge(), DiGraphSet())

        gs = DiGraphSet([g0])
        self.assertEqual(gs.remove_some_edge(), DiGraphSet())

        gs = DiGraphSet([g1r])
        self.assertEqual(gs.remove_some_edge(), DiGraphSet([g0]))

        gs = DiGraphSet([g0, g1r])
        self.assertEqual(gs.remove_some_edge(), DiGraphSet([g0]))

        gs = DiGraphSet([g1, g12])
        self.assertEqual(gs.remove_some_edge(), DiGraphSet([g0, g1, g2]))

        gs1 = DiGraphSet([g0, g4, g12, g234])
        gs2 = DiGraphSet([g0, g1, g2, g23, g24, g34])
        self.assertEqual(gs1.remove_some_edge(), gs2)

    def test_add_some_edge(self):
        DiGraphSet.set_universe([e1, e1r, e2, e3, e4])

        gs = DiGraphSet([])
        self.assertEqual(gs.add_some_edge(), DiGraphSet())

        gs = DiGraphSet([g0])
        self.assertEqual(gs.add_some_edge(), DiGraphSet([g1, g2, g3, g4, g1r]))

        DiGraphSet.set_universe([e1, e2, e3, e4])

        gs = DiGraphSet([g1])
        self.assertEqual(gs.add_some_edge(), DiGraphSet([g12, g13, g14]))

        gs = DiGraphSet([g0, g1])
        self.assertEqual(gs.add_some_edge(),
                         DiGraphSet([g1, g2, g3, g4, g12, g13, g14]))

        gs = DiGraphSet([g1, g12])
        self.assertEqual(gs.add_some_edge(),
                         DiGraphSet([g12, g13, g14, g123, g124]))

        gs1 = DiGraphSet([g0, g4, g12, g234])
        gs2 = DiGraphSet([g1, g2, g3, g4, g14, g24, g34, g123, g124, g1234])
        self.assertEqual(gs1.add_some_edge(), gs2)

    def test_remove_add_some_edges(self):
        DiGraphSet.set_universe([e1, e1r, e2, e3, e4])

        gs = DiGraphSet([])
        self.assertEqual(gs.remove_add_some_edges(), DiGraphSet())

        gs = DiGraphSet([g0])
        self.assertEqual(gs.remove_add_some_edges(), DiGraphSet())

        gs = DiGraphSet([g1r])
        self.assertEqual(gs.remove_add_some_edges(),
                         DiGraphSet([g1, g2, g3, g4]))

        gs = DiGraphSet([g0, g1])
        self.assertEqual(gs.remove_add_some_edges(),
                         DiGraphSet([g1r, g2, g3, g4]))

        DiGraphSet.set_universe([e1, e2, e3, e4])

        gs = DiGraphSet([g1, g12])
        self.assertEqual(gs.remove_add_some_edges(),
                         DiGraphSet([g2, g3, g4, g13, g14, g23, g24]))

        gs1 = DiGraphSet([g0, g4, g12, g234])
        gs2 = DiGraphSet([g1, g2, g3, g13, g14, g23, g24, g123, g124, g134])
        self.assertEqual(gs1.remove_add_some_edges(), gs2)

    def test_io(self):
        self.setUp()
        gs = DiGraphSet()
        st = gs.dumps()
        self.assertEqual(st, "B\n.\n")
        gs = DiGraphSet.loads(st)
        self.assertEqual(gs, DiGraphSet())

        gs = DiGraphSet([g0])
        st = gs.dumps()
        self.assertEqual(st, "T\n.\n")
        gs = DiGraphSet.loads(st)
        self.assertEqual(gs, DiGraphSet([g0]))

        v = [g0, g1, g12, g123, g1234, g134, g14, g4]
        gs = DiGraphSet(v)
        st = gs.dumps()
        gs = DiGraphSet.loads(st)
        self.assertEqual(gs, DiGraphSet(v))

        # skip this test, becasue string is treated as an element
#        gs = DiGraphSet(st)
#        self.assertEqual(gs, DiGraphSet(v))

        with tempfile.TemporaryFile() as f:
            gs.dump(f)
            f.seek(0)
            gs = DiGraphSet.load(f)
            self.assertEqual(gs, DiGraphSet(v))

    def test_directed_cycles(self):
        DiGraphSet.set_universe(universe_edges)
        gs = DiGraphSet.directed_cycles()
        self.assertEqual(len(gs), 2 * (2 + 1) + len(universe_edges) / 2)

        self.assertTrue([(2, 3), (3, 2)] in gs)
        self.assertTrue([(1, 2), (2, 3), (3, 6), (6, 5), (5, 4), (4, 1)] in gs)
        self.assertTrue([(4, 5), (5, 2), (2, 1), (1, 4)] in gs)
        self.assertTrue([(5, 4), (4, 1), (1, 2), (2, 5)] in gs)

        self.assertTrue([(1, 2), (2, 5), (5, 4), (1, 4)] not in gs)
        self.assertTrue([(1, 4), (4, 5), (5, 4), (4, 1)] not in gs)

    def test_directed_hamiltonian_cycles(self):
        DiGraphSet.set_universe(universe_edges)
        gs = DiGraphSet.directed_hamiltonian_cycles()
        cycles = DiGraphSet.directed_cycles()
        self.assertEqual(len(gs), 2)
        for c in gs:
            self.assertTrue(c in cycles)
        self.assertTrue([(1, 2), (2, 3), (3, 6), (6, 5), (5, 4), (4, 1)] in gs)
        self.assertTrue([(1, 2), (2, 3)] not in gs)

    def test_directed_st_paths(self):
        DiGraphSet.set_universe(universe_edges)
        s, t = 1, 6
        gs = DiGraphSet.directed_st_paths(s, t, False)
        self.assertTrue([(1, 4), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 4), (4, 5), (5, 2), (2, 3), (3, 6)] in gs)
        self.assertTrue([(1, 4), (4, 5)] not in gs)
        self.assertEqual(len(gs), 4)

    def test_directed_st_paths_hack1(self):
        edges = [(1, 2), (1, 3), (1, 4), (2, 4), (3, 1)]
        DiGraphSet.set_universe(edges)

        s, t = 2, 4
        gs = DiGraphSet.directed_st_paths(s, t, False)
        self.assertTrue([(2, 4)] in gs)
        self.assertEqual(len(gs), 1)

    def test_directed_st_paths_hack2(self):
        edges = [(1, 2)]
        DiGraphSet.set_universe(edges)

        s, t = 1, 2
        gs = DiGraphSet.directed_st_paths(s, t)
        self.assertEqual(len(gs), 1)

    def test_directed_st_hamiltonian_paths(self):
        DiGraphSet.set_universe(universe_edges)
        s, t = 1, 6
        s_to_t = [(1, 4), (4, 5), (5, 2), (2, 3), (3, 6)]
        t_to_s = [(6, 3), (3, 2), (2, 5), (5, 4), (4, 1)]

        gs = DiGraphSet.directed_st_paths(s, t, True)
        self.assertTrue(s_to_t in gs)
        self.assertTrue(t_to_s not in gs)
        self.assertEqual(len(gs), 1)

        gs = DiGraphSet.directed_st_paths(t, s, True)
        self.assertTrue(s_to_t not in gs)
        self.assertTrue(t_to_s in gs)
        self.assertEqual(len(gs), 1)

    def test_directed_st_hamiltonian_paths_hack1(self):
        edges = [(1, 2), (3, 4)]
        DiGraphSet.set_universe(edges)

        s, t = 1, 2
        gs = DiGraphSet.directed_st_paths(s, t, is_hamiltonian=True)
        for gg in gs:
            print(gg)
        self.assertEqual(len(gs), 0)

    def test_hamiltonian_path_in_paths(self):
        DiGraphSet.set_universe(universe_edges)
        for s in range(1, 7):
            for t in range(1, 7):
                path = DiGraphSet.directed_st_paths(s, t, False)
                hamiltonian = DiGraphSet.directed_st_paths(s, t, True)
                self.assertTrue(hamiltonian.issubset(path))

    def test_rooted_forests(self):
        DiGraphSet.set_universe(universe_edges)
        gs = DiGraphSet.rooted_forests()

        self.assertTrue([(1, 2), (2, 3), (3, 6), (6, 5), (5, 4)] in gs)
        self.assertTrue([(5, 4), (4, 1), (5, 6), (6, 3)] in gs)
        self.assertTrue([(4, 1), (4, 5), (2, 3), (3, 6)] in gs)
        self.assertTrue([(2, 1), (2, 5), (2, 3)] in gs)
        self.assertTrue([(5, 2), (6, 3)] in gs)
        self.assertTrue([(1, 4)] in gs)
        self.assertTrue([] in gs)

        self.assertTrue([(2, 1), (4, 1)] not in gs)
        self.assertTrue([(1, 2), (2, 5), (5, 2), (2, 1)] not in gs)
        self.assertTrue([(1, 2), (2, 1)] not in gs)

        roots = [1, 2, 3]
        gs = DiGraphSet.rooted_forests(roots)
        self.assertEqual(len(gs), 1)
        self.assertTrue([(1, 4), (2, 5), (3, 6)] in gs)

        roots = [1, 4]
        gs = DiGraphSet.rooted_forests(roots)
        self.assertEqual(len(gs), 6)
        self.assertTrue([(1, 2), (4, 5)] in gs)
        self.assertTrue([(1, 2), (2, 3), (3, 6), (4, 5)] in gs)
        self.assertTrue([(1, 2), (5, 4)] not in gs)
        self.assertTrue([(1, 2), (2, 5)] not in gs)

        roots = [2]
        gs = DiGraphSet.rooted_forests(roots)
        self.assertTrue([(2, 1), (1, 4), (4, 5), (5, 6), (6, 3)] in gs)
        self.assertTrue([(2, 1), (2, 3), (2, 5)] in gs)
        self.assertTrue([(3, 6), (6, 5), (5, 4)] not in gs)
        self.assertTrue([(1, 2)] not in gs)

    def test_rooted_spanning_forests(self):
        DiGraphSet.set_universe(universe_edges)
        is_spanning = True

        roots = [1, 4]
        gs = DiGraphSet.rooted_forests(roots, is_spanning)
        self.assertEqual(len(gs), 3)
        self.assertTrue([(1, 2), (4, 5), (2, 3), (5, 6)] in gs)
        self.assertTrue([(1, 2), (4, 5), (2, 3), (3, 6)] in gs)
        self.assertTrue([(1, 2), (4, 5), (6, 3), (5, 6)] in gs)

        roots = [3, 4]
        gs = DiGraphSet.rooted_forests(roots, is_spanning)
        self.assertTrue([(3, 6), (6, 5), (5, 2), (4, 1)] in gs)
        self.assertTrue([(3, 6), (6, 5), (5, 2), (1, 4)] not in gs)

    def test_rooted_spanning_trees(self):
        DiGraphSet.set_universe(universe_edges)
        root = 1
        is_spanning = True

        gs = DiGraphSet.rooted_trees(root, is_spanning)
        self.assertEqual(len(gs), 15)  # det(L)
        self.assertTrue([(1, 2), (2, 3), (1, 4), (2, 5), (3, 6)] in gs)
        self.assertTrue([(1, 2), (2, 3), (4, 1), (2, 5), (3, 6)] not in gs)
        for rooted_tree in gs:
            self.assertEqual(len(rooted_tree), 5)
            self.assertTrue((1, 2) in rooted_tree or (1, 4) in rooted_tree)

    def test_rooted_trees(self):
        DiGraphSet.set_universe(universe_edges)
        root = 1
        is_spanning = False

        gs = DiGraphSet.rooted_trees(root, is_spanning)
        gs.issubset(DiGraphSet.rooted_trees(root, True))

        self.assertEqual(len(gs), 45)

        self.assertTrue([] in gs)
        self.assertTrue([(1, 2)] in gs)
        self.assertTrue([(1, 2), (1, 4)] in gs)
        self.assertTrue([(1, 2), (1, 4), (4, 5)] in gs)
        self.assertTrue([(1, 2), (1, 4), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 2), (1, 4), (4, 5), (5, 6), (6, 3)] in gs)

        self.assertTrue([(4, 1)] not in gs)
        self.assertTrue([(2, 3)] not in gs)
        self.assertTrue([(1, 2), (2, 5), (5, 4), (4, 1)] not in gs)

    def test_rooted_spanning_forests_and_trees(self):
        DiGraphSet.set_universe(universe_edges)
        is_spanning = True

        for root in range(1, 7):
            roots = [root]
            gs = DiGraphSet.rooted_forests(roots, is_spanning)
            spanning_trees = DiGraphSet.rooted_trees(root, is_spanning)
            self.assertEqual(gs, spanning_trees)

    def test_graphs(self):
        DiGraphSet.set_universe(universe_edges)

        gs = DiGraphSet.graphs()
        self.assertEqual(len(gs), 2**len(universe_edges))

    def test_degree_constraints(self):
        DiGraphSet.set_universe(universe_edges)

        in_dc = {}
        out_dc = {}
        # cycles
        for v in DiGraphSet._vertices:
            in_dc[v] = out_dc[v] = range(1, 2)
        gs = DiGraphSet.graphs(in_degree_constraints=in_dc,
                               out_degree_constraints=out_dc)
        self.assertEqual(len(gs), 9)
        self.assertTrue([(1, 2), (2, 3), (3, 6), (6, 5), (5, 4), (4, 1)] in gs)
        self.assertTrue([(1, 4), (4, 5), (5, 6), (6, 3), (3, 2), (2, 1)] in gs)
        self.assertTrue([(1, 2), (2, 5), (5, 4), (4, 1)] not in gs)

        in_dc = {}
        out_dc = {}
        # all subgraphs
        for v in DiGraphSet._vertices:
            in_dc[v] = out_dc[v] = range(0, 4)
        gs = DiGraphSet.graphs(in_degree_constraints=in_dc,
                               out_degree_constraints=out_dc)
        self.assertEqual(len(gs), 2**len(universe_edges))

        in_dc = {}
        for v in DiGraphSet._vertices:
            in_dc[v] = range(1, 2)
        gs = DiGraphSet.graphs(in_degree_constraints=in_dc)
        self.assertEqual(len(gs), 2**4 * 3**2)

        in_dc = {}
        for v in DiGraphSet._vertices:
            in_dc[v] = range(1, 4)
        gs = DiGraphSet.graphs(in_degree_constraints=in_dc)
        self.assertEqual(len(gs), 3**4 * 7**2)

        out_dc = {}
        for v in DiGraphSet._vertices:
            out_dc[v] = range(1, 2)
        gs = DiGraphSet.graphs(out_degree_constraints=out_dc)
        self.assertEqual(len(gs), 2**4 * 3**2)

        out_dc = {}
        for v in DiGraphSet._vertices:
            out_dc[v] = range(1, 4)
        gs = DiGraphSet.graphs(out_degree_constraints=out_dc)
        self.assertEqual(len(gs), 3**4 * 7**2)

    def test_trees_in_graphs(self):
        DiGraphSet.set_universe(universe_edges)

        root = 1
        trees = DiGraphSet.rooted_trees(root, is_spanning=True)

        in_dc = {}
        out_dc = {}
        for v in range(1, len(DiGraphSet._vertices)+1):
            if v == root:
                in_dc[v] = range(0, 1)
                out_dc[v] = range(1, len(DiGraphSet._vertices))
            else:
                in_dc[v] = range(1, 2)
        gs = DiGraphSet.graphs(in_degree_constraints=in_dc,
                               out_degree_constraints=out_dc)

        self.assertTrue(trees.issubset(gs))

    def test_with_graphset(self):
        graphillion_universe = [f1, f2, f3, f4, f5, f6, f7]
        GraphSet.set_universe(graphillion_universe)
        DiGraphSet.set_universe(graphillion_universe)

        root = 1
        trees_g = GraphSet.trees(root)
        trees_dg = DiGraphSet.rooted_trees(root)

        self.assertEqual(len(trees_g), 45)
        self.assertTrue([] in trees_g)
        self.assertTrue([(1, 2), (1, 4), (2, 5), (4, 5)] not in trees_g)

        self.assertTrue([(1, 2), (1, 4), (4, 5), (5, 6), (2, 3)] in trees_dg)
        self.assertTrue([(1, 2), (2, 5), (5, 4), (4, 1)] not in trees_dg)

if __name__ == '__main__':
    unittest.main()
