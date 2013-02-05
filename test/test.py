import unittest
from illion import setset, graphset


class TestSetset(unittest.TestCase):

    def setUp(self):
        setset.universe([])

    def tearDown(self):
        pass

    def runTest(self):
        self.init()
        self.constructors()
        self.comparison()
        self.unary_operators()
        self.binary_operators()
        self.capacity()
        self.iterators()
        self.lookup()
        self.modifiers()
        self.io()
        self.large()

    def init(self):
        self.assertEqual(setset._obj2int, {})
        self.assertEqual(setset._int2obj, [None])
        self.assertEqual(setset.universe(), [])

        setset.universe(['i', 'ii'])
        self.assertEqual(setset._obj2int, {'i': 1, 'ii': 2})
        self.assertEqual(setset._int2obj, [None, 'i', 'ii'])
        self.assertEqual(setset.universe(), ['i', 'ii'])

        ss = setset({})
        self.assertEqual(
            ss,
            setset([set(), set(['i']), set(['i', 'ii']), set(['ii'])]))

        setset.universe(['1'])
        self.assertEqual(setset._obj2int, {'1': 1})
        self.assertEqual(setset._int2obj, [None, '1'])
        self.assertEqual(setset.universe(), ['1'])

        ss = setset({})
        self.assertEqual(ss, setset([set(), set(['1'])]))

    def constructors(self):
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

        # more than four sets
        ss = setset([set(), set(['1']), set(['1', '2']), set(['1', '4']),
                     set(['4'])])
        self.assertEqual(
            repr(ss),
            "setset([set([]), set(['1']), set(['4']), set(['1', '2']), ...])")

    def comparison(self):
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

    def unary_operators(self):
        self.assertEqual(setset.universe(), ['1', '2', '3', '4'])

        ss = setset([set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
                     set(['1', '2', '3', '4']), set(['1', '3', '4']),
                     set(['1', '4']), set(['4'])])

        self.assertTrue(isinstance(~ss, setset))
        self.assertEqual(
            ~ss,
             setset([set(['1', '2', '4']), set(['1', '3']), set(['2']),
                     set(['2', '3']), set(['2', '3', '4']), set(['2', '4']),
                     set(['3']), set(['3', '4'])]))
        self.assertTrue(isinstance(ss.complement(), setset))
        self.assertEqual(
            ss.complement(),
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
        self.assertTrue(isinstance(ss.equal(3), setset))
        self.assertEqual(ss.equal(3), setset([set(['1', '2', '3']),
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

    def binary_operators(self):
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

        ss = setset(u).nonsubsets(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(
            ss,
            setset([set(['1', '2', '3']), set(['1', '2', '3', '4']),
                    set(['1', '3', '4'])]))

        ss = setset(u).nonsupersets(setset(v))
        self.assertTrue(isinstance(ss, setset))
        self.assertEqual(ss, setset([set(), set(['1']), set(['4'])]))

    def capacity(self):
        ss = setset()
        self.assertFalse(ss)

        ss = setset([set(), set(['1', '2']), set(['1', '3'])])
        self.assertTrue(ss)

        self.assertEqual(len(ss), 3)
        self.assertEqual(ss.len(), 3)

    def iterators(self):
        ss1 = setset([set(), set(['1', '2']), set(['1', '3'])])
        ss2 = setset()
        for s in ss1:
            ss2 = ss2 | setset(s)
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

        ss = setset([set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
                     set(['1', '2', '3', '4']), set(['1', '3', '4']),
                     set(['1', '4']), set(['4'])])
        r = []
        for s in ss.optimize({'1': .3, '2': -.2, '3': -.2, '4': .4}):
            r.append(s)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], set(['1', '4']))
        self.assertEqual(r[1], set(['1', '3', '4']))
        self.assertEqual(r[2], set(['4']))

        r = []
        for s in ss.optimize({}):
            r.append(s)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], set(['1', '2', '3', '4']))
        self.assertEqual(r[-1], set())

    def lookup(self):
        ss1 = setset([set(), set(['1', '2']), set(['1', '3'])])
        self.assertTrue(set(['1', '2']) in ss1)
        self.assertTrue(set(['1']) not in ss1)

        ss2 = ss1.include('1')
        self.assertTrue(isinstance(ss2, setset))
        self.assertEqual(ss2, setset([set(['1', '2']), set(['1', '3'])]))

        ss2 = ss1.exclude('2')
        self.assertTrue(isinstance(ss2, setset))
        self.assertEqual(ss2, setset([set(), set(['1', '3'])]))

    def modifiers(self):
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

        v = [set(['1']), set(['1', '2']), set(['1', '3'])]
        ss = setset(v)
        s = ss.pop()
        self.assertTrue(s not in ss)
        self.assertEqual(ss | setset(s), setset(v))

        self.assertTrue(ss)
        ss.clear()
        self.assertFalse(ss)

        self.assertRaises(KeyError, ss.pop)

    def io(self):
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

        # skip this test, becasue string is treated as an element in hook_arg()
#        ss = setset(st)
#        self.assertEqual(ss, setset(v))

        ss.dump(open('/tmp/illion_', 'w'))
        ss = setset()
        ss.load(open('/tmp/illion_'))
        self.assertEqual(ss, setset(v))

    def large(self):
        setset.universe(xrange(10000))
        ss = setset({}) - setset([set([1]) - set([1, 2])])
        self.assertTrue(ss)
        self.assertEqual(len(str(ss.len())), 3011)


class TestGraphset(unittest.TestCase):

    def setUp(self):
        setset.universe([])

    def tearDown(self):
        pass

    def runTest(self):
        self.init()
        self.constructors()
        self.subgraphs()
        self.iterators()
        self.lookup()
        self.modifiers()
        self.large()

    def init(self):
        graphset.universe([('i', 'ii')])
        self.assertEqual(graphset.universe(), [('i', 'ii')])

        gs = graphset({})
        self.assertEqual(len(gs), 2**1)

        graphset.universe([(1, 2, .3), (1, 3, -.2), (2, 4, -.2), (3, 4, .4)],
                          traversal='dfs', source=1)
        self.assertEqual(graphset.universe(),
                         [(1, 3, -.2), (3, 4, .4), (1, 2, .3), (2, 4, -.2)])

        gs = graphset({})
        self.assertEqual(len(gs), 2**4)

    def constructors(self):
        gs = graphset()
        self.assertTrue(isinstance(gs, graphset))
        self.assertEqual(len(gs), 0)

        gs = graphset(set([(2, 1)]))
        self.assertEqual(len(gs), 1)
        self.assertTrue(set([(1, 2)]) in gs)

        gs = graphset([set([(1, 2)]), set([(3, 1)])])
        self.assertEqual(len(gs), 2)
        self.assertTrue(set([(1, 2)]) in gs)
        self.assertTrue(set([(1, 3)]) in gs)

        gs = graphset({'include': [(1, 2), (1, 3)], 'exclude': [(4, 3)]})
        self.assertEqual(len(gs), 2)
        self.assertTrue(set([(1, 2), (1, 3)]) in gs)
        self.assertTrue(set([(1, 2), (1, 3), (2, 4)]) in gs)

        self.assertRaises(KeyError, graphset, set([(1, 4)]))
        self.assertRaises(KeyError, graphset, [set([(1, 4)])])
        self.assertRaises(KeyError, graphset, {'include': [(1, 4)]})

    def subgraphs(self):
        pass

    def iterators(self):
        gs = graphset({})
        r = []
        for s in gs.optimize():
            r.append(s)
        self.assertEqual(len(r), 16)
        self.assertEqual(r[0], set([(1, 2), (3, 4)]))
        self.assertEqual(r[-1], set([(1, 3), (2, 4)]))

    def lookup(self):
        gs1 = graphset({}) - graphset([set([(1, 2)]), set([(2, 4), (3, 4)])])

        self.assertTrue(set([(1, 2), (1, 3)]) in gs1)
        self.assertTrue(set([(1, 2)]) not in gs1)

        gs2 = gs1.include_edge((2, 1))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.exclude_edge((1, 3))
        self.assertEqual(len(gs2), 6)

        self.assertEqual(len(gs1.include_vertex(1)), 11)
        self.assertEqual(len(gs1.exclude_vertex(1)), 3)

    def modifiers(self):
        gs = graphset({}) - graphset([set([(1, 2)]), set([(2, 4), (3, 4)])])

        gs.add(set([(1, 2)]))
        self.assertEqual(len(gs), 15)

        gs.remove(set([(1, 2)]))
        self.assertEqual(len(gs), 14)

        gs.discard(set([(1, 2)]))
        self.assertEqual(len(gs), 14)

        self.assertRaises(KeyError, gs.add, set([(1, 4)]))
        self.assertRaises(KeyError, gs.remove, set([(1, 4)]))
        self.assertRaises(KeyError, gs.discard, set([(1, 4)]))

    def large(self):
        import networkx as nx

        g = nx.grid_2d_graph(11, 11)
        graphset.universe(g.edges(), traversal='bfs')
        self.assertEqual(len(graphset.universe()), 220)
        self.assertEqual(graphset.universe()[:2],
                         [((0, 1), (0, 0)), ((1, 0), (0, 0))])

        gs = graphset({});
        gs -= graphset([set([((0, 1), (0, 0))]),
                        set([((0, 1), (0, 0)), ((1, 0), (0, 0))])])
        self.assertEqual(gs.len(), 2**220 - 2)

        del nx

if __name__ == '__main__':
    unittest.main()
