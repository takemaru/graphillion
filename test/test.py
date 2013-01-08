from illion import setset

ss = setset()
print ss
ss.dump()

ss = setset(set())
ss.dump()

ss = setset(frozenset([1, 2]))
ss.dump()

ss = setset([set(), set([1, 2]), set([1, 3])])
ss.dump()

ss = setset({"include": set([1, 2]), "exclude": set([4])})
ss.dump()

#ss = setset([{"include": set([1, 2]), "exclude": set([4])},
#             {"include": set([1, 3, 4])},
#             {"exclude": set([2, 3])}])
#ss.dump()

ss = setset(set([1, 2]))
assert(ss == setset(set([1, 2])))
assert(ss != setset(set([1, 3])))

v = [set(), set([1, 2]), set([1, 3])]
ss = setset(v)
assert(ss <= setset(v))
assert(not (ss <= setset([set(), set([1, 2])])))
assert(ss < setset([set(), set([1]), set([1, 2]), set([1, 3])]))
assert(not (ss < setset(v)))

assert(ss >= setset(v))
assert(not (ss >= setset([set([1]), set([1, 2])])))
assert(ss > setset([set(), set([1, 2])]))
assert(not (ss > setset(v)))

ss = setset([set(), set([1]), set([1, 2]), set([1, 2, 3]), set([1, 2, 3, 4]),
             set([1, 3, 4]), set([1, 4]), set([4])])
assert(~ss == setset([set([1, 2, 4]), set([1, 3]), set([2]), set([2, 3]),
                      set([2, 3, 4]), set([2, 4]), set([3]), set([3, 4])]))

print 'ok'
