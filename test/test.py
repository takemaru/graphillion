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

from graphillion import setset, GraphSet
import tempfile
import unittest

class TestSetset(unittest.TestCase):

    def setUp(self):
        setset.set_universe(['1', '2', '3', '4'])

    def tearDown(self):
        pass

    def test_init(self):
        setset.set_universe([])

        self.assertEqual(setset._obj2int, {})
        self.assertEqual(setset._int2obj, [None])
        self.assertEqual(setset.get_universe(), [])

        setset.set_universe(['i', 'ii'])
        self.assertEqual(setset._obj2int, {'i': 1, 'ii': 2})
        self.assertEqual(setset._int2obj, [None, 'i', 'ii'])
        self.assertEqual(setset.get_universe(), ['i', 'ii'])

        ss = setset({})
        self.assertEqual(
            ss,
            setset([set(), set(['i']), set(['i', 'ii']), set(['ii'])]))

        setset.set_universe(['1'])
        self.assertEqual(setset._obj2int, {'1': 1})
        self.assertEqual(setset._int2obj, [None, '1'])
        self.assertEqual(setset.get_universe(), ['1'])

        ss = setset({})
        self.assertEqual(ss, setset([set(), set(['1'])]))

    def test_constructors(self):
        ss = setset()
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(repr(ss), 'setset([])')

        ss = setset(set())
        self.assertEqual(repr(ss), 'setset([set([])])')

        ss = setset(frozenset(['1', '2']))
        self.assertEqual(repr(ss), "setset([set(['1', '2'])])")

        ss = setset([set(), set(['1', '2']), set(['1', '3'])])
        self.assertEqual(repr(ss),
                         "setset([set([]), set(['1', '2']), set(['1', '3'])])")

        ss = setset({'include': ('1', '2'), 'exclude': ('4',)})
        self.assertEqual(repr(ss),
                         "setset([set(['1', '2']), set(['1', '3', '2'])])")

        # copy constructor
        ss = setset([set(), set(['1', '2']), set(['1', '3'])])
        self.assertEqual(repr(ss),
                         "setset([set([]), set(['1', '2']), set(['1', '3'])])")

        ss1 = setset([set(), set(['1', '2']), set(['1', '3'])])
        ss2 = ss1.copy()
        self.assertTrue(isinstance(ss2, setset))
        ss1.clear()
        self.assertEqual(repr(ss1), 'setset([])')
        self.assertEqual(repr(ss2),
                         "setset([set([]), set(['1', '2']), set(['1', '3'])])")

        # large set of sets
        ss = setset({})
        self.assertEqual(
            repr(ss),
            "setset([set([]), set(['1']), set(['2']), set(['3']), set(['4']), set(['1', ' ...")

    def test_comparison(self):
        ss = setset(set(['1', '2']))
        self.assertEqual(ss, setset(set(['1', '2'])))
        self.assertNotEqual(ss, setset(set(['1', '3'])))

        v = [set(), set(['1', '2']), set(['1', '3'])]
        ss = setset(v)
        self.assertTrue(
            ss.isdisjoint(setset([set(['1']), set(['1', '2', '3'])])))
        self.assertFalse(ss.isdisjoint(setset([set(['1']), set(['1', '2'])])))

        self.assertTrue(ss.issubset(setset(v)))
        self.assertFalse(ss.issubset(setset([set(), set(['1', '2'])])))
        self.assertTrue(ss <= setset(v))
        self.assertFalse(ss <= setset([set(), set(['1', '2'])]))
        self.assertTrue(
            ss < setset([set(), set(['1']), set(['1', '2']), set(['1', '3'])]))
        self.assertFalse(ss < setset(v))

        self.assertTrue(ss.issuperset(setset(v)))
        self.assertFalse(ss.issuperset(setset([set(['1']), set(['1', '2'])])))
        self.assertTrue(ss >= setset(v))
        self.assertFalse(ss >= setset([set(['1']), set(['1', '2'])]))
        self.assertTrue(ss > setset([set(), set(['1', '2'])]))
        self.assertFalse(ss > setset(v))

    def test_unary_operators(self):
        ss = setset([set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
                     set(['1', '2', '3', '4']), set(['1', '3', '4']),
                     set(['1', '4']), set(['4'])])

        self.assertTrue(isinstance(~ss, setset))
        self.assertEqual(
            ~ss,
             setset([set(['1', '2', '4']), set(['1', '3']), set(['2']),
                     set(['2', '3']), set(['2', '3', '4']), set(['2', '4']),
                     set(['3']), set(['3', '4'])]))

        self.assertTrue(isinstance(ss.smaller(3), setset))
        self.assertEqual(
            ss.smaller(3),
            setset([set(), set(['1']), set(['1', '2']), set(['1', '4']),
                    set(['4'])]))
        self.assertTrue(isinstance(ss.larger(3), setset))
        self.assertEqual(ss.larger(3), setset([set(['1', '2', '3', '4'])]))
        self.assertTrue(isinstance(ss.same_size(3), setset))
        self.assertEqual(ss.same_size(3), setset([set(['1', '2', '3']),
                                                  set(['1', '3', '4'])]))

        ss = setset([set(['1', '2']), set(['1', '4']), set(['2', '3']),
                     set(['3', '4'])])
        self.assertTrue(isinstance(ss.hitting(), setset))
        self.assertEqual(
            ss.hitting(),
            setset([set(['1', '2', '3']), set(['1', '2', '3', '4']),
                    set(['1', '2', '4']), set(['1', '3']), set(['1', '3', '4']),
                    set(['2', '3', '4']), set(['2', '4'])]))

        ss = setset([set(['1', '2']), set(['1', '2', '3']),
                     set(['1', '2', '3', '4']), set(['2', '4', '5'])])
        self.assertTrue(isinstance(ss.minimal(), setset))
        self.assertEqual(ss.minimal(),
                         setset([set(['1', '2']), set(['2', '4', '5'])]))
        self.assertTrue(isinstance(ss.maximal(), setset))
        self.assertEqual(
            ss.maximal(),
            setset([set(['1', '2', '3', '4']), set(['2', '4', '5'])]))

    def test_binary_operators(self):
        u = [set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
             set(['1', '2', '3', '4']), set(['1', '3', '4']), set(['1', '4']),
             set(['4'])]
        v = [set(['1', '2']), set(['1', '4']), set(['2', '3']), set(['3', '4'])]
        ss = setset(u) & setset(v)
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(['1', '2']), set(['1', '4'])]))
        ss = setset(u).intersection(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(['1', '2']), set(['1', '4'])]))

        ss = setset(u)
        ss &= setset(v)
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(['1', '2']), set(['1', '4'])]))
        ss = setset(u)
        ss.intersection_update(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(['1', '2']), set(['1', '4'])]))

        ss = setset(u) | setset(v)
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
                    set(['1', '2', '3', '4']), set(['1', '3', '4']),
                    set(['1', '4']), set(['2', '3']), set(['3', '4']),
                    set(['4'])]))
        ss = setset(u).union(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
                    set(['1', '2', '3', '4']), set(['1', '3', '4']),
                    set(['1', '4']), set(['2', '3']), set(['3', '4']),
                    set(['4'])]))

        ss = setset(u)
        ss |= setset(v)
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
                    set(['1', '2', '3', '4']), set(['1', '3', '4']),
                    set(['1', '4']), set(['2', '3']), set(['3', '4']),
                    set(['4'])]))
        ss = setset(u)
        ss.update(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
                    set(['1', '2', '3', '4']), set(['1', '3', '4']),
                    set(['1', '4']), set(['2', '3']), set(['3', '4']),
                    set(['4'])]))

        ss = setset(u) - setset(v)
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(), set(['1']), set(['1', '2', '3']),
                    set(['1', '2', '3', '4']), set(['1', '3', '4']),
                    set(['4'])]))
        ss = setset(u).difference(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(), set(['1']), set(['1', '2', '3']),
                    set(['1', '2', '3', '4']), set(['1', '3', '4']),
                    set(['4'])]))

        ss = setset(u)
        ss -= setset(v)
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(), set(['1']), set(['1', '2', '3']),
                    set(['1', '2', '3', '4']), set(['1', '3', '4']),
                    set(['4'])]))
        ss = setset(u)
        ss.difference_update(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(), set(['1']), set(['1', '2', '3']),
                    set(['1', '2', '3', '4']), set(['1', '3', '4']),
                    set(['4'])]))

        ss = setset(u) ^ setset(v)
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss
            , setset([set(), set(['1']), set(['1', '2', '3']),
                      set(['1', '2', '3', '4']), set(['1', '3', '4']),
                      set(['2', '3']), set(['3', '4']), set(['4'])]))
        ss = setset(u).symmetric_difference(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(), set(['1']), set(['1', '2', '3']),
                    set(['1', '2', '3', '4']), set(['1', '3', '4']),
                    set(['2', '3']), set(['3', '4']), set(['4'])]))

        ss = setset(u)
        ss ^= setset(v)
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(), set(['1']), set(['1', '2', '3']),
                    set(['1', '2', '3', '4']), set(['1', '3', '4']),
                    set(['2', '3']), set(['3', '4']), set(['4'])]))
        ss = setset(u)
        ss.symmetric_difference_update(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(), set(['1']), set(['1', '2', '3']),
                    set(['1', '2', '3', '4']), set(['1', '3', '4']),
                    set(['2', '3']), set(['3', '4']), set(['4'])]))

        v = [set(['1', '2'])]
        ss = setset(u) / setset(v)
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(), set(['3']), set(['3', '4'])]))
        ss = setset(u).quotient(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(), set(['3']), set(['3', '4'])]))

        ss = setset(u)
        ss /= setset(v)
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(), set(['3']), set(['3', '4'])]))
        ss = setset(u)
        ss.quotient_update(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(), set(['3']), set(['3', '4'])]))

        ss = setset(u) % setset(v)
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(), set(['1']), set(['1', '3', '4']),
                                     set(['1', '4']), set(['4'])]))
        ss = setset(u).remainder(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(), set(['1']), set(['1', '3', '4']),
                                     set(['1', '4']), set(['4'])]))

        ss = setset(u)
        ss %= setset(v)
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(), set(['1']), set(['1', '3', '4']),
                                     set(['1', '4']), set(['4'])]))
        ss = setset(u)
        ss.remainder_update(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(), set(['1']), set(['1', '3', '4']),
                                     set(['1', '4']), set(['4'])]))

        ss = setset(u).flip('1')
        self.assertEqual(ss, setset([set(), set(['1']), set(['1', '4']), set(['2']),
                                     set(['2', '3']), set(['2', '3', '4']),
                                     set(['3', '4']), set(['4'])]))

        v = [set(['1', '2']), set(['1', '4']), set(['2', '3']), set(['3', '4'])]
        ss = setset(u).join(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(['1', '2']), set(['1', '2', '3']), set(['1', '2', '4']),
                    set(['1', '2', '3', '4']), set(['1', '3', '4']),
                    set(['1', '4']), set(['2', '3']), set(['2', '3', '4']),
                    set(['3', '4'])]))

        ss = setset(u).meet(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(), set(['1']), set(['1', '2']), set(['1', '4']),
                    set(['2']), set(['2', '3']), set(['3']), set(['3', '4']),
                    set(['4'])]))

        v = [set(['1', '2']), set(['1', '4']), set(['2', '3']), set(['3', '4'])]
        ss = setset(u).subsets(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(), set(['1']), set(['1', '2']), set(['1', '4']),
                    set(['4'])]))

        ss = setset(u).supersets(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(['1', '2']), set(['1', '2', '3']),
                    set(['1', '2', '3', '4']), set(['1', '3', '4']),
                    set(['1', '4'])]))

        ss = setset(u).non_subsets(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(['1', '2', '3']), set(['1', '2', '3', '4']),
                    set(['1', '3', '4'])]))

        ss = setset(u).non_supersets(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(), set(['1']), set(['4'])]))

    def capacity(self):
        ss = setset()
        self.assertFalse(ss)

        ss = setset([set(), set(['1', '2']), set(['1', '3'])])
        self.assertTrue(ss)

        self.assertEqual(len(ss), 3)
        self.assertEqual(ss.len(), 3)

    def test_iterators(self):
        ss1 = setset([set(), set(['1', '2']), set(['1', '3'])])
        ss2 = setset()
        for s in ss1:
            ss2 = ss2 | setset(s)
        self.assertEqual(ss1, setset([set(), set(['1', '2']), set(['1', '3'])]))
        self.assertEqual(ss1, ss2)

        ss2 = setset()
        for s in ss1:
            ss2 = ss2 | setset(s)
        self.assertEqual(ss1, ss2)

        ss1 = setset([set(), set(['1', '2']), set(['1', '3'])])
        ss2 = setset()
        for s in ss1.randomize():
            ss2 = ss2 | setset(s)
        self.assertEqual(ss1, ss2)

        gen = ss1.randomize()
        self.assertTrue(isinstance(gen.next(), set))

        ss = setset([set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
                     set(['1', '2', '3', '4']), set(['1', '3', '4']),
                     set(['1', '4']), set(['4'])])
        r = []
        for s in ss.maximize({'1': .3, '2': -.2, '3': -.2}, default=.4):
            r.append(s)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], set(['1', '4']))
        self.assertEqual(r[1], set(['1', '3', '4']))
        self.assertEqual(r[2], set(['4']))

        r = []
        for s in ss.maximize():
            r.append(s)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], set(['1', '2', '3', '4']))
        self.assertEqual(r[-1], set())

        r = []
        for s in ss.minimize({'1': .3, '2': -.2, '3': -.2}, default=.4):
            r.append(s)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], set(['1', '2', '3']))
        self.assertEqual(r[1], set())
        self.assertEqual(r[2], set(['1', '2']))

        r = []
        for s in ss.minimize():
            r.append(s)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], set())
        self.assertEqual(r[-1], set(['1', '2', '3', '4']))

    def test_lookup(self):
        ss1 = setset([set(), set(['1', '2']), set(['1', '3'])])
        self.assertTrue(set(['1', '2']) in ss1)
        self.assertTrue(set(['1']) not in ss1)

        ss2 = ss1.include('1')
        self.assertTrue(isinstance(ss2, setset))
        self.assertEqual(ss2, setset([set(['1', '2']), set(['1', '3'])]))

        ss2 = ss1.exclude('2')
        self.assertTrue(isinstance(ss2, setset))
        self.assertEqual(ss2, setset([set(), set(['1', '3'])]))

    def test_modifiers(self):
        v = [set(), set(['1', '2']), set(['1', '3'])]
        ss = setset(v)
        ss.add(set(['1']))
        self.assertTrue(set(['1']) in ss)

        ss.remove(set(['1']))
        self.assertTrue(set(['1']) not in ss)
        self.assertRaises(KeyError, ss.remove, set(['1']))

        ss.add(set(['1']))
        ss.discard(set(['1']))
        self.assertTrue(set(['1']) not in ss)
        ss.discard(set(['1']))  # no exception raised

        ss = setset(v)
        ss.add('2')
        self.assertEqual(ss, setset([set(['1', '2']), set(['1', '2', '3']),
                                     set(['2'])]))

        ss = setset(v)
        ss.remove('2')
        self.assertEqual(ss, setset([set(), set(['1']), set(['1', '3'])]))
        self.assertRaises(KeyError, ss.remove, '4')

        ss = setset(v)
        ss.discard('2')
        self.assertEqual(ss, setset([set(), set(['1']), set(['1', '3'])]))
        ss.discard('4')  # no exception raised

        v = [set(['1']), set(['1', '2']), set(['1', '3'])]
        ss = setset(v)
        s = ss.pop()
        self.assertTrue(s not in ss)
        self.assertEqual(ss | setset(s), setset(v))

        self.assertTrue(ss)
        ss.clear()
        self.assertFalse(ss)

        self.assertRaises(KeyError, ss.pop)

    def test_io(self):
        ss = setset()
        st = ss.dumps()
        self.assertEqual(st, "B\n.\n")
        ss.loads(st)
        self.assertEqual(ss, setset())

        ss = setset(set())
        st = ss.dumps()
        self.assertEqual(st, "T\n.\n")
        ss.loads(st)
        self.assertEqual(ss, setset(set()))

        v = [set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
             set(['1', '2', '3', '4']), set(['1', '3', '4']), set(['1', '4']),
             set(['4'])]
        ss = setset(v)
        st = ss.dumps()
        ss = setset()
        ss.loads(st)
        self.assertEqual(ss, setset(v))

        # skip this test, becasue string is treated as an element
#        ss = setset(st)
#        self.assertEqual(ss, setset(v))

        f = tempfile.TemporaryFile()
        ss.dump(f)
        ss = setset()
        f.seek(0)
        ss.load(f)
        self.assertEqual(ss, setset(v))

    def test_large(self):
        n = 5000
        setset.set_universe(xrange(n))
        ss = setset({}) - setset([set([1]) - set([1, 2])])
        self.assertTrue(ss)
        self.assertAlmostEqual(ss.len() / (2**n - 2), 1)

        i = 0
        for s in ss:
            if i > 3: break
            i += 1


class TestGraphSet(unittest.TestCase):

    def setUp(self):
        GraphSet.set_universe([(1, 2, .3), (1, 3, -.2), (2, 4, -.2), (3, 4, .4)],
                              traversal='dfs', source=1)

    def tearDown(self):
        pass

    def test_init(self):
        GraphSet.set_universe([('i', 'ii')])
        self.assertEqual(GraphSet.get_universe(), [('i', 'ii')])

        gs = GraphSet({})
        self.assertEqual(len(gs), 2**1)

        GraphSet.set_universe([(1, 2, .3), (1, 3, -.2), (2, 4, -.2), (3, 4, .4)],
                              traversal='dfs', source=1)
        self.assertEqual(GraphSet.get_universe(),
                         [(1, 3, -.2), (3, 4, .4), (1, 2, .3), (2, 4, -.2)])

        gs = GraphSet({})
        self.assertEqual(len(gs), 2**4)

    def test_constructors(self):
        gs = GraphSet()
        self.assertTrue(isinstance(gs, GraphSet))
        self.assertEqual(len(gs), 0)

        gs = GraphSet(set([(2, 1)]))
        self.assertEqual(len(gs), 1)
        self.assertTrue(set([(1, 2)]) in gs)

        gs = GraphSet([set([(1, 2)]), set([(3, 1)])])
        self.assertEqual(len(gs), 2)
        self.assertTrue(set([(1, 2)]) in gs)
        self.assertTrue(set([(1, 3)]) in gs)

        gs = GraphSet({'include': [(1, 2), (1, 3)], 'exclude': [(4, 3)]})
        self.assertEqual(len(gs), 2)
        self.assertTrue(set([(1, 2), (1, 3)]) in gs)
        self.assertTrue(set([(1, 2), (1, 3), (2, 4)]) in gs)

        self.assertRaises(KeyError, GraphSet, set([(1, 4)]))
        self.assertRaises(KeyError, GraphSet, [set([(1, 4)])])
        self.assertRaises(KeyError, GraphSet, {'include': [(1, 4)]})

    def test_subgraphs(self):
        pass

    def test_binary_operators(self):
        gs = GraphSet([set([(1, 2)]), set([(2, 4), (3, 4)])])
        gs = gs.flip((4, 2))
        self.assertEqual(gs, GraphSet([set([(1, 2), (2, 4)]), set([(3, 4)])]))

    def test_iterators(self):
        gs = GraphSet({})
        r = []
        for s in gs.maximize():
            r.append(s)
        self.assertEqual(len(r), 16)
        self.assertEqual(r[0], set([(1, 2), (3, 4)]))
        self.assertEqual(r[-1], set([(1, 3), (2, 4)]))

    def test_lookup(self):
        gs1 = GraphSet({}) - GraphSet([set([(1, 2)]), set([(2, 4), (3, 4)])])

        self.assertTrue(set([(1, 2), (1, 3)]) in gs1)
        self.assertTrue(set([(1, 2)]) not in gs1)

        gs2 = gs1.include((2, 1))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.exclude((1, 3))
        self.assertEqual(len(gs2), 6)

        self.assertEqual(len(gs1.include(1)), 11)
        self.assertEqual(len(gs1.exclude(1)), 3)

    def test_modifiers(self):
        gs = GraphSet({}) - GraphSet([set([(1, 2)]), set([(2, 4), (3, 4)])])

        gs.add(set([(1, 2)]))
        self.assertEqual(len(gs), 15)

        gs.remove(set([(1, 2)]))
        self.assertEqual(len(gs), 14)

        gs.discard(set([(1, 2)]))
        self.assertEqual(len(gs), 14)

        self.assertRaises(KeyError, gs.add, set([(1, 4)]))
        self.assertRaises(KeyError, gs.remove, set([(1, 4)]))
        self.assertRaises(KeyError, gs.discard, set([(1, 4)]))

        gs = GraphSet([set([(1, 2)]), set([(3, 4)])])
        gs.add((3, 4))
        self.assertEqual(gs, GraphSet([set([(1, 2), (3, 4)]), set([(3, 4)])]))

        gs = GraphSet([set([(1, 2), (3, 4)]), set([(1, 2)]), set([(1, 3)])])
        gs.remove((3, 4))
        self.assertEqual(gs, GraphSet([set([(1, 2)]), set([(1, 3)])]))

        self.assertRaises(KeyError, gs.add, (1, 4))
        self.assertRaises(KeyError, gs.remove, (1, 4))

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
