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
from graphillion.universe import ObjectTable
import _graphillion

class setset(setset_base):
    """Represents and manipulates a set of sets.

    A setset_base object stores a set of sets.  A set element can be any
    hashable object like a number, a text string, and a tuple.

    Like Python set types, setset_base supports `set in setset`,
    `len(setset)`, and `for set in setset`.  It also supports all set
    methods and operators,
    * isdisjoint(), issubset(), issuperset(), union(), intersection(),
      difference(), symmetric_difference(), copy(), update(),
      intersection_update(), difference_update(),
      symmetric_difference_update(), add(), remove(), discard(),
      pop(), clear(),
    * ==, !=, <=, <, >=, >, |, &, -, ^, |=, &=, -=, ^=.

    Examples:
      >>> from graphillion import setset
      >>> ss = setset([set([1]), set([1,2])])
      >>> len(ss)
      2
      >>> for s in ss:
      ...   s
      set([1])
      set([1, 2])
    """

    def __init__(self, setset_or_constraints=None):
        setset_base.__init__(self, setset._objtable, setset_or_constraints)

    def __repr__(self):
        return setset_base.__repr__(self, setset._objtable)

    def _repr(self, outer_braces=('[', ']'), inner_braces=('[', ']'), obj_to_str=None):
        return setset_base._repr(self, setset._objtable, outer_braces, inner_braces, obj_to_str)

    def __contains__(self, set_or_elem):
        return setset_base._contains(self, setset._objtable, set_or_elem)

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

    def add_some_element(self):
        return setset_base.add_some_element(self, setset._objtable)

    def remove_add_some_elements(self):
        return setset_base.remove_add_some_elements(self, setset._objtable)

    def to_vertexsetset(self):
        return setset_base.to_vertexsetset(self, setset._objtable)

    def to_edgevertexsetset(self):
        return setset_base.to_edgevertexsetset(self, setset._objtable)

    #@staticmethod
    #def get_vertices_from_top():
    #    return setset_base.get_vertices_from_top(setset._objtable)

    @staticmethod
    def load(fp):
        """Deserialize a file `fp` to `self`.

        This method does not deserialize the universe, which should be
        loaded separately by pickle.

        Args:
          fp: A read-supporting file-like object.

        Examples of dump():
          >>> import pickle
          >>> fp = open('/path/to/setset_base', 'wb')
          >>> ss.dump(fp)
          >>> fp = open('/path/to/universe' 'wb')
          >>> pickle.dump(setset_base.universe(), fp)

        Examples of load():
          >>> import pickle
          >>> fp = open('/path/to/universe')
          >>> setset_base.set_universe(pickle.load(fp))
          >>> fp = open('/path/to/setset_base')
          >>> ss = setset_base.load(fp)

        See Also:
          loads()
        """
        return setset(_graphillion.load(fp))

    @staticmethod
    def loads(s):
        """Deserialize `s` to `self`.

        This method does not deserialize the universe, which should be
        loaded separately by pickle.

        Args:
          s: A string instance.

        Examples of dump():
          >>> import pickle
          >>> setset_str = ss.dumps()
          >>> universe_str = pickle.dumps(setset_base.universe())

        Examples of load():
          >>> import pickle
          >>> setset_base.set_universe(pickle.loads(universe_str))
          >>> ss = setset_base.load(graphset_str)

        See Also:
          load()
        """
        return setset(_graphillion.loads(s))

    @staticmethod
    def set_universe(universe):
        if len(universe) != len(set(universe)):
            raise ValueError('duplicated elements found')
        setset._objtable = ObjectTable()
        for e in universe:
            setset._objtable.add_elem(e)
        setset._objtable.check_universe()

    @staticmethod
    def universe():
        setset._objtable.check_universe()
        return setset._objtable.int2obj[1:]

    _objtable = ObjectTable()
