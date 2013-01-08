import illion

ss = illion.setset()
print ss
ss.dump()

ss = illion.setset(set())
ss.dump()

ss = illion.setset(frozenset([1, 2]))
ss.dump()

ss = illion.setset([set(), set([1, 2]), set([1, 3])])
ss.dump()

ss = illion.setset({"include": set([1, 2]), "exclude": set([4])})
ss.dump()

#ss = illion.setset([{"include": set([1, 2]), "exclude": set([4])},
#                    {"include": set([1, 3, 4])},
#                    {"exclude": set([2, 3])}])
#ss.dump()

print 'ok'
