import _illion

class setset(_illion.setset):
    def optimize(self, weights):
        it = super(setset, self).optimize(weights)
        while (True):
            yield it.next()

#class graphset(setset):
#    pass
