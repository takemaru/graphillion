from illion import setset, graphset
import networkx as nx


class TestSetset(object):

    def run(self):
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
        assert setset._obj2int == {} and setset._int2obj  == [None]
        assert setset.universe() == []

        setset.universe(['i', 'ii'])
        assert setset._obj2int == {'i': 1, 'ii': 2}
        assert setset._int2obj == [None, 'i', 'ii']
        assert setset.universe() == ['i', 'ii']

        ss = setset({})
        assert ss == setset([set(), set(['i']), set(['i', 'ii']), set(['ii'])])

        setset.universe(['1'])
        assert setset._obj2int == {'1': 1}
        assert setset._int2obj == [None, '1']
        assert setset.universe() == ['1']

        ss = setset({})
        assert ss == setset([set(), set(['1'])])

    def constructors(self):
        name = 'setset'

        ss = setset()
        assert isinstance(ss, setset)
        assert repr(ss) == '<%s object of 0x8000000000>' % name
        assert ss._enums() == name + '([])'

        ss = setset(set())
        assert ss._enums() == name + '([set([])])'

        ss = setset(frozenset(['1', '2']))
        assert ss._enums() == name + '([set([1, 2])])'

        ss = setset([set(), set(['1', '2']), set(['1', '3'])])
        assert ss._enums() == name + '([set([1, 2]), set([1, 3]), set([])])'

        ss = setset({'include': ('1', '2'), 'exclude': ('4',)})
        assert ss._enums() == name + '([set([1, 2, 3]), set([1, 2])])'

        # copy constructor
        ss = setset([set(), set(['1', '2']), set(['1', '3'])])
        assert ss._enums() == name + '([set([1, 2]), set([1, 3]), set([])])'

        ss1 = setset([set(), set(['1', '2']), set(['1', '3'])])
        ss2 = ss1.copy()
        assert isinstance(ss2, setset)
        ss1.clear()
        assert ss1._enums() == name + '([])'
        assert ss2._enums() == name + '([set([1, 2]), set([1, 3]), set([])])'

    def comparison(self):
        ss = setset(set(['1', '2']))
        assert ss == setset(set(['1', '2']))
        assert ss != setset(set(['1', '3']))

        v = [set(), set(['1', '2']), set(['1', '3'])]
        ss = setset(v)
        assert ss.isdisjoint(setset([set(['1']), set(['1', '2', '3'])]))
        assert not ss.isdisjoint(setset([set(['1']), set(['1', '2'])]))

        assert ss.issubset(setset(v))
        assert not ss.issubset(setset([set(), set(['1', '2'])]))
        assert ss <= setset(v)
        assert not (ss <= setset([set(), set(['1', '2'])]))
        assert ss < setset([set(), set(['1']), set(['1', '2']), set(['1', '3'])])
        assert not (ss < setset(v))

        assert ss.issuperset(setset(v))
        assert not ss.issuperset(setset([set(['1']), set(['1', '2'])]))
        assert ss >= setset(v)
        assert not (ss >= setset([set(['1']), set(['1', '2'])]))
        assert ss > setset([set(), set(['1', '2'])])
        assert not (ss > setset(v))

    def unary_operators(self):
        assert setset.universe() == ['1', '2', '3', '4']

        ss = setset([set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
                     set(['1', '2', '3', '4']), set(['1', '3', '4']),
                     set(['1', '4']), set(['4'])])

        assert isinstance(~ss, setset)
        assert ~ss == setset([set(['1', '2', '4']), set(['1', '3']), set(['2']),
                              set(['2', '3']), set(['2', '3', '4']),
                              set(['2', '4']), set(['3']), set(['3', '4'])])
        assert isinstance(ss.complement(), setset)
        assert ss.complement() == setset([set(['1', '2', '4']), set(['1', '3']),
                                          set(['2']), set(['2', '3']),
                                          set(['2', '3', '4']), set(['2', '4']),
                                          set(['3']), set(['3', '4'])])
        assert isinstance(ss.smaller(3), setset)
        assert ss.smaller(3) == setset([set(), set(['1']), set(['1', '2']),
                                        set(['1', '4']), set(['4'])])

        ss = setset([set(['1', '2']), set(['1', '4']), set(['2', '3']),
                     set(['3', '4'])])
        assert isinstance(ss.hitting(), setset)
        assert ss.hitting() == setset([set(['1', '2', '3']),
                                       set(['1', '2', '3', '4']),
                                       set(['1', '2', '4']), set(['1', '3']),
                                       set(['1', '3', '4']),
                                       set(['2', '3', '4']), set(['2', '4'])])

        ss = setset([set(['1', '2']), set(['1', '2', '3']),
                     set(['1', '2', '3', '4']), set(['2', '4', '5'])])
        assert isinstance(ss.minimal(), setset)
        assert ss.minimal() == setset([set(['1', '2']), set(['2', '4', '5'])])
        assert isinstance(ss.maximal(), setset)
        assert ss.maximal() == setset([set(['1', '2', '3', '4']),
                                       set(['2', '4', '5'])])

    def binary_operators(self):
        u = [set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
             set(['1', '2', '3', '4']),
             set(['1', '3', '4']), set(['1', '4']), set(['4'])]
        v = [set(['1', '2']), set(['1', '4']), set(['2', '3']), set(['3', '4'])]
        ss = setset(u) & setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(['1', '2']), set(['1', '4'])])
        ss = setset(u).intersection(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(['1', '2']), set(['1', '4'])])

        ss = setset(u)
        ss &= setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(['1', '2']), set(['1', '4'])])
        ss = setset(u)
        ss.intersection_update(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(['1', '2']), set(['1', '4'])])

        ss = setset(u) | setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2']),
                             set(['1', '2', '3']), set(['1', '2', '3', '4']),
                             set(['1', '3', '4']), set(['1', '4']),
                             set(['2', '3']), set(['3', '4']), set(['4'])])
        ss = setset(u).union(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2']),
                             set(['1', '2', '3']), set(['1', '2', '3', '4']),
                             set(['1', '3', '4']), set(['1', '4']),
                             set(['2', '3']), set(['3', '4']), set(['4'])])

        ss = setset(u)
        ss |= setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2']),
                             set(['1', '2', '3']), set(['1', '2', '3', '4']),
                             set(['1', '3', '4']), set(['1', '4']),
                             set(['2', '3']), set(['3', '4']), set(['4'])])
        ss = setset(u)
        ss.update(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2']),
                             set(['1', '2', '3']), set(['1', '2', '3', '4']),
                             set(['1', '3', '4']), set(['1', '4']),
                             set(['2', '3']), set(['3', '4']), set(['4'])])

        ss = setset(u) - setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2', '3']),
                             set(['1', '2', '3', '4']), set(['1', '3', '4']),
                             set(['4'])])
        ss = setset(u).difference(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2', '3']),
                             set(['1', '2', '3', '4']), set(['1', '3', '4']),
                             set(['4'])])

        ss = setset(u)
        ss -= setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2', '3']),
                             set(['1', '2', '3', '4']), set(['1', '3', '4']),
                             set(['4'])])
        ss = setset(u)
        ss.difference_update(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2', '3']),
                             set(['1', '2', '3', '4']), set(['1', '3', '4']),
                             set(['4'])])

        ss = setset(u) ^ setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2', '3']),
                             set(['1', '2', '3', '4']), set(['1', '3', '4']),
                             set(['2', '3']), set(['3', '4']), set(['4'])])
        ss = setset(u).symmetric_difference(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2', '3']),
                             set(['1', '2', '3', '4']), set(['1', '3', '4']),
                             set(['2', '3']), set(['3', '4']), set(['4'])])

        ss = setset(u)
        ss ^= setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2', '3']),
                             set(['1', '2', '3', '4']), set(['1', '3', '4']),
                             set(['2', '3']), set(['3', '4']), set(['4'])])
        ss = setset(u)
        ss.symmetric_difference_update(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2', '3']),
                             set(['1', '2', '3', '4']), set(['1', '3', '4']),
                             set(['2', '3']), set(['3', '4']), set(['4'])])

        v = [set(['1', '2'])]
        ss = setset(u) / setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['3']), set(['3', '4'])])
        ss = setset(u).quotient(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['3']), set(['3', '4'])])

        ss = setset(u)
        ss /= setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['3']), set(['3', '4'])])
        ss = setset(u)
        ss.quotient_update(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['3']), set(['3', '4'])])

        ss = setset(u) % setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '3', '4']),
                             set(['1', '4']), set(['4'])])
        ss = setset(u).remainder(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '3', '4']),
                             set(['1', '4']), set(['4'])])

        ss = setset(u)
        ss %= setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '3', '4']),
                             set(['1', '4']), set(['4'])])
        ss = setset(u)
        ss.remainder_update(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '3', '4']),
                             set(['1', '4']), set(['4'])])

        v = [set(['1', '2']), set(['1', '4']), set(['2', '3']), set(['3', '4'])]
        ss = setset(u).join(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(['1', '2']), set(['1', '2', '3']),
                             set(['1', '2', '4']), set(['1', '2', '3', '4']),
                             set(['1', '3', '4']), set(['1', '4']),
                             set(['2', '3']), set(['2', '3', '4']),
                             set(['3', '4'])])

        ss = setset(u).meet(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2']), set(['1', '4']),
                             set(['2']), set(['2', '3']), set(['3']),
                             set(['3', '4']), set(['4'])])

        v = [set(['1', '2']), set(['1', '4']), set(['2', '3']), set(['3', '4'])]
        ss = setset(u).subsets(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['1', '2']), set(['1', '4']),
                             set(['4'])])

        ss = setset(u).supersets(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(['1', '2']), set(['1', '2', '3']),
                             set(['1', '2', '3', '4']), set(['1', '3', '4']),
                             set(['1', '4'])])

        ss = setset(u).nonsubsets(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(['1', '2', '3']), set(['1', '2', '3', '4']),
                             set(['1', '3', '4'])])

        ss = setset(u).nonsupersets(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['1']), set(['4'])])

    def capacity(self):
        ss = setset()
        assert not ss

        ss = setset([set(), set(['1', '2']), set(['1', '3'])])
        assert not not ss

        assert len(ss) == 3
        assert ss.len() == 3

    def iterators(self):
        ss1 = setset([set(), set(['1', '2']), set(['1', '3'])])
        ss2 = setset()
        for s in ss1:
            ss2 = ss2 | setset(s)
        assert ss1 == ss2

        ss2 = setset()
        for s in ss1:
            ss2 = ss2 | setset(s)
        assert ss1 == ss2

        ss1 = setset([set(), set(['1', '2']), set(['1', '3'])])
        ss2 = setset()
        for s in ss1.randomize():
            ss2 = ss2 | setset(s)
        assert ss1 == ss2

        ss = setset([set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
                     set(['1', '2', '3', '4']), set(['1', '3', '4']),
                     set(['1', '4']), set(['4'])])
        r = []
        for s in ss.optimize({'1': .3, '2': -.2, '3': -.2, '4': .4}):
            r.append(s)
        assert len(r) == 8
        assert r[0] == set(['1', '4'])
        assert r[1] == set(['1', '3', '4'])
        assert r[2] == set(['4'])

    def lookup(self):
        ss1 = setset([set(), set(['1', '2']), set(['1', '3'])])
        assert set(['1', '2']) in ss1
        assert set(['1']) not in ss1

        ss2 = ss1.include('1')
        assert isinstance(ss2, setset)
        assert ss2 == setset([set(['1', '2']), set(['1', '3'])])

        ss2 = ss1.exclude('2')
        assert isinstance(ss2, setset)
        assert ss2 == setset([set(), set(['1', '3'])])

    def modifiers(self):
        v = [set(), set(['1', '2']), set(['1', '3'])]
        ss = setset(v)
        ss.add(set(['1']))
        assert set(['1']) in ss

        ss.remove(set(['1']))
        assert set(['1']) not in ss

        try:
            ss.remove(set(['1']))
        except KeyError:
            pass
        else:
            assert False

        ss.add(set(['1']))
        ss.discard(set(['1']))
        assert set(['1']) not in ss
        ss.discard(set(['1']))  # no exception raised

        v = [set(['1']), set(['1', '2']), set(['1', '3'])]
        ss = setset(v)
        s = ss.pop()
        assert s not in ss
        assert ss | setset(s) == setset(v)

        assert ss
        ss.clear()
        assert not ss

        try:
            ss.pop()
        except KeyError:
            pass
        else:
            assert False

    def io(self):
        ss = setset()
        st = ss.dumps()
        assert st == "B\n.\n"
        ss.loads(st)
        assert ss == setset()

        ss = setset(set())
        st = ss.dumps()
        assert st == "T\n.\n"
        ss.loads(st)
        assert ss == setset(set())

        v = [set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
             set(['1', '2', '3', '4']), set(['1', '3', '4']), set(['1', '4']),
             set(['4'])]
        ss = setset(v)
        st = ss.dumps()
        ss = setset()
        ss.loads(st)
        assert ss == setset(v)

        # skip this test, becasue string is treated as an element in hook_arg()
#        ss = setset(st)
#        assert ss == setset(v)

        ss.dump(open('/tmp/illion_', 'w'))
        ss = setset()
        ss.load(open('/tmp/illion_'))
        assert ss == setset(v)

    def large(self):
        setset.universe(xrange(10000))
        ss = setset({}) - setset([set([1]) - set([1, 2])])
        assert not not ss
        assert len(str(ss.len())) == 3011


class TestGraphset(object):

    def run(self):
        self.init()
        self.constructors()
        self.subgraphs()
        self.iterators()
        self.lookup()
        self.modifiers()
        self.large()

    def init(self):
        g = nx.Graph()
        g.add_edge('i', 'ii')
        graphset.universe(g)
        g = graphset.universe()
        assert g.edges() == [('i', 'ii')]
        assert 'weight' not in g.edge['i']['ii']

        gs = graphset({})
        assert len(gs) == 2**1

        g = nx.Graph()
        edges = [(1, 2, .3), (1, 3, -.2), (2, 4, -.2), (3, 4, .4)]
        g.add_weighted_edges_from(edges)
        graphset.universe(g)
        g = graphset.universe()
        assert g.edges() == [(1, 2), (1, 3), (2, 4), (3, 4)]
        assert g.edge[1][2]['weight'] == .3

        gs = graphset({})
        assert len(gs) == 2**4

    def constructors(self):
        gs = graphset()
        assert isinstance(gs, graphset)
        assert len(gs) == 0

        gs = graphset(set([(1, 2)]))
        assert len(gs) == 1
        assert set([(1, 2)]) in gs

        gs = graphset([set([(1, 2)]), set([(1, 3)])])
        assert len(gs) == 2
        assert set([(1, 2)]) in gs
        assert set([(1, 3)]) in gs

        gs = graphset({'include': [(1, 2), (1, 3)], 'exclude': [(3, 4)]})
        assert len(gs) == 2
        assert set([(1, 2), (1, 3)]) in gs
        assert set([(1, 2), (1, 3), (2, 4)]) in gs

        try:
            gs = graphset(set([(1, 4)]))
        except KeyError:
            pass
        else:
            assert False

        try:
            gs = graphset([set([(1, 4)])])
        except KeyError:
            pass
        else:
            assert False

        try:
            gs = graphset({'include': [(1, 4)]})
        except KeyError:
            pass
        else:
            assert False

        g = graphset.universe()
        assert g.edges() == [(1, 2), (1, 3), (2, 4), (3, 4)]

    def subgraphs(self):
        pass

    def iterators(self):
        gs = graphset({})
        r = []
        for s in gs.optimize():
            r.append(s)
        assert len(r) == 16
        assert r[0] == set([(1, 2), (3, 4)])
        assert r[-1] == set([(1, 3), (2, 4)])

    def lookup(self):
        gs1 = graphset({}) - graphset([set([(1, 2)]), set([(2, 4), (3, 4)])])

        assert set([(1, 2), (1, 3)]) in gs1
        assert set([(1, 2)]) not in gs1

        gs2 = gs1.include((1, 2))
        assert len(gs2) == 7

        gs2 = gs1.exclude((1, 3))
        assert len(gs2) == 6

        assert len(gs1.include_vertex(1)) == 11
        assert len(gs1.exclude_vertex(1)) == 3

    def modifiers(self):
        gs = graphset({}) - graphset([set([(1, 2)]), set([(2, 4), (3, 4)])])

        gs.add(set([(1, 2)]))
        assert len(gs) == 15

        gs.remove(set([(1, 2)]))
        assert len(gs) == 14

        gs.discard(set([(1, 2)]))
        assert len(gs) == 14

        try:
            gs.add(set([(1, 4)]))
        except KeyError:
            pass
        else:
            assert False

        try:
            gs.remove(set([(1, 4)]))
        except KeyError:
            pass
        else:
            assert False

        try:
            gs.discard(set([(1, 4)]))
        except KeyError:
            pass
        else:
            assert False

    def large(self):
        g = nx.grid_2d_graph(11, 11)
        graphset.universe(g)
        assert len(graphset.universe().edges()) == 220

        gs = graphset({});
        gs -= graphset([set([((0, 1), (0, 0))]),
                        set([((0, 1), (0, 0)), ((1, 0), (0, 0))])])
        assert gs.len() == 2**220 - 2


if __name__ == '__main__':
    TestSetset().run()
    TestGraphset().run()
    print __file__, 'ok'
