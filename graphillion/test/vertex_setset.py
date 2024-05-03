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

from graphillion import GraphSet, setset, VertexSetSet
import tempfile
import unittest


e1 = (1,2)
e2 = (1,3)
e3 = (2,4)
e4 = (3,4)
e5 = (5,6)
e6 = (6,7)

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


class TestVertexSetSet(unittest.TestCase):

    def setUp(self):
        GraphSet.set_universe([e1 + (.1,), e2 + (-.2,), e3 + (-.3,), e4 + (.4,), e5])
        VertexSetSet.set_universe([(1, .3), (2, -.2), (3, -.3), (4, .4)])

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
            "VertexSetSet([[], [1, 2], [1, 3]])")

        vss = VertexSetSet({})
        self.assertEqual(
            repr(vss),
            "VertexSetSet([[], [1], [2], [3], [4], [1, 2], [1, 3], [2, 3], [1, 4], [2, 4] ...")

    def test_show_messages(self):
        a = VertexSetSet.show_messages()
        b = VertexSetSet.show_messages(True)
        self.assertTrue(b)
        c = VertexSetSet.show_messages(False)
        self.assertTrue(c)
        d = VertexSetSet.show_messages(a)
        self.assertFalse(d)

    def test_comparison(self):
        vss = VertexSetSet([vs12])
        self.assertEqual(vss, VertexSetSet([vs12]))
        self.assertNotEqual(vss, VertexSetSet([vs13]))

        # __nonzero__
        self.assertTrue(vss)
        self.assertFalse(VertexSetSet())

        v = [vs0, vs12, vs13]
        vss = VertexSetSet(v)
        self.assertTrue(vss.isdisjoint(VertexSetSet([vs1, vs123])))
        self.assertFalse(vss.isdisjoint(VertexSetSet([vs1, vs12])))

        self.assertTrue(vss.issubset(VertexSetSet(v)))
        self.assertFalse(vss.issubset(VertexSetSet([vs0, vs12])))
        self.assertTrue(vss <= VertexSetSet(v))
        self.assertFalse(vss <= VertexSetSet([vs0, vs12]))
        self.assertTrue(vss < VertexSetSet([vs0, vs1, vs12, vs13]))
        self.assertFalse(vss < VertexSetSet(v))

        self.assertTrue(vss.issuperset(VertexSetSet(v)))
        self.assertFalse(vss.issuperset(VertexSetSet([vs1, vs12])))
        self.assertTrue(vss >= VertexSetSet(v))
        self.assertFalse(vss >= VertexSetSet([vs1, vs12]))
        self.assertTrue(vss > VertexSetSet([[], vs12]))
        self.assertFalse(vss > VertexSetSet(v))

#     def test_unary_operators(self):
        vss = VertexSetSet([vs0, vs1, vs12, vs123, vs1234, vs134, vs14, vs4])

        self.assertTrue(isinstance(~vss, VertexSetSet))
        self.assertEqual(~vss, VertexSetSet([vs124, vs13, vs2, vs23, vs234, vs24, vs3, vs34]))

        self.assertTrue(isinstance(vss.smaller(3), VertexSetSet))
        self.assertEqual(vss.smaller(3), VertexSetSet([vs0, vs1, vs12, vs14, vs4]))
        self.assertTrue(isinstance(vss.larger(3), VertexSetSet))
        self.assertEqual(vss.larger(3), VertexSetSet([vs1234]))
        self.assertTrue(isinstance(vss.graph_size(3), VertexSetSet))
        self.assertEqual(vss.graph_size(3), VertexSetSet([vs123, vs134]))
        self.assertTrue(isinstance(vss.len(3), VertexSetSet))
        self.assertEqual(vss.len(3), VertexSetSet([vs123, vs134]))

        vss = VertexSetSet([vs12, vs123, vs234])
        self.assertTrue(isinstance(vss.minimal(), VertexSetSet))
        self.assertEqual(vss.minimal(), VertexSetSet([vs12, vs234]))
        self.assertTrue(isinstance(vss.maximal(), VertexSetSet))
        self.assertEqual(vss.maximal(), VertexSetSet([vs123, vs234]))

        vss = VertexSetSet([vs12, vs14, vs23, vs34])
        self.assertTrue(isinstance(vss.blocking(), VertexSetSet))
        self.assertEqual(
            vss.blocking(), VertexSetSet([vs123, vs1234, vs124, vs13, vs134, vs234, vs24]))

    def test_binary_operators(self):
        u = [vs0, vs1, vs12, vs123, vs1234, vs134, vs14, vs4]
        v = [vs12, vs14, vs23, vs34]

        vss = VertexSetSet(u) | VertexSetSet(v)
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(
            vss, VertexSetSet([vs0, vs1, vs12, vs123, vs1234, vs134, vs14, vs23, vs34, vs4]))
        vss = VertexSetSet(u).union(VertexSetSet(u), VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(
            vss, VertexSetSet([vs0, vs1, vs12, vs123, vs1234, vs134, vs14, vs23, vs34, vs4]))

        vss = VertexSetSet(u)
        vss |= VertexSetSet(v)
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(
            vss, VertexSetSet([vs0, vs1, vs12, vs123, vs1234, vs134, vs14, vs23, vs34, vs4]))
        vss = VertexSetSet(u)
        vss.update(VertexSetSet(u), VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(
            vss, VertexSetSet([vs0, vs1, vs12, vs123, vs1234, vs134, vs14, vs23, vs34, vs4]))

        vss = VertexSetSet(u) & VertexSetSet(v)
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs12, vs14]))
        vss = VertexSetSet(u).intersection(VertexSetSet(u), VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs12, vs14]))

        vss = VertexSetSet(u)
        vss &= VertexSetSet(v)
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs12, vs14]))
        vss = VertexSetSet(u)
        vss.intersection_update(VertexSetSet(u), VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs12, vs14]))

        vss = VertexSetSet(u) - VertexSetSet(v)
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs123, vs1234, vs134, vs4]))
        vss = VertexSetSet(u).difference(VertexSetSet(), VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs123, vs1234, vs134, vs4]))

        vss = VertexSetSet(u)
        vss -= VertexSetSet(v)
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs123, vs1234, vs134, vs4]))
        vss = VertexSetSet(u)
        vss.difference_update(VertexSetSet(), VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs123, vs1234, vs134, vs4]))

        vss = VertexSetSet(u) ^ VertexSetSet(v)
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs123, vs1234, vs134, vs23, vs34, vs4]))
        vss = VertexSetSet(u).symmetric_difference(VertexSetSet(), VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs123, vs1234, vs134, vs23, vs34, vs4]))

        vss = VertexSetSet(u)
        vss ^= VertexSetSet(v)
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs123, vs1234, vs134, vs23, vs34, vs4]))
        vss = VertexSetSet(u)
        vss.symmetric_difference_update(VertexSetSet(), VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs123, vs1234, vs134, vs23, vs34, vs4]))

        v = [vs12]
        vss = VertexSetSet(u) / VertexSetSet(v)
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs3, vs34]))
        vss = VertexSetSet(u).quotient(VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs3, vs34]))

        vss = VertexSetSet(u)
        vss /= VertexSetSet(v)
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs3, vs34]))
        vss = VertexSetSet(u)
        vss.quotient_update(VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs3, vs34]))

        vss = VertexSetSet(u) % VertexSetSet(v)
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs134, vs14, vs4]))
        vss = VertexSetSet(u).remainder(VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs134, vs14, vs4]))

        vss = VertexSetSet(u)
        vss %= VertexSetSet(v)
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs134, vs14, vs4]))
        vss = VertexSetSet(u)
        vss.remainder_update(VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs134, vs14, vs4]))

        vss = VertexSetSet(u).complement()
        self.assertEqual(vss, VertexSetSet([vs0, vs123, vs1234, vs2, vs23, vs234, vs34, vs4]))

        v = [vs12, vs14, vs23, vs34]
        vss = VertexSetSet(u).join(VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(
            vss, VertexSetSet([vs12, vs123, vs124, vs1234, vs134, vs14, vs23, vs234, vs34]))

        vss = VertexSetSet(u).meet(VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs12, vs14, vs2, vs23, vs3, vs34, vs4]))

        v = [vs12, vs14, vs23, vs34]
        vss = VertexSetSet(u).subgraphs(VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs12, vs14, vs4]))

        vss = VertexSetSet(u).supergraphs(VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs12, vs123, vs1234, vs134, vs14]))

        vss = VertexSetSet(u).non_subgraphs(VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs123, vs1234, vs134]))

        vss = VertexSetSet(u).non_supergraphs(VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs4]))

        vss1 = VertexSetSet({}) - VertexSetSet([vs1, vs34])

        vss2 = vss1.including(VertexSetSet([vs1, vs2]))
        self.assertTrue(isinstance(vss2, VertexSetSet))
        self.assertEqual(len(vss2), 11)

        vss2 = vss1.including(vs1)
        self.assertTrue(isinstance(vss2, VertexSetSet))
        self.assertEqual(len(vss2), 7)

        self.assertRaises(KeyError, vss1.including, 5)

        vss2 = vss1.excluding(VertexSetSet([vs1, vs2]))
        self.assertTrue(isinstance(vss2, VertexSetSet))
        self.assertEqual(len(vss2), 3)

        vss2 = vss1.excluding(vs1)
        self.assertTrue(isinstance(vss2, VertexSetSet))
        self.assertEqual(len(vss2), 7)

        vss2 = vss1.excluding(v2)
        self.assertTrue(isinstance(vss2, VertexSetSet))
        self.assertEqual(len(vss2), 6)

        self.assertRaises(KeyError, vss1.excluding, 5)

        v = [vs12, vs14, vs23, vs34]
        vss = VertexSetSet(u).included(VertexSetSet(v))
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs12, vs14, vs4]))

        vss = VertexSetSet(u).included(vs12)
        self.assertTrue(isinstance(vss, VertexSetSet))
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs12]))

    def test_capacity(self):
        vss = VertexSetSet()
        self.assertFalse(vss)

        vss = VertexSetSet([vs0, vs12, vs13])
        self.assertTrue(vss)

        self.assertEqual(len(vss), 3)
        self.assertEqual(vss.len(), 3)

    def test_iterators(self):
        vss1 = VertexSetSet([vs0, vs12, vs13])
        vss2 = VertexSetSet()
        for g in vss1:
            self.assertTrue(isinstance(g, list))
            vss2 = vss2 | VertexSetSet([g])
        self.assertEqual(vss1, VertexSetSet([vs0, vs12, vs13]))
        self.assertEqual(vss1, vss2)

        vss2 = VertexSetSet()
        for g in vss1:
            self.assertTrue(isinstance(g, list))
            vss2 = vss2 | VertexSetSet([g])
        self.assertEqual(vss1, vss2)

        vss1 = VertexSetSet([vs0, vs12, vs13])
        vss2 = VertexSetSet()
        for g in vss1.rand_iter():
            self.assertTrue(isinstance(g, list))
            vss2 = vss2 | VertexSetSet([g])
        self.assertEqual(vss1, vss2)

        gen = vss1.rand_iter()
        self.assertTrue(isinstance(next(gen), list))

        # copy from setUp()
        # VertexSetSet.set_universe([(1, .3), (2, -.2), (3, -.3), (4, .4)])
        vss = VertexSetSet([vs0, vs1, vs12, vs123, vs1234, vs134, vs14, vs4])
        r = []
        for g in vss.max_iter():
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], vs14)
        self.assertEqual(r[1], vs4)
        self.assertEqual(r[2], vs134)

        r = []
        for g in vss.max_iter({v1: -.3, v2: .2, v3: .2, v4: -.4}):
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], vs123)
        self.assertEqual(r[1], vs0)
        self.assertEqual(r[2], vs12)

        r = []
        for g in vss.min_iter():
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], vs123)
        self.assertEqual(r[1], vs0)
        self.assertEqual(r[2], vs12)

        r = []
        for g in vss.min_iter({v1: -.3, v2: .2, v3: .2, v4: -.4}):
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], vs14)
        self.assertEqual(r[1], vs134)
        self.assertEqual(r[2], vs4)

        vss = VertexSetSet([[]])
        self.assertEqual(list(vss.min_iter()), [[]])

    def test_lookup(self):
        vss1 = VertexSetSet([vs1, vs12])

        self.assertTrue(vs12 in vss1)
        self.assertTrue(vs2 not in vss1)
        self.assertTrue(v1 in vss1)
        self.assertTrue(v4 not in vss1)

    def test_modifiers(self):
        v = [vs0, vs12, vs13]
        vss = VertexSetSet(v)
        vss.add(vs1)
        self.assertTrue(vs1 in vss)

        vss.remove(vs1)
        self.assertTrue(vs1 not in vss)
        self.assertRaises(KeyError, vss.remove, vs1)

        vss.add(vs0)
        vss.discard(vs0)
        self.assertTrue(vs0 not in vss)
        vss.discard(vs0)  # no exception raised

        vss = VertexSetSet(v)
        vss.add(v2)
        self.assertEqual(vss, VertexSetSet([vs12, vs123, vs2]))

        vss = VertexSetSet(v)
        vss.remove(v2)
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs13]))
        self.assertRaises(KeyError, vss.remove, v4)

        vss = VertexSetSet(v)
        vss.discard(v2)
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs13]))
        vss.discard(v4)  # no exception raised

        v = [vs1, vs12, vs13]
        vss = VertexSetSet(v)
        g = vss.pop()
        self.assertTrue(isinstance(g, list))
        self.assertTrue(g not in vss)
        self.assertEqual(vss | VertexSetSet([g]), VertexSetSet(v))

        self.assertTrue(vss)
        vss.clear()
        self.assertFalse(vss)

        self.assertRaises(KeyError, vss.pop)

        self.assertRaises(KeyError, vss.add, [5])
        self.assertRaises(KeyError, vss.remove, [5])
        self.assertRaises(KeyError, vss.discard, [5])

        self.assertRaises(KeyError, vss.add, 5)
        self.assertRaises(KeyError, vss.remove, 5)
        self.assertRaises(KeyError, vss.discard, 5)

        u = [vs0, vs1, vs12, vs123, vs1234, vs134, vs14, vs4]
        vss = VertexSetSet(u)
        vss.flip(v1)
        self.assertEqual(vss, VertexSetSet([vs0, vs1, vs14, vs2, vs23, vs234, vs34, vs4]))

    def test_probability(self):
        p = {v1: .9, v2: .8, v3: .7, v4: .6}

        vss = VertexSetSet()
        self.assertEqual(vss.probability(p), 0)

        vss = VertexSetSet([vs0])
        self.assertAlmostEqual(vss.probability(p), .0024)

        vss = VertexSetSet([vs1])
        self.assertAlmostEqual(vss.probability(p), .0216)

        vss = VertexSetSet([vs2])
        self.assertAlmostEqual(vss.probability(p), .0096)

        vss = VertexSetSet([vs12, vs13])
        self.assertAlmostEqual(vss.probability(p), .1368)

        vss = VertexSetSet([vs1234])
        self.assertAlmostEqual(vss.probability(p), .3024)

        vss = VertexSetSet([vs0, vs1, vs2, vs12, vs13, vs1234])
        self.assertAlmostEqual(vss.probability(p), .4728)

    def test_cost_le(self):
        VertexSetSet.set_universe([v1, v2, v3, v4])
        vss = VertexSetSet([vs0, vs1, vs2, vs3, vs4, vs12, vs14, vs134, vs234, vs1234])

        costs = {v1: 2, v2: 14, v3: 4, v4: 7}
        cost_bound = 13

        small_cost_vss = vss.cost_le(costs, cost_bound)
        self.assertIn(vs0, small_cost_vss) # cost: 0
        self.assertIn(vs1, small_cost_vss) # cost: 2
        self.assertNotIn(vs2, small_cost_vss) # cost: 14
        self.assertIn(vs3, small_cost_vss) # cost: 4
        self.assertIn(vs4, small_cost_vss) # cost: 7
        self.assertNotIn(vs12, small_cost_vss) # cost: 16
        self.assertIn(vs14, small_cost_vss) # cost: 9
        self.assertIn(vs134, small_cost_vss) # cost: 13
        self.assertNotIn(vs234, small_cost_vss) # cost: 25
        self.assertNotIn(vs1234, small_cost_vss) # cost: 27

    def test_cost_ge(self):
        VertexSetSet.set_universe([v1, v2, v3, v4])
        vss = VertexSetSet([vs0, vs1, vs2, vs3, vs4, vs12, vs14, vs134, vs234, vs1234])

        costs = {v1: 2, v2: 14, v3: 4, v4: 7}
        cost_bound = 13

        large_cost_vss = vss.cost_ge(costs, cost_bound)
        self.assertNotIn(vs0, large_cost_vss) # cost: 0
        self.assertNotIn(vs1, large_cost_vss) # cost: 2
        self.assertIn(vs2, large_cost_vss) # cost: 14
        self.assertNotIn(vs3, large_cost_vss) # cost: 4
        self.assertNotIn(vs4, large_cost_vss) # cost: 7
        self.assertIn(vs12, large_cost_vss) # cost: 16
        self.assertNotIn(vs14, large_cost_vss) # cost: 9
        self.assertIn(vs134, large_cost_vss) # cost: 13
        self.assertIn(vs234, large_cost_vss) # cost: 25
        self.assertIn(vs1234, large_cost_vss) # cost: 27

    def test_cost_eq(self):
        VertexSetSet.set_universe([v1, v2, v3, v4])
        vss = VertexSetSet([vs0, vs1, vs2, vs3, vs4, vs12, vs14, vs134, vs234, vs1234])

        costs = {v1: 2, v2: 14, v3: 4, v4: 7}
        cost = 13

        equal_cost_vss = vss.cost_eq(costs, cost)
        self.assertNotIn(vs0, equal_cost_vss) # cost: 0
        self.assertNotIn(vs1, equal_cost_vss) # cost: 2
        self.assertNotIn(vs2, equal_cost_vss) # cost: 14
        self.assertNotIn(vs3, equal_cost_vss) # cost: 4
        self.assertNotIn(vs4, equal_cost_vss) # cost: 7
        self.assertNotIn(vs12, equal_cost_vss) # cost: 16
        self.assertNotIn(vs14, equal_cost_vss) # cost: 9
        self.assertIn(vs134, equal_cost_vss) # cost: 13
        self.assertNotIn(vs234, equal_cost_vss) # cost: 25
        self.assertNotIn(vs1234, equal_cost_vss) # cost: 27

    def test_io(self):
        vss = VertexSetSet()
        st = vss.dumps()
        self.assertEqual(st, "B\n.\n")
        vss = VertexSetSet.loads(st)
        self.assertEqual(vss, VertexSetSet())

        vss = VertexSetSet([vs0])
        st = vss.dumps()
        self.assertEqual(st, "T\n.\n")
        vss = VertexSetSet.loads(st)
        self.assertEqual(vss, VertexSetSet([vs0]))

        v = [vs0, vs1, vs12, vs123, vs1234, vs134, vs14, vs4]
        vss = VertexSetSet(v)
        st = vss.dumps()
        vss = VertexSetSet.loads(st)
        self.assertEqual(vss, VertexSetSet(v))

        with tempfile.TemporaryFile() as f:
            vss.dump(f)
            f.seek(0)
            vss = VertexSetSet.load(f)
            self.assertEqual(vss, VertexSetSet(v))

    def test_independent_sets(self):
        GraphSet.set_universe([(1, 2), (1, 3), (1, 4),
                               (1, 5), (1, 6), (1, 7)])
        VertexSetSet.set_universe([1, 2, 3, 4, 5])
        edges = [(1, 2), (2, 3), (3, 4), (4, 5), (5, 1), (1, 4)]
        vss = VertexSetSet.independent_sets(edges)

        self.assertEqual(len(vss), 10)
        self.assertTrue([1] in vss)
        self.assertTrue([2, 5] in vss)
        self.assertTrue([1, 4] not in vss)

        VertexSetSet.set_universe([1, 2, 3, 4, 5, 6])
        edges = [(1, 2), (2, 3), (3, 4), (4, 5), (5, 6), (6, 1), (1, 4)]
        vss = VertexSetSet.independent_sets(edges, 2)

        self.assertEqual(len(vss), 9)
        self.assertTrue([1] in vss)
        self.assertTrue([2, 5] in vss)
        self.assertTrue([1, 5] not in vss)

    def test_dominating_sets(self):
        GraphSet.set_universe([(1, 2), (1, 3), (1, 4),
                               (1, 5), (1, 6), (1, 7)])
        VertexSetSet.set_universe([1, 2, 3, 4, 5, 6])
        edges = [(1, 2), (2, 3), (3, 4), (4, 5), (5, 6), (6, 1), (1, 4)]
        vss = VertexSetSet.dominating_sets(edges)

        self.assertEqual(len(vss), 41)
        self.assertTrue([1, 4] in vss)
        self.assertTrue([2, 5] in vss)
        self.assertTrue([1, 6] not in vss)

        VertexSetSet.set_universe([1, 2, 3, 4, 5, 6])
        edges = [(1, 2), (2, 3), (3, 4), (4, 5), (5, 6), (6, 1), (1, 4)]
        vss = VertexSetSet.dominating_sets(edges, 2)
        self.assertEqual(len(vss), 59)
        self.assertTrue([1] in vss)
        self.assertTrue([2, 5] in vss.minimal())
        self.assertTrue([2, 4, 6] not in vss.minimal())

    def test_vertex_covers(self):
        GraphSet.set_universe([(1, 2), (1, 3), (1, 4),
                               (1, 5), (1, 6)])
        VertexSetSet.set_universe([1, 2, 3, 4, 5])
        edges = [(1, 2), (2, 3), (3, 4), (4, 1), (1, 3), (3, 5)]
        vss = VertexSetSet.vertex_covers(edges)

        self.assertEqual(len(vss), 11)
        self.assertTrue([1, 3] in vss)
        self.assertTrue([2, 3, 4] in vss.minimal())
        self.assertTrue([2, 3, 5] not in vss)

    def test_cliques(self):
        GraphSet.set_universe([(1, 2), (1, 3), (1, 4), (1, 5), (1, 6)])
        VertexSetSet.set_universe([1, 2, 3, 4, 5])
        edges = [(1, 2), (1, 3), (1, 4), (1, 5),
                 (2, 3), (2, 4), (2, 5), (4, 5)]
        vss = VertexSetSet.cliques(edges)

        self.assertEqual(len(vss), 20)
        self.assertEqual(vss.maximal(), VertexSetSet([[1, 2, 3], [1, 2, 4, 5]]))

    def test_remove_some_vertex(self):
        GraphSet.set_universe([(1, 2), (1, 3), (1, 4), (2, 3), (2, 4)])
        VertexSetSet.set_universe()

        vss = VertexSetSet([])
        self.assertEqual(vss.remove_some_vertex(), VertexSetSet())

        vss = VertexSetSet([vs0])
        self.assertEqual(vss.remove_some_vertex(), VertexSetSet())

        vss = VertexSetSet([vs1])
        self.assertEqual(vss.remove_some_vertex(), VertexSetSet([vs0]))

        vss = VertexSetSet([vs0, vs1])
        self.assertEqual(vss.remove_some_vertex(), VertexSetSet([vs0]))

        vss = VertexSetSet([vs1, vs12])
        self.assertEqual(vss.remove_some_vertex(), VertexSetSet([vs0, vs1, vs2]))

        vss1 = VertexSetSet([vs0, vs4, vs12, vs234])
        vss2 = VertexSetSet([vs0, vs1, vs2, vs23, vs24, vs34])
        self.assertEqual(vss1.remove_some_vertex(), vss2)

    def test_add_some_vertex(self):
        GraphSet.set_universe([(1, 2), (1, 3), (1, 4), (2, 3), (2, 4)])
        VertexSetSet.set_universe()

        vss = VertexSetSet([])
        self.assertEqual(vss.add_some_vertex(), VertexSetSet())

        vss = VertexSetSet([vs0])
        vss1 = vss.add_some_vertex()
        self.assertEqual(vss.add_some_vertex(), VertexSetSet([vs1, vs2, vs3, vs4]))

        vss = VertexSetSet([vs1])
        self.assertEqual(vss.add_some_vertex(), VertexSetSet([vs12, vs13, vs14]))

        vss = VertexSetSet([vs0, vs1])
        self.assertEqual(vss.add_some_vertex(),
                         VertexSetSet([vs1, vs2, vs3, vs4, vs12, vs13, vs14]))

        vss = VertexSetSet([vs1, vs12])
        self.assertEqual(vss.add_some_vertex(),
                         VertexSetSet([vs12, vs13, vs14, vs123, vs124]))

        vss1 = VertexSetSet([vs0, vs4, vs12, vs234])
        vss2 = VertexSetSet([vs1, vs2, vs3, vs4, vs14, vs24, vs34, vs123, vs124, vs1234])
        self.assertEqual(vss1.add_some_vertex(), vss2)

    def test_remove_add_some_vertices(self):
        GraphSet.set_universe([(1, 2), (1, 3), (1, 4), (2, 3), (2, 4)])
        VertexSetSet.set_universe()

        vss = VertexSetSet([])
        self.assertEqual(vss.remove_add_some_vertices(), VertexSetSet())

        vss = VertexSetSet([vs0])
        self.assertEqual(vss.remove_add_some_vertices(), VertexSetSet())

        vss = VertexSetSet([vs1])
        self.assertEqual(vss.remove_add_some_vertices(),
                         VertexSetSet([vs2, vs3, vs4]))

        vss = VertexSetSet([vs0, vs1])
        self.assertEqual(vss.remove_add_some_vertices(),
                         VertexSetSet([vs2, vs3, vs4]))

        vss = VertexSetSet([vs1, vs12])
        self.assertEqual(vss.remove_add_some_vertices(),
                         VertexSetSet([vs2, vs3, vs4, vs13, vs14, vs23, vs24]))

        vss1 = VertexSetSet([vs0, vs4, vs12, vs234])
        vss2 = VertexSetSet([vs1, vs2, vs3, vs13, vs14, vs23, vs24, vs123, vs124, vs134])
        self.assertEqual(vss1.remove_add_some_vertices(), vss2)

    def test_to_vertexsetset(self):
        e1 = (1, 2)
        e2 = (3, 4)
        e3 = (2, 3)
        e4 = (2, 4)
        e5 = (2, 5)
        GraphSet.set_universe([e1, e2, e3, e4, e5], "as-is")
        VertexSetSet.set_universe()
        self.assertEqual(VertexSetSet.universe(), [1, 3, 4, 2, 5])

        vss1 = GraphSet([]).to_vertexsetset() # empty graphset
        self.assertEqual(vss1, VertexSetSet([]))
        # graphset consisting only of the empty graph
        vss2 = GraphSet([[]]).to_vertexsetset()
        self.assertEqual(vss2, VertexSetSet([[]]))

        g1 = [e2]
        vss3 = GraphSet([g1]).to_vertexsetset()
        self.assertEqual(vss3, VertexSetSet([[3, 4]]))
        g2 = [e1, e2]
        vss4 = GraphSet([g1, g2]).to_vertexsetset()
        self.assertEqual(vss4, VertexSetSet([[3, 4], [1, 2, 3, 4]]))
        g3 = [e1, e2, e4]
        vss5 = GraphSet([g1, g2, g3]).to_vertexsetset()
        self.assertEqual(vss5, VertexSetSet([[3, 4], [1, 2, 3, 4]]))
        g4 = [e2, e3]
        vss6 = GraphSet([g4]).to_vertexsetset()
        self.assertEqual(vss6, VertexSetSet([[2, 3, 4]]))

#     skip tests below because networkx cannot be used with VertexSetSet class now
#     def test_networkx(self):
#         try:
#             import networkx as nx
#         except ImportError:
#             return

#         try:
#             if nx.__version__[0] == "1": # for NetworkX version 1.x
#                 VertexSetSet.converters['to_graph'] = nx.Graph
#                 VertexSetSet.converters['to_edges'] = nx.Graph.edges
#             else: # for NetworkX version 2.x
#                 VertexSetSet.converters['to_graph'] = nx.from_edgelist
#                 VertexSetSet.converters['to_edges'] = nx.to_edgelist

#             g = nx.grid_2d_graph(3, 3)
#             VertexSetSet.set_universe(g)
#             g = VertexSetSet.universe()
#             self.assertTrue(isinstance(g, nx.Graph))
#             self.assertEqual(len(g.edges()), 12)

#             v00, v01, v10 = (0,0), (0,1), (1,0)
#             e1, e2 = (v00, v01), (v00, v10)
#             vss = VertexSetSet([nx.Graph([e1])])
#             self.assertEqual(len(vss), 1)
#             g = vss.pop()
#             self.assertEqual(len(vss), 0)
#             self.assertTrue(isinstance(g, nx.Graph))
#             self.assertTrue(list(g.edges()) == [(v00, v01)] or list(g.edges()) == [(v01, v00)])
#             vss.add(nx.Graph([e2]))
#             self.assertEqual(len(vss), 1)
#         except:
#             raise
#         finally:
#             VertexSetSet.converters['to_graph'] = lambda edges: edges
#             VertexSetSet.converters['to_edges'] = lambda graph: graph

#     def test_large(self):
#         try:
#             import networkx as nx
#         except ImportError:
#             return

#         try:
#             if nx.__version__[0] == "1": # for NetworkX version 1.x
#                 VertexSetSet.converters['to_graph'] = nx.Graph
#                 VertexSetSet.converters['to_edges'] = nx.Graph.edges
#             else: # for NetworkX version 2.x
#                 VertexSetSet.converters['to_graph'] = nx.from_edgelist
#                 VertexSetSet.converters['to_edges'] = nx.to_edgelist

#             g = nx.grid_2d_graph(8, 8)
#             v00, v01, v10 = (0,0), (0,1), (1,0)

#             VertexSetSet.set_universe(g, traversal='bfs')
#             self.assertEqual(len(VertexSetSet.universe().edges()), 112)
# #            self.assertEqual(VertexSetSet.universe().edges()[:2], [(v00, v01), (v00, v10)])

#             vss = VertexSetSet({});
#             vss -= VertexSetSet([nx.Graph([(v00, v01)]),
#                             nx.Graph([(v00, v01), (v00, v10)])])
#             self.assertEqual(vss.len(), 5192296858534827628530496329220094)

#             i = 0
#             for g in vss:
#                 if i > 100: break
#                 i += 1

#             paths = VertexSetSet.paths((0, 0), (7, 7))
#             self.assertEqual(len(paths), 789360053252)
#         except:
#             raise
#         finally:
#             VertexSetSet.converters['to_graph'] = lambda edges: edges
#             VertexSetSet.converters['to_edges'] = lambda graph: graph


if __name__ == '__main__':
    unittest.main()
