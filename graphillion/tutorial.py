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

"""Helper functions for tutorial.
"""

from builtins import range
from graphillion import GraphSet
from random import seed, shuffle

def grid(m, n=None, prob_to_remove_edge=0.0):
    import networkx as nx
    # critical edge probability is 0.5 in the percolation theory
    assert 0 <= prob_to_remove_edge and prob_to_remove_edge < 0.4
    seed(1)
    m += 1
    if n is None:
        n = m
    else:
        n += 1
    edges = []
    for v in range(1, m * n + 1):
        if v % n != 0:
            edges.append((v, v + 1))
        if v <= (m - 1) * n:
            edges.append((v, v + n))
    g = nx.Graph(edges)
    while prob_to_remove_edge > 0:
        g = nx.Graph(edges)
        edges_removed = edges[:]
        shuffle(edges_removed)
        g.remove_edges_from(edges_removed[:int(len(edges)*prob_to_remove_edge)])
        if nx.is_connected(g) and len(g[1]) == 2:
            break
    return g.edges()

def draw(g, universe=None):
    import networkx as nx
    import matplotlib.pyplot as plt
    if not isinstance(g, nx.Graph):
        g = nx.Graph(list(g))
    if universe is None:
        universe = GraphSet.universe()
    if not isinstance(universe, nx.Graph):
        universe = nx.Graph(list(universe))
    n = sorted(universe[1].keys())[1] - 1
    m = universe.number_of_nodes() // n
    g.add_nodes_from(universe.nodes())
    pos = {}
    for v in range(1, m * n + 1):
        pos[v] = ((v - 1) % n, (m * n - v) // n)
    nx.draw(g, pos)
    plt.show()

def how_many_turns(path):
    path = set(path)
    turns = 0
    pos = 1
    direction = 1
    while (True):
        edges = [e for e in path if e[0] == pos or e[1] == pos]
        if not edges: break
        edge = edges[0]
        path -= set([edge])
        next_direction = abs(edge[1] - edge[0])
        if direction != next_direction:
            turns +=1
        pos = edge[1] if edge[0] == pos else edge[0]
        direction = next_direction
    return turns

def hist(data):
    import matplotlib.pyplot as plt
    plt.hist(data)
    plt.show()
