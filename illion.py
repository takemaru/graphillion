import _illion

class setset(_illion.setset):
    def __init__(self, *arg):
        self._init(*arg);

    def optimize(self, weights):
        i = self.opt_iter(weights)
        while (True):
            yield i.next()

#class graphset(setset):
#    pass
