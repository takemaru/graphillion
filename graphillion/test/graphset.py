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

import glob
import os
import sys
import tempfile
import unittest

sys.path = ['.'] + glob.glob(os.path.join('build', 'lib.*')) + sys.path
from graphillion import GraphSet


e1 = (1,2)
e2 = (1,3)
e3 = (2,4)
e4 = (3,4)

g0 = set()
g1 = set([e1])
g2 = set([e2])
g3 = set([e3])
g4 = set([e4])
g12 = set([e1, e2])
g13 = set([e1, e3])
g14 = set([e1, e4])
g23 = set([e2, e3])
g24 = set([e2, e4])
g34 = set([e3, e4])
g123 = set([e1, e2, e3])
g124 = set([e1, e2, e4])
g134 = set([e1, e3, e4])
g234 = set([e2, e3, e4])
g1234 = set([e1, e2, e3, e4])


class TestGraphSet(unittest.TestCase):

    def setUp(self):
        GraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                              traversal='dfs', source=1)

    def tearDown(self):
        pass

    def test_init(self):
        GraphSet.set_universe([('i', 'ii')])
        self.assertEqual(GraphSet.get_universe(), [('i', 'ii')])

        gs = GraphSet({})
        self.assertEqual(len(gs), 2**1)

        GraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                              traversal='dfs', source=1)
        self.assertEqual(GraphSet.get_universe(),
                         [e2 + (-.2,), e4 + (.4,), e1 + (.3,), e3 + (-.2,)])

        gs = GraphSet({})
        self.assertEqual(len(gs), 2**4)

    def test_constructors(self):
        gs = GraphSet()
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(len(gs), 0)

        gs = GraphSet(set([(2,1)]))
        self.assertEqual(len(gs), 1)
        self.assertTrue(g1 in gs)

        gs = GraphSet([g1, set([(3,1)])])
        self.assertEqual(len(gs), 2)
        self.assertTrue(g1 in gs)
        self.assertTrue(g2 in gs)

        gs = GraphSet({'include': [e1, e2], 'exclude': [(4,3)]})
        self.assertEqual(len(gs), 2)
        self.assertTrue(g12 in gs)
        self.assertTrue(g123 in gs)

        self.assertRaises(KeyError, GraphSet, set([(1,4)]))
        self.assertRaises(KeyError, GraphSet, [set([(1,4)])])
        self.assertRaises(KeyError, GraphSet, {'include': [(1,4)]})

        # copy constructor
        gs1 = GraphSet([g0, g12, g13])
        gs2 = gs1.copy()
        self.assertTrue(isinstance(gs2, GraphSet))
        gs1.clear()
        self.assertEqual(gs1, GraphSet())
        self.assertEqual(gs2, GraphSet([g0, g12, g13]))

        # repr for large GraphSet
        gs = GraphSet({})
        self.assertEqual(
            repr(gs),
            "GraphSet([set([]), set([(1, 3)]), set([(3, 4)]), set([(1, 2)]), set([(2, 4)] ...")

    def test_subgraphs(self):
        pass

    def test_comparison(self):
        gs = GraphSet(g12)
        self.assertEqual(gs, GraphSet(g12))
        self.assertNotEqual(gs, GraphSet(g13))

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
        self.assertTrue(gs > GraphSet([set(), g12]))
        self.assertFalse(gs > GraphSet(v))

    def test_unary_operators(self):
        gs = GraphSet([g0, g1, g12, g123, g1234, g134, g14, g4])

        self.assertTrue(isinstance(~gs, GraphSet))
        self.assertEqual(~gs, GraphSet([g124, g13, g2, g23, g234, g24, g3, g34]))

        self.assertTrue(isinstance(gs.smaller(3), GraphSet))
        self.assertEqual(gs.smaller(3), GraphSet([g0, g1, g12, g14, g4]))
        self.assertTrue(isinstance(gs.larger(3), GraphSet))
        self.assertEqual(gs.larger(3), GraphSet([g1234]))
        self.assertTrue(isinstance(gs.same_size(3), GraphSet))
        self.assertEqual(gs.same_size(3), GraphSet([g123, g134]))

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
        gs = GraphSet(u) & GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g12, g14]))
        gs = GraphSet(u).intersection(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g12, g14]))

        gs = GraphSet(u)
        gs &= GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g12, g14]))
        gs = GraphSet(u)
        gs.intersection_update(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g12, g14]))

        gs = GraphSet(u) | GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(
            gs, GraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))
        gs = GraphSet(u).union(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(
            gs, GraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))

        gs = GraphSet(u)
        gs |= GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(
            gs, GraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))
        gs = GraphSet(u)
        gs.update(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(
            gs, GraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))

        gs = GraphSet(u) - GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g4]))
        gs = GraphSet(u).difference(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g4]))

        gs = GraphSet(u)
        gs -= GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g4]))
        gs = GraphSet(u)
        gs.difference_update(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g4]))

        gs = GraphSet(u) ^ GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g23, g34, g4]))
        gs = GraphSet(u).symmetric_difference(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g23, g34, g4]))

        gs = GraphSet(u)
        gs ^= GraphSet(v)
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g123, g1234, g134, g23, g34, g4]))
        gs = GraphSet(u)
        gs.symmetric_difference_update(GraphSet(v))
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

        gs = GraphSet(u).flip(e1)
        self.assertEqual(gs, GraphSet([g0, g1, g14, g2, g23, g234, g34, g4]))

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

        v = [g12, g14, g23, g34]
        gs = GraphSet(u).subgraphs(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g0, g1, g12, g14, g4]))

        gs = GraphSet(u).supergraphs(GraphSet(v))
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(gs, GraphSet([g12, g123, g1234, g134, g14]))

#        gs = GraphSet(u).non_subgraphs(GraphSet(v))
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g123, g1234, g134]))

#        gs = GraphSet(u).non_supergraphs(GraphSet(v))
#        self.assertTrue(isinstance(gs, GraphSet))
#        self.assertEqual(gs, GraphSet([g0, g1, g4]))

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
        for s in gs1:
            gs2 = gs2 | GraphSet(s)
        self.assertEqual(gs1, GraphSet([g0, g12, g13]))
        self.assertEqual(gs1, gs2)

        gs2 = GraphSet()
        for s in gs1:
            gs2 = gs2 | GraphSet(s)
        self.assertEqual(gs1, gs2)

        gs1 = GraphSet([g0, g12, g13])
        gs2 = GraphSet()
        for s in gs1.randomize():
            gs2 = gs2 | GraphSet(s)
        self.assertEqual(gs1, gs2)

        gen = gs1.randomize()
        self.assertTrue(isinstance(gen.next(), set))

        gs = GraphSet([g0, g1, g12, g123, g1234, g134, g14, g4])
        r = []
        for s in gs.maximize():
            r.append(s)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], g14)
        self.assertEqual(r[1], g134)
        self.assertEqual(r[2], g4)

        r = []
        for s in gs.minimize():
            r.append(s)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], g123)
        self.assertEqual(r[1], g0)
        self.assertEqual(r[2], g12)

    def test_lookup(self):
        gs1 = GraphSet({}) - GraphSet([g1, g34])

        self.assertTrue(g12 in gs1)
        self.assertTrue(g1 not in gs1)

        gs2 = gs1.include((2,1))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.exclude(e2)
        self.assertEqual(len(gs2), 6)

        self.assertEqual(len(gs1.include(1)), 11)
        self.assertEqual(len(gs1.exclude(1)), 3)

    def test_modifiers(self):
        v = [g0, g12, g13]
        gs = GraphSet(v)
        gs.add(g1)
        self.assertTrue(g1 in gs)

        gs.remove(g1)
        self.assertTrue(g1 not in gs)
        self.assertRaises(KeyError, gs.remove, g1)

        gs.add(g1)
        gs.discard(g1)
        self.assertTrue(g1 not in gs)
        gs.discard(g1)  # no exception raised

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
        self.assertTrue(g not in gs)
        self.assertEqual(gs | GraphSet(g), GraphSet(v))

        self.assertTrue(gs)
        gs.clear()
        self.assertFalse(gs)

        self.assertRaises(KeyError, gs.pop)

        self.assertRaises(KeyError, gs.add, set([(1,4)]))
        self.assertRaises(KeyError, gs.remove, set([(1,4)]))
        self.assertRaises(KeyError, gs.discard, set([(1,4)]))

        self.assertRaises(KeyError, gs.add, (1,4))
        self.assertRaises(KeyError, gs.remove, (1,4))
        self.assertRaises(KeyError, gs.discard, (1,4))

    def test_io(self):
        gs = GraphSet()
        st = gs.dumps()
        self.assertEqual(st, "B\n.\n")
        gs.loads(st)
        self.assertEqual(gs, GraphSet())

        gs = GraphSet(g0)
        st = gs.dumps()
        self.assertEqual(st, "T\n.\n")
        gs.loads(st)
        self.assertEqual(gs, GraphSet(g0))

        v = [g0, g1, g12, g123, g1234, g134, g14, g4]
        gs = GraphSet(v)
        st = gs.dumps()
        gs = GraphSet()
        gs.loads(st)
        self.assertEqual(gs, GraphSet(v))

        # skip this test, becasue string is treated as an element
#        gs = GraphSet(st)
#        self.assertEqual(gs, GraphSet(v))

        f = tempfile.TemporaryFile()
        gs.dump(f)
        gs = GraphSet()
        f.seek(0)
        gs.load(f)
        self.assertEqual(gs, GraphSet(v))

    def test_large(self):
        import networkx as nx

        g = nx.grid_2d_graph(11, 11)
        v00, v01, v10 = (0,0), (0,1), (1,0)

        GraphSet.set_universe(g.edges())
        self.assertEqual(len(GraphSet.get_universe()), 220)
        self.assertEqual(GraphSet.get_universe()[:2], [(v01, v00), (v10, v00)])

        gs = GraphSet({});
        gs -= GraphSet([set([(v01, v00)]), set([(v01, v00), (v10, v00)])])
        self.assertAlmostEqual(gs.len() / (2**220 - 2), 1)

        i = 0
        for s in gs:
            if i > 50: break
            i += 1

        del nx


if __name__ == '__main__':
    unittest.main()
