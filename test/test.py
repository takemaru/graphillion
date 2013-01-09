import _illion
from illion import setset

def constructors():
    ss = setset()
    assert repr(ss) == '<setset object of 0x8000000000>'
    assert ss._enums() == '{}'

    ss = setset(set())
    assert ss._enums() == '{{}}'

    ss = setset(frozenset([1, 2]))
    assert ss._enums() == '{{1,2}}'

    ss = setset([set(), set([1, 2]), set([1, 3])])
    assert ss._enums() == '{{1,2},{1,3},{}}'

    ss = setset({'include': set([1, 2]), 'exclude': set([4])})
    assert ss._enums() == '{{1,2,3},{1,2}}'

    ss = setset([{'include': set([1, 2]), 'exclude': set([4])},
                 {'include': set([1, 3, 4])},
                 {'exclude': set([2, 3])}])
    assert ss._enums() ==  '{{1,2,3,4},{1,2,3},{1,2},{1,3,4},{1,4},{1},{4},{}}'

    # copy constructor
    ss = setset([set(), set([1, 2]), set([1, 3])])
    assert ss._enums() == '{{1,2},{1,3},{}}'

    ss1 = setset([set(), set([1, 2]), set([1, 3])])
    ss2 = ss1.copy()
    ss1.clear()
    assert ss1._enums() == '{}'
    assert ss2._enums() == '{{1,2},{1,3},{}}'

def comparison():
    ss = setset(set([1, 2]))
    assert ss == setset(set([1, 2]))
    assert ss != setset(set([1, 3]))

    v = [set(), set([1, 2]), set([1, 3])]
    ss = setset(v)
    assert ss.isdisjoint(setset([set([1]), set([1, 2, 3])]))
    assert not ss.isdisjoint(setset([set([1]), set([1, 2])]))

    assert ss.issubset(setset(v))
    assert not ss.issubset(setset([set(), set([1, 2])]))
    assert ss <= setset(v)
    assert not (ss <= setset([set(), set([1, 2])]))
    assert ss < setset([set(), set([1]), set([1, 2]), set([1, 3])])
    assert not (ss < setset(v))

    assert ss.issuperset(setset(v))
    assert not ss.issuperset(setset([set([1]), set([1, 2])]))
    assert ss >= setset(v)
    assert not (ss >= setset([set([1]), set([1, 2])]))
    assert ss > setset([set(), set([1, 2])])
    assert not (ss > setset(v))

def unary_operators():
    ss = setset([set(), set([1]), set([1, 2]), set([1, 2, 3]), set([1, 2, 3, 4]),
                 set([1, 3, 4]), set([1, 4]), set([4])])
    assert ~ss == setset([set([1, 2, 4]), set([1, 3]), set([2]), set([2, 3]),
                          set([2, 3, 4]), set([2, 4]), set([3]), set([3, 4])])
    assert ss.complement() == setset([set([1, 2, 4]), set([1, 3]), set([2]),
                                      set([2, 3]), set([2, 3, 4]), set([2, 4]),
                                      set([3]), set([3, 4])])
    assert ss.smaller(2) == setset([set(), set([1]), set([1, 2]), set([1, 4]),
                                    set([4])])

    ss = setset([set([1, 2]), set([1, 4]), set([2, 3]), set([3, 4])])
    assert ss.hitting() == setset([set([1, 2, 3]), set([1, 2, 3, 4]),
                                   set([1, 2, 4]), set([1, 3]), set([1, 3, 4]),
                                   set([2, 3, 4]), set([2, 4])])

    ss = setset([set([1, 2]), set([1, 2, 3]), set([1, 2, 3, 4]), set([2, 4, 5])])
    assert ss.minimal() == setset([set([1, 2]), set([2, 4, 5])])
    assert ss.maximal() == setset([set([1, 2, 3, 4]), set([2, 4, 5])])

def binary_operators():
    u = [set(), set([1]), set([1, 2]), set([1, 2, 3]), set([1, 2, 3, 4]),
         set([1, 3, 4]), set([1, 4]), set([4])]
    v = [set([1, 2]), set([1, 4]), set([2, 3]), set([3, 4])]
    ss = setset(u) & setset(v)
    assert ss == setset([set([1, 2]), set([1, 4])])
    ss = setset(u).intersection(setset(v))
    assert ss == setset([set([1, 2]), set([1, 4])])

    ss = setset(u)
    ss &= setset(v)
    assert ss == setset([set([1, 2]), set([1, 4])])
    ss = setset(u)
    ss.intersection_update(setset(v))
    assert ss == setset([set([1, 2]), set([1, 4])])

    ss = setset(u) | setset(v)
    assert ss == setset([set(), set([1]), set([1, 2]), set([1, 2, 3]),
                         set([1, 2, 3, 4]), set([1, 3, 4]), set([1, 4]),
                         set([2, 3]), set([3, 4]), set([4])])
    ss = setset(u).union(setset(v))
    assert ss == setset([set(), set([1]), set([1, 2]), set([1, 2, 3]),
                         set([1, 2, 3, 4]), set([1, 3, 4]), set([1, 4]),
                         set([2, 3]), set([3, 4]), set([4])])

    ss = setset(u)
    ss |= setset(v)
    assert ss == setset([set(), set([1]), set([1, 2]), set([1, 2, 3]),
                         set([1, 2, 3, 4]), set([1, 3, 4]), set([1, 4]),
                         set([2, 3]), set([3, 4]), set([4])])
    ss = setset(u)
    ss.update(setset(v))
    assert ss == setset([set(), set([1]), set([1, 2]), set([1, 2, 3]),
                         set([1, 2, 3, 4]), set([1, 3, 4]), set([1, 4]),
                         set([2, 3]), set([3, 4]), set([4])])

    ss = setset(u) - setset(v)
    assert ss == setset([set(), set([1]), set([1, 2, 3]), set([1, 2, 3, 4]),
                         set([1, 3, 4]), set([4])])
    ss = setset(u).difference(setset(v))
    assert ss == setset([set(), set([1]), set([1, 2, 3]), set([1, 2, 3, 4]),
                         set([1, 3, 4]), set([4])])

    ss = setset(u)
    ss -= setset(v)
    assert ss == setset([set(), set([1]), set([1, 2, 3]), set([1, 2, 3, 4]),
                         set([1, 3, 4]), set([4])])
    ss = setset(u)
    ss.difference_update(setset(v))
    assert ss == setset([set(), set([1]), set([1, 2, 3]), set([1, 2, 3, 4]),
                         set([1, 3, 4]), set([4])])

    ss = setset(u) * setset(v)
    assert ss == setset([set([1, 2]), set([1, 2, 3]), set([1, 2, 4]),
                         set([1, 2, 3, 4]), set([1, 3, 4]), set([1, 4]),
                         set([2, 3]), set([2, 3, 4]), set([3, 4])])
    ss = setset(u).product(setset(v))
    assert ss == setset([set([1, 2]), set([1, 2, 3]), set([1, 2, 4]),
                         set([1, 2, 3, 4]), set([1, 3, 4]), set([1, 4]),
                         set([2, 3]), set([2, 3, 4]), set([3, 4])])

    ss = setset(u)
    ss *= setset(v)
    assert ss == setset([set([1, 2]), set([1, 2, 3]), set([1, 2, 4]),
                         set([1, 2, 3, 4]), set([1, 3, 4]), set([1, 4]),
                         set([2, 3]), set([2, 3, 4]), set([3, 4])])
    ss = setset(u)
    ss.product_update(setset(v))
    assert ss == setset([set([1, 2]), set([1, 2, 3]), set([1, 2, 4]),
                         set([1, 2, 3, 4]), set([1, 3, 4]), set([1, 4]),
                         set([2, 3]), set([2, 3, 4]), set([3, 4])])

    ss = setset(u) ^ setset(v)
    assert ss == setset([set(), set([1]), set([1, 2, 3]), set([1, 2, 3, 4]),
                         set([1, 3, 4]), set([2, 3]), set([3, 4]), set([4])])
    ss = setset(u).symmetric_difference(setset(v))
    assert ss == setset([set(), set([1]), set([1, 2, 3]), set([1, 2, 3, 4]),
                         set([1, 3, 4]), set([2, 3]), set([3, 4]), set([4])])

    ss = setset(u)
    ss ^= setset(v)
    assert ss == setset([set(), set([1]), set([1, 2, 3]), set([1, 2, 3, 4]),
                         set([1, 3, 4]), set([2, 3]), set([3, 4]), set([4])])
    ss = setset(u)
    ss.symmetric_difference_update(setset(v))
    assert ss == setset([set(), set([1]), set([1, 2, 3]), set([1, 2, 3, 4]),
                         set([1, 3, 4]), set([2, 3]), set([3, 4]), set([4])])

    v = [set([1, 2])]
    ss = setset(u) / setset(v)
    assert ss == setset([set(), set([3]), set([3, 4])])
    ss = setset(u).divide(setset(v))
    assert ss == setset([set(), set([3]), set([3, 4])])

    ss = setset(u)
    ss /= setset(v)
    assert ss == setset([set(), set([3]), set([3, 4])])
    ss = setset(u)
    ss.divide_update(setset(v))
    assert ss == setset([set(), set([3]), set([3, 4])])

    ss = setset(u) % setset(v)
    assert ss == setset([set(), set([1]), set([1, 3, 4]), set([1, 4]), set([4])])
    ss = setset(u).remainder(setset(v))
    assert ss == setset([set(), set([1]), set([1, 3, 4]), set([1, 4]), set([4])])

    ss = setset(u)
    ss %= setset(v)
    assert ss == setset([set(), set([1]), set([1, 3, 4]), set([1, 4]), set([4])])
    ss = setset(u)
    ss.remainder_update(setset(v))
    assert ss == setset([set(), set([1]), set([1, 3, 4]), set([1, 4]), set([4])])

    v = [set([1, 2]), set([1, 4]), set([2, 3]), set([3, 4])]
    ss = setset(u).subsets(setset(v))
    assert ss == setset([set(), set([1]), set([1, 2]), set([1, 4]), set([4])])

    ss = setset(u).supersets(setset(v))
    assert ss == setset([set([1, 2]), set([1, 2, 3]), set([1, 2, 3, 4]),
                         set([1, 3, 4]), set([1, 4])])

    ss = setset(u).nonsubsets(setset(v))
    assert ss == setset([set([1, 2, 3]), set([1, 2, 3, 4]), set([1, 3, 4])])

    ss = setset(u).nonsupersets(setset(v))
    assert ss == setset([set(), set([1]), set([4])])

def capacity():
    ss = setset()
    assert not ss

    ss = setset([set(), set([1, 2]), set([1, 3])])
    assert not not ss

    assert len(ss) == 3
    assert ss.len() == 3

def iterators():
    ss1 = setset([set(), set([1, 2]), set([1, 3])])
    ss2 = setset()
    for s in ss1:
        ss2 = ss2 | setset(s)
    assert ss1 == ss2

    ss2.clear()
    for s in ss1:
        ss2 = ss2 | setset(s)
    assert ss1 == ss2

    ss = setset([set(), set([1]), set([1, 2]), set([1, 2, 3]), set([1, 2, 3, 4]),
                 set([1, 3, 4]), set([1, 4]), set([4])])
    r = []
    for s in ss.optimize([0, 3, -2, -2, 4]):  # 1-offset list
        r.append(s)
    assert len(r) == 8
    assert r[0] == set([1, 4])
    assert r[1] == set([1, 3, 4])
    assert r[2] == set([4])

def lookup():
    ss = setset([set(), set([1, 2]), set([1, 3])])
    assert set([1, 2]) in ss
    assert set([1]) not in ss

    assert ss.find(1) == setset([set([1, 2]), set([1, 3])])

    assert ss.not_find(2) == setset([set(), set([1, 3])])

def modifiers():
    v = [set(), set([1, 2]), set([1, 3])]
    ss = setset(v)
    ss.add(set([1]))
    assert set([1]) in ss

    ss.remove(set([1]))
    assert set([1]) not in ss

    try:
        ss.remove(set([1]))
    except KeyError:
        pass
    else:
        assert False

    ss.add(set([1]))
    ss.discard(set([1]))
    assert set([1]) not in ss
    ss.discard(set([1]))  # no exception raised

    s = ss.pop()
    assert s not in ss
    assert ss | setset(s) == setset([set(), set([1, 2]), set([1, 3])])

    assert ss
    ss.clear()
    assert not ss

def stream():
    ss = setset()
    str = ss.dumps()
    assert str == "B\n.\n"
    ss.loads(str)
    assert ss == setset()

    ss = setset(set())
    str = ss.dumps()
    assert str == "T\n.\n"
    ss.loads(str)
    assert ss == setset(set())

    v = [set(), set([1]), set([1, 2]), set([1, 2, 3]), set([1, 2, 3, 4]),
         set([1, 3, 4]), set([1, 4]), set([4])]
    ss = setset(v)
    str = ss.dumps()
    ss = setset()
    ss.loads(str)
    assert ss == setset(v)

    ss = setset(str)
    assert ss == setset(v)

    ss.dump(open('/tmp/illion_', 'w'))
    ss = setset()
    ss.load(open('/tmp/illion_'))
    assert ss == setset(v)

if __name__ == '__main__':
    constructors()
    comparison()
    unary_operators()
    binary_operators()
    capacity()
    iterators()
    lookup()
    modifiers()
    stream()
    print 'ok'
