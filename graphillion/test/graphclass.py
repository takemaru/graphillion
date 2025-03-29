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

from builtins import range
from graphillion import GraphSet
from graphillion import GraphClass
import unittest

# Tests for some graph classes are under construction

e1 = (1, 2)
e2 = (1, 3)
e3 = (2, 3)
e4 = (2, 4)
e5 = (3, 4)
e6 = (3, 5)
e7 = (4, 5)

g0 = []
g1 = [e1]
g2 = [e2]
g3 = [e3]
g4 = [e4]
g12 = [e1, e2]
g13 = [e1, e3]
g14 = [e1, e4]
g23 = [e2, e3]
g24 = [e2, e4]
g34 = [e3, e4]
g123 = [e1, e2, e3]
g134 = [e1, e3, e4]
g234 = [e2, e3, e4]
g345 = [e3, e4, e5]
g1234 = [e1, e2, e3, e4]
g2345 = [e2, e3, e4, e5]
g12345 = [e1, e2, e3, e4, e5]


class TestGraphClass(unittest.TestCase):

    def setUp(self):
        # Simple graph with potential for various structures
        GraphSet.set_universe([e1, e2, e3, e4, e5, e6, e7])

    def tearDown(self):
        pass

    def test_claw_graphs(self):
        """Test for claw graphs (K_{1,3})."""
        gs = GraphClass.claw_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # A claw graph has one vertex with degree 3 and three vertices with degree 1
        # Example: vertex 2 connected to vertices 1, 3, and 4
        claw = [(1, 2), (2, 3), (2, 4)]
        self.assertTrue(claw in gs)

        # This is not a claw graph (forms a cycle)
        non_claw = [(1, 2), (2, 3), (3, 1)]
        self.assertFalse(non_claw in gs)

    def test_claw_free_graphs(self):
        """Test for claw-free graphs."""
        gs = GraphClass.claw_free_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # A claw graph shouldn't be in the result
        claw = [(1, 2), (2, 3), (2, 4)]
        self.assertFalse(claw in gs)

        # A path of length 3 should be in the result (is claw-free)
        path = [(1, 2), (2, 3), (3, 4)]
        self.assertTrue(path in gs)

        # A cycle is claw-free
        cycle = [(1, 2), (2, 3), (3, 1)]
        self.assertTrue(cycle in gs)

    def test_diamond_graphs(self):
        """Test for diamond graphs (K_4 minus one edge)."""
        gs = GraphClass.diamond_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A diamond has 4 vertices, 5 edges, and consists of two vertices with degree 3 and two with degree 2
        # # Example: Complete graph on vertices 1,2,3,4 without edge (1,4)
        # diamond = [(1, 2), (1, 3), (2, 3), (2, 4), (3, 4)]
        # self.assertTrue(diamond in gs)

        # # A tetrahedron (K_4) is not a diamond
        # tetrahedron = [(1, 2), (1, 3), (1, 4), (2, 3), (2, 4), (3, 4)]
        # self.assertFalse(tetrahedron in gs)

    def test_diamond_free_graphs(self):
        """Test for diamond-free graphs."""
        gs = GraphClass.diamond_free_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A diamond graph shouldn't be in the result
        # diamond = [(1, 2), (1, 3), (2, 3), (2, 4), (3, 4)]
        # self.assertFalse(diamond in gs)

        # # A tree is diamond-free
        # tree = [(1, 2), (2, 3), (3, 4), (3, 5)]
        # self.assertTrue(tree in gs)

        # # A cycle is diamond-free
        # cycle = [(1, 2), (2, 3), (3, 4), (4, 1)]
        # self.assertTrue(cycle in gs)

    def test_gem_graphs(self):
        """Test for gem graphs."""
        gs = GraphClass.gem_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A gem has 5 vertices, 8 edges, with a path of 4 vertices plus a vertex connected to all path vertices
        # gem = [(1, 2), (2, 3), (3, 4), (1, 5), (2, 5), (3, 5), (4, 5)]
        # self.assertTrue(gem in gs)

        # # A cycle is not a gem
        # cycle = [(1, 2), (2, 3), (3, 4), (4, 1)]
        # self.assertFalse(cycle in gs)

    def test_gem_free_graphs(self):
        """Test for gem-free graphs."""
        gs = GraphClass.gem_free_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A gem graph shouldn't be in the result
        # gem = [(1, 2), (2, 3), (3, 4), (1, 5), (2, 5), (3, 5), (4, 5)]
        # self.assertFalse(gem in gs)

        # # A path is gem-free
        # path = [(1, 2), (2, 3), (3, 4), (4, 5)]
        # self.assertTrue(path in gs)

        # # A cycle is gem-free
        # cycle = [(1, 2), (2, 3), (3, 4), (4, 1)]
        # self.assertTrue(cycle in gs)

    def test_odd_hole_graphs(self):
        """Test for odd hole graphs (cycles of odd length ≥ 5)."""
        gs = GraphClass.odd_hole_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A cycle of length 5 is an odd hole
        # cycle5 = [(1, 2), (2, 3), (3, 4), (4, 5), (5, 1)]
        # self.assertTrue(cycle5 in gs)

        # # A cycle of length 4 is not an odd hole
        # cycle4 = [(1, 2), (2, 3), (3, 4), (4, 1)]
        # self.assertFalse(cycle4 in gs)

        # # A cycle of length 3 is not an odd hole (too small)
        # cycle3 = [(1, 2), (2, 3), (3, 1)]
        # self.assertFalse(cycle3 in gs)

    def test_odd_hole_free_graphs(self):
        """Test for odd-hole-free graphs."""
        gs = GraphClass.odd_hole_free_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # An odd hole shouldn't be in the result
        # cycle5 = [(1, 2), (2, 3), (3, 4), (4, 5), (5, 1)]
        # self.assertFalse(cycle5 in gs)

        # # A cycle of length 4 is odd-hole-free
        # cycle4 = [(1, 2), (2, 3), (3, 4), (4, 1)]
        # self.assertTrue(cycle4 in gs)

        # # A tree is odd-hole-free
        # tree = [(1, 2), (2, 3), (3, 4), (3, 5)]
        # self.assertTrue(tree in gs)

    def test_chordal_graphs(self):
        """Test for chordal graphs."""
        gs = GraphClass.chordal_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A tree is chordal
        # tree = [(1, 2), (2, 3), (3, 4), (3, 5)]
        # self.assertTrue(tree in gs)

        # # A cycle of length 3 is chordal
        # cycle3 = [(1, 2), (2, 3), (3, 1)]
        # self.assertTrue(cycle3 in gs)

        # # A cycle of length 4 or more is not chordal (unless it has chords)
        # cycle4 = [(1, 2), (2, 3), (3, 4), (4, 1)]
        # self.assertFalse(cycle4 in gs)

        # # A cycle of length 4 with a chord is chordal
        # cycle4_with_chord = [(1, 2), (2, 3), (3, 4), (4, 1), (1, 3)]
        # self.assertTrue(cycle4_with_chord in gs)

    def test_cographs(self):
        """Test for cographs (P4-free graphs)."""
        gs = GraphClass.cographs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A path of length 3 (P4) should not be in the result
        # p4 = [(1, 2), (2, 3), (3, 4)]
        # self.assertFalse(p4 in gs)

        # # A star is a cograph
        # star = [(1, 2), (1, 3), (1, 4)]
        # self.assertTrue(star in gs)

        # # A complete graph is a cograph
        # complete3 = [(1, 2), (2, 3), (3, 1)]
        # self.assertTrue(complete3 in gs)

    def test_chordal_bipartite_graphs(self):
        """Test for chordal bipartite graphs."""
        gs = GraphClass.chordal_bipartite_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A tree is chordal bipartite
        # tree = [(1, 2), (2, 3), (3, 4)]
        # self.assertTrue(tree in gs)

        # # A cycle of length 4 is bipartite but not chordal bipartite (no chord possible in bipartite graph)
        # cycle4 = [(1, 2), (2, 3), (3, 4), (4, 1)]
        # self.assertFalse(cycle4 in gs)

        # # A cycle of length 6 without a chord is not chordal bipartite
        # cycle6 = [(1, 2), (2, 3), (3, 4), (4, 5), (5, 6), (6, 1)]
        # self.assertFalse(cycle6 in gs)

        # # A cycle of length 6 with a chord is chordal bipartite
        # cycle6_with_chord = [(1, 2), (2, 3), (3, 4), (4, 5), (5, 6), (6, 1), (1, 4)]
        # self.assertTrue(cycle6_with_chord in gs)

    def test_split_graphs(self):
        """Test for split graphs."""
        gs = GraphClass.split_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A complete graph is a split graph
        # complete3 = [(1, 2), (2, 3), (3, 1)]
        # self.assertTrue(complete3 in gs)

        # # A star is a split graph (center forms a clique, leaves form an independent set)
        # star = [(1, 2), (1, 3), (1, 4)]
        # self.assertTrue(star in gs)

        # # A cycle of length 4 is not a split graph
        # cycle4 = [(1, 2), (2, 3), (3, 4), (4, 1)]
        # self.assertFalse(cycle4 in gs)

        # # A cycle of length 5 is not a split graph
        # cycle5 = [(1, 2), (2, 3), (3, 4), (4, 5), (5, 1)]
        # self.assertFalse(cycle5 in gs)

        # # A disjoint pair of edges (2K2) is not a split graph
        # disjoint_edges = [(1, 2), (3, 4)]
        # self.assertFalse(disjoint_edges in gs)

    def test_block_graphs(self):
        """Test for block graphs."""
        gs = GraphClass.block_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A tree is a block graph
        # tree = [(1, 2), (2, 3), (3, 4), (3, 5)]
        # self.assertTrue(tree in gs)

        # # A complete graph is a block graph
        # complete3 = [(1, 2), (2, 3), (3, 1)]
        # self.assertTrue(complete3 in gs)

        # # A diamond is not a block graph
        # diamond = [(1, 2), (1, 3), (2, 3), (2, 4), (3, 4)]
        # self.assertFalse(diamond in gs)

        # # Two triangles sharing a vertex form a block graph
        # two_triangles = [(1, 2), (2, 3), (3, 1), (3, 4), (4, 5), (5, 3)]
        # self.assertTrue(two_triangles in gs)

    def test_ptolemaic_graphs(self):
        """Test for ptolemaic graphs."""
        gs = GraphClass.ptolemaic_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A tree is a ptolemaic graph
        # tree = [(1, 2), (2, 3), (3, 4), (3, 5)]
        # self.assertTrue(tree in gs)

        # # A complete graph is a ptolemaic graph
        # complete3 = [(1, 2), (2, 3), (3, 1)]
        # self.assertTrue(complete3 in gs)

        # # A gem is not a ptolemaic graph
        # gem = [(1, 2), (2, 3), (3, 4), (1, 5), (2, 5), (3, 5), (4, 5)]
        # self.assertFalse(gem in gs)

        # # A cycle of length 4 is not chordal, thus not ptolemaic
        # cycle4 = [(1, 2), (2, 3), (3, 4), (4, 1)]
        # self.assertFalse(cycle4 in gs)

    def test_threshold_graphs(self):
        """Test for threshold graphs."""
        gs = GraphClass.threshold_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A complete graph is a threshold graph
        # complete3 = [(1, 2), (2, 3), (3, 1)]
        # self.assertTrue(complete3 in gs)

        # # A star is a threshold graph
        # star = [(1, 2), (1, 3), (1, 4)]
        # self.assertTrue(star in gs)

        # # A path of length 3 (P4) is not a threshold graph
        # p4 = [(1, 2), (2, 3), (3, 4)]
        # self.assertFalse(p4 in gs)

        # # A cycle of length 4 is not a threshold graph
        # cycle4 = [(1, 2), (2, 3), (3, 4), (4, 1)]
        # self.assertFalse(cycle4 in gs)

        # # A disjoint pair of edges (2K2) is not a threshold graph
        # disjoint_edges = [(1, 2), (3, 4)]
        # self.assertFalse(disjoint_edges in gs)

    def test_gridline_graphs(self):
        """Test for gridline graphs."""
        gs = GraphClass.gridline_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A path is a gridline graph
        # path = [(1, 2), (2, 3), (3, 4)]
        # self.assertTrue(path in gs)

        # # A grid (partially-filled in) should be a gridline graph
        # grid = [(1, 2), (2, 3), (1, 4), (2, 5), (3, 6), (4, 5), (5, 6)]
        # self.assertTrue(grid in gs)

        # # A claw is not a gridline graph
        # claw = [(1, 2), (2, 3), (2, 4)]
        # self.assertFalse(claw in gs)

        # # A diamond is not a gridline graph
        # diamond = [(1, 2), (1, 3), (2, 3), (2, 4), (3, 4)]
        # self.assertFalse(diamond in gs)

    def test_domino_graphs(self):
        """Test for domino graphs."""
        gs = GraphClass.domino_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A domino is a 2x3 grid graph
        # domino = [(1, 2), (2, 3), (4, 5), (5, 6), (1, 4), (2, 5), (3, 6)]
        # self.assertTrue(domino in gs)

        # # A claw is not a domino graph
        # claw = [(1, 2), (2, 3), (2, 4)]
        # self.assertFalse(claw in gs)

        # # A gem is not a domino graph
        # gem = [(1, 2), (2, 3), (3, 4), (1, 5), (2, 5), (3, 5), (4, 5)]
        # self.assertFalse(gem in gs)

    def test_linear_domino_graphs(self):
        """Test for linear domino graphs."""
        gs = GraphClass.linear_domino_graphs()
        self.assertTrue(isinstance(gs, GraphSet))

        # # A path is a linear domino graph
        # path = [(1, 2), (2, 3), (3, 4)]
        # self.assertTrue(path in gs)

        # # A grid (partially-filled in) can be a linear domino graph
        # grid = [(1, 2), (2, 3), (1, 4), (2, 5), (3, 6), (4, 5), (5, 6)]
        # self.assertTrue(grid in gs)

        # # A claw is not a linear domino graph
        # claw = [(1, 2), (2, 3), (2, 4)]
        # self.assertFalse(claw in gs)

        # # A diamond is not a linear domino graph
        # diamond = [(1, 2), (1, 3), (2, 3), (2, 4), (3, 4)]
        # self.assertFalse(diamond in gs)


if __name__ == '__main__':
    unittest.main()
