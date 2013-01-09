import _illion

class setset(_illion.setset):
    def optimize(self, weights):
        i = self.opt_iter(weights)
        while (True):
            yield i.next()

#class graphset(setset):
#    pass
