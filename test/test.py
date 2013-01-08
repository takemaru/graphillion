from illion import setset

def constructors():
    ss = setset()
#    print ss
    assert(ss.dumps() == '{}');

    ss = setset(set())
    assert(ss.dumps() == '{{}}');

    ss = setset(frozenset([1, 2]))
    assert(ss.dumps() == '{{1,2}}');

    ss = setset([set(), set([1, 2]), set([1, 3])])
    assert(ss.dumps() == '{{1,2},{1,3},{}}');

    ss = setset({'include': set([1, 2]), 'exclude': set([4])})
    assert(ss.dumps() == '{{1,2,3},{1,2}}');

    # TODO:
    #ss = setset([{'include': set([1, 2]), 'exclude': set([4])},
    #             {'include': set([1, 3, 4])},
    #             {'exclude': set([2, 3])}])
    #assert(ss.dumps() == e0 + e1 + e1*e2 + e1*e2*e3 + e1*e2*e3*e4 + e1*e3*e4 + e1*e4 + e4);

    # copy constructor
    ss = setset(setset([set([1]), set([2])]))
    assert(ss.dumps() == '{{1},{2}}');

def comparison():
    ss = setset(set([1, 2]))
    assert(ss == setset(set([1, 2])))
    assert(ss != setset(set([1, 3])))

    v = [set(), set([1, 2]), set([1, 3])]
    ss = setset(v)
    assert(ss.isdisjoint(setset([set([1]), set([1, 2, 3])])))
    assert(not ss.isdisjoint(setset([set([1]), set([1, 2])])))

    assert(ss.issubset(setset(v)));
    assert(not ss.issubset(setset([set(), set([1, 2])])))
    assert(ss <= setset(v))
    assert(not (ss <= setset([set(), set([1, 2])])))
    assert(ss < setset([set(), set([1]), set([1, 2]), set([1, 3])]))
    assert(not (ss < setset(v)))

    assert(ss.issuperset(setset(v)));
    assert(not ss.issuperset(setset([set([1]), set([1, 2])])))
    assert(ss >= setset(v))
    assert(not (ss >= setset([set([1]), set([1, 2])])))
    assert(ss > setset([set(), set([1, 2])]))
    assert(not (ss > setset(v)))

def unary_operators():
    ss = setset([set(), set([1]), set([1, 2]), set([1, 2, 3]), set([1, 2, 3, 4]),
                 set([1, 3, 4]), set([1, 4]), set([4])])
    assert(~ss == setset([set([1, 2, 4]), set([1, 3]), set([2]), set([2, 3]),
                          set([2, 3, 4]), set([2, 4]), set([3]), set([3, 4])]))

if __name__ == '__main__':
    constructors()
    comparison()
    unary_operators()
    print 'ok'
