from illion import setset

def init():
    assert not setset.INT_ONLY
    assert setset.obj2int == {} and setset.int2obj  == [None]
    assert setset.universe() == []

def constructors():
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

def iterators():
    ss1 = setset([set(), set(['1', '2']), set(['1', '3'])])
    ss2 = setset()
    for s in ss1:
        ss2 = ss2 | setset(s)
    assert ss1 == ss2

    ss2 = setset()
    for s in ss1:
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

def lookup():
    ss1 = setset([set(), set(['1', '2']), set(['1', '3'])])
    assert set(['1', '2']) in ss1
    assert set(['1']) not in ss1

    ss2 = ss1.find('1')
    assert isinstance(ss2, setset)
    assert ss2 == setset([set(['1', '2']), set(['1', '3'])])

    ss2 = ss1.not_find('2')
    assert isinstance(ss2, setset)
    assert ss2 == setset([set(), set(['1', '3'])])

def modifiers():
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
#    assert s not in ss
    assert ss | setset(s) == setset(v)

if __name__ == '__main__':
    name = 'setset'

    init()
    constructors()
#    iterators()
    lookup()
    modifiers()
    print __file__, 'ok'
