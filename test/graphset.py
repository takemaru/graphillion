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
import unittest


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

    def test_subgraphs(self):
        pass

    def test_binary_operators(self):
        gs = GraphSet([g1, g34])
        gs = gs.flip((4, 2))
        self.assertEqual(gs, GraphSet([g13, g4]))

        gs = GraphSet([g1, g34])
        gs = gs.complement()
        self.assertEqual(gs, GraphSet([g12, g234]))

    def test_iterators(self):
        gs = GraphSet({})
        r = []
        for s in gs.maximize():
            r.append(s)
        self.assertEqual(len(r), 16)
        self.assertEqual(r[0], set([(1, 2), (3, 4)]))
        self.assertEqual(r[-1], set([(1, 3), (2, 4)]))

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
        gs = GraphSet({}) - GraphSet([g1, g34])

        gs.add(g1)
        self.assertEqual(len(gs), 15)

        gs.remove(g1)
        self.assertEqual(len(gs), 14)

        gs.discard(g1)
        self.assertEqual(len(gs), 14)

        self.assertRaises(KeyError, gs.add, set([(1,4)]))
        self.assertRaises(KeyError, gs.remove, set([(1,4)]))
        self.assertRaises(KeyError, gs.discard, set([(1,4)]))

        gs = GraphSet([g1, g4])
        gs.add(e4)
        self.assertEqual(gs, GraphSet([g4, g14]))

        gs = GraphSet([g1, g2, g14])
        gs.remove(e4)
        self.assertEqual(gs, GraphSet([g1, g2]))

        self.assertRaises(KeyError, gs.add, (1,4))
        self.assertRaises(KeyError, gs.remove, (1,4))

    def test_large(self):
        import networkx as nx

        g = nx.grid_2d_graph(11, 11)
        GraphSet.set_universe(g.edges())
        self.assertEqual(len(GraphSet.get_universe()), 220)
        self.assertEqual(GraphSet.get_universe()[:2],
                         [((0, 1), (0, 0)), ((1, 0), (0, 0))])

        gs = GraphSet({});
        gs -= GraphSet([set([((0, 1), (0, 0))]),
                        set([((0, 1), (0, 0)), ((1, 0), (0, 0))])])
        self.assertAlmostEqual(gs.len() / (2**220 - 2), 1)

        i = 0
        for s in gs:
            if i > 50: break
            i += 1

        del nx


if __name__ == '__main__':
    unittest.main()
