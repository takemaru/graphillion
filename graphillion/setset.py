# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

"""Module for a set of sets.
"""

from graphillion.setset_base import setset_base
from graphillion.setset_base import ObjectTable
import _graphillion

class setset(setset_base):

    _objtable = ObjectTable()

    def __init__(self, setset_or_constraints=None):
        setset_base.__init__(self, setset._objtable, setset_or_constraints)

    def __repr__(self):
        return setset_base.__repr__(self, setset._objtable)

    def _repr(self, outer_braces=('[', ']'), inner_braces=('[', ']'), obj_to_str=None):
        return setset_base._repr(self, setset._objtable, outer_braces, inner_braces, obj_to_str)

    def __contains__(self, set_or_elem):
        return setset_base.__contains__(self, setset._objtable, set_or_elem)

    def __invert__(self):
        return setset_base._invert(self, setset._objtable)

    def add(self, set_or_elem):
        return setset_base.add(self, setset._objtable, set_or_elem)

    def remove(self, set_or_elem):
        return setset_base.remove(self, setset._objtable, set_or_elem)

    def discard(self, set_or_elem):
        return setset_base.discard(self, setset._objtable, set_or_elem)

    def pop(self):
        return setset_base.pop(self, setset._objtable)

    def hitting(self):
        return setset_base.hitting(self, setset._objtable)

    def flip(self, elem=None):
        return setset_base.flip(self, setset._objtable, elem)

    def __iter__(self):
        return setset_base._iter(self, setset._objtable)

    def rand_iter(self):
        return setset_base.rand_iter(self, setset._objtable)

    def min_iter(self, weights=None, default=1):
        return setset_base.min_iter(self, setset._objtable, weights, default)

    def max_iter(self, weights=None, default=1):
        return setset_base.max_iter(self, setset._objtable, weights, default)

    def supersets(self, obj):
        return setset_base.supersets(self, setset._objtable, obj)

    def non_supersets(self, obj):
        return setset_base.non_supersets(self, setset._objtable, obj)

    def choice(self):
        return setset_base.choice(self, setset._objtable)

    def probability(self, probabilities):
        return setset_base.probability(self, setset._objtable, probabilities)

    def cost_le(self, costs, cost_bound):
        return setset_base.cost_le(self, setset._objtable, costs, cost_bound)

    def add_some_element(self, num_elems = None):
        return setset_base.add_some_element(self, setset._objtable, num_elems)

    def remove_add_some_elements(self, num_elems = None):
        return setset_base.remove_add_some_elements(self, setset._objtable, num_elems)

    def to_vertexsetset(self):
        return setset_base.to_vertexsetset(self, setset._objtable)

    @staticmethod
    def get_vertices_from_top():
        return setset_base.get_vertices_from_top(setset._objtable)

    @staticmethod
    def load(fp):
        return setset(_graphillion.load(fp))

    @staticmethod
    def loads(s):
        return setset(_graphillion.loads(s))

    @staticmethod
    def set_universe(universe):
        if len(universe) != len(set(universe)):
            raise ValueError('duplicated elements found')
        _graphillion._num_elems(0)
        setset._objtable = ObjectTable()
        for e in universe:
            setset._objtable.add_elem(e)
        setset._objtable.check_universe()

        setset_base.set_universe(universe)

    @staticmethod
    def universe():
        setset._objtable.check_universe()
        return setset._objtable.int2obj[1:]
