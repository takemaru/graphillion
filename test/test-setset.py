from illion import setset

def iterators():
    ss = setset([set(), set([1]), set([1, 2]), set([1, 2, 3]), set([1, 2, 3, 4]),
                 set([1, 3, 4]), set([1, 4]), set([4])])
    r = []
    for s in ss.optimize([0, 3, -2, -2, 4]):  # 1-offset list
        r.append(s)
    assert len(r) == 8
    assert r[0] == set([1, 4])
    assert r[1] == set([1, 3, 4])
    assert r[2] == set([4])

if __name__ == '__main__':
    iterators()
    print __file__, 'ok'
