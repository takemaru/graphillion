from illion import setset

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

    def init(self):
        assert not setset.INT_ONLY
        assert setset.obj2int == {} and setset.int2obj  == [None]
        assert setset.universe() == []

    def constructors(self):
        name = 'setset'

        ss = setset()
        assert isinstance(ss, setset)
        assert repr(ss) == '<%s object of 0x8000000000>' % name
        assert ss._enums() == name + '([])'
        assert setset.obj2int == {} and setset.int2obj == [None]
        assert setset.universe() == []

        ss = setset(set())
        assert ss._enums() == name + '([set([])])'
        assert setset.obj2int == {} and setset.int2obj == [None]
        assert setset.universe() == []

        ss = setset(frozenset(['1', '2']))
        assert ss._enums() == name + '([set([1, 2])])'
        assert setset.obj2int == {'1': 1, '2': 2}
        assert setset.int2obj == [None, '1', '2']
        assert setset.universe() == ['1', '2']

        ss = setset([set(), set(['1', '2']), set(['1', '3'])])
        assert ss._enums() == name + '([set([1, 2]), set([1, 3]), set([])])'
        assert setset.obj2int == {'1': 1, '2': 2, '3': 3}
        assert setset.int2obj == [None, '1', '2', '3']
        assert setset.universe() == ['1', '2', '3']

        ss = setset({'include': set(['1', '2']), 'exclude': set(['4'])})
        assert ss._enums() == name + '([set([1, 2, 3]), set([1, 2])])'
        assert setset.obj2int == {'1': 1, '2': 2, '3': 3, '4': 4}
        assert setset.int2obj == [None, '1', '2', '3', '4']
        assert setset.universe() == ['1', '2', '3', '4']

        ss = setset([{'include': set(['1', '2']), 'exclude': set(['4'])},
                     {'include': set(['1', '3', '4'])},
                     {'exclude': set(['2', '3'])}])
        assert ss._enums() == (name + '([set([1, 2, 3, 4]), set([1, 2, 3]), '
                                        'set([1, 2]), set([1, 3, 4]), set([1, 4]), '
                                        'set([1]), set([4]), set([])])')

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
        ss = setset([set(), set(['1']), set(['1', '2']), set(['1', '2', '3']),
                     set(['1', '2', '3', '4']), set(['1', '3', '4']),
                     set(['1', '4']), set(['4'])])
        assert isinstance(~ss, setset)
        assert isinstance(ss.complement(), setset)
        assert isinstance(ss.smaller(2), setset)
        assert isinstance(ss.hitting(), setset)
        assert isinstance(ss.minimal(), setset)
        assert isinstance(ss.maximal(), setset)

        assert ~ss == setset([set(['1', '2', '4']), set(['1', '3']), set(['2']),
                              set(['2', '3']), set(['2', '3', '4']),
                              set(['2', '4']), set(['3']), set(['3', '4'])])
        assert ss.complement() == setset([set(['1', '2', '4']), set(['1', '3']),
                                          set(['2']), set(['2', '3']),
                                          set(['2', '3', '4']), set(['2', '4']),
                                          set(['3']), set(['3', '4'])])
        assert ss.smaller(2) == setset([set(), set(['1']), set(['1', '2']),
                                        set(['1', '4']), set(['4'])])

        ss = setset([set(['1', '2']), set(['1', '4']), set(['2', '3']),
                     set(['3', '4'])])
        assert ss.hitting() == setset([set(['1', '2', '3']),
                                       set(['1', '2', '3', '4']),
                                       set(['1', '2', '4']), set(['1', '3']),
                                       set(['1', '3', '4']),
                                       set(['2', '3', '4']), set(['2', '4'])])

        ss = setset([set(['1', '2']), set(['1', '2', '3']),
                     set(['1', '2', '3', '4']), set(['2', '4', '5'])])
        assert ss.minimal() == setset([set(['1', '2']), set(['2', '4', '5'])])
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

        ss = setset(u) * setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(['1', '2']), set(['1', '2', '3']),
                             set(['1', '2', '4']), set(['1', '2', '3', '4']),
                             set(['1', '3', '4']), set(['1', '4']),
                             set(['2', '3']), set(['2', '3', '4']),
                             set(['3', '4'])])
        ss = setset(u).product(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(['1', '2']), set(['1', '2', '3']),
                             set(['1', '2', '4']), set(['1', '2', '3', '4']),
                             set(['1', '3', '4']), set(['1', '4']),
                             set(['2', '3']), set(['2', '3', '4']),
                             set(['3', '4'])])

        ss = setset(u)
        ss *= setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(['1', '2']), set(['1', '2', '3']),
                             set(['1', '2', '4']), set(['1', '2', '3', '4']),
                             set(['1', '3', '4']), set(['1', '4']),
                             set(['2', '3']), set(['2', '3', '4']),
                             set(['3', '4'])])
        ss = setset(u)
        ss.product_update(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(['1', '2']), set(['1', '2', '3']),
                             set(['1', '2', '4']), set(['1', '2', '3', '4']),
                             set(['1', '3', '4']), set(['1', '4']),
                             set(['2', '3']), set(['2', '3', '4']),
                             set(['3', '4'])])

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
        ss = setset(u).divide(setset(v))
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['3']), set(['3', '4'])])

        ss = setset(u)
        ss /= setset(v)
        assert isinstance(ss, setset)
        assert ss == setset([set(), set(['3']), set(['3', '4'])])
        ss = setset(u)
        ss.divide_update(setset(v))
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
        for s in ss.optimize({'1': 3, '2': -2, '3': -2, '4': 4}):
            r.append(s)
        assert len(r) == 8
        assert r[0] == set(['1', '4'])
        assert r[1] == set(['1', '3', '4'])
        assert r[2] == set(['4'])

    def lookup(self):
        ss1 = setset([set(), set(['1', '2']), set(['1', '3'])])
        assert set(['1', '2']) in ss1
        assert set(['1']) not in ss1

        ss2 = ss1.find('1')
        assert isinstance(ss2, setset)
        assert ss2 == setset([set(['1', '2']), set(['1', '3'])])

        ss2 = ss1.not_find('2')
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

        # string is treated as an element in hook_arg()
#        ss = setset(st)
#        assert ss == setset(v)

        ss.dump(open('/tmp/illion_', 'w'))
        ss = setset()
        ss.load(open('/tmp/illion_'))
        assert ss == setset(v)

if __name__ == '__main__':
    TestSetset().run()
    print __file__, 'ok'
