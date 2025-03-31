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

"""Module for constructing GraphSets that represent various graph classes

This module provides a collection of methods for generating various graph classes
that are recognized in graph theory. Each method returns a GraphSet containing
all subgraphs from the universal graph that satisfy the properties of the
specified graph class.

Graph classes are typically defined by forbidden structures (like specific
induced subgraphs) or by specific structural properties (like being chordal).
This module implements many common graph classes from graph theory literature.
"""

from graphillion.universe import Universe
from graphillion import GraphSet

class GraphClass:
    """Provides methods to create GraphSets of specific graph classes.

    This class contains static methods that generate various well-known graph
    classes as GraphSet objects. Most methods either generate the specific graph
    structure (like a claw graph) or create the set of all graphs that do not
    contain certain forbidden induced subgraphs.

    All methods return GraphSet objects containing graphs that satisfy the
    properties of the specified graph class.
    """

    @staticmethod
    def claw_graphs():
        """Returns a GraphSet containing all claw graphs.

        A claw graph is a graph isomorphic to K_{1,3}, i.e., a single vertex
        connected to three other vertices, with no other edges. It resembles
        a claw shape.

        Examples:
            >>> GraphClass.claw_graphs()
            GraphSet([...])  # All claw-shaped graphs

        Returns:
            A new GraphSet object containing all claw graphs.
        """
        deg_dist = {0: GraphSet.DegreeDistribution_Any, 1: 3, 3: 1}
        gs = GraphSet.degree_distribution_graphs(deg_dist, False)
        return gs

    @staticmethod
    def claw_free_graphs():
        """Returns a GraphSet of all claw-free graphs.

        A claw-free graph is a graph that does not contain any induced subgraph
        isomorphic to the claw graph (K_{1,3}).

        Examples:
            >>> GraphClass.claw_free_graphs()
            GraphSet([...])  # All graphs without claw as an induced subgraph

        Returns:
            A new GraphSet object containing all claw-free graphs.

        See Also:
            claw_graphs(): For the definition of claw graphs.
        """
        return GraphSet.forbidden_induced_subgraphs(GraphClass.claw_graphs())

    @staticmethod
    def diamond_graphs():
        """Returns a GraphSet containing all diamond graphs.

        A diamond graph is a graph obtained by removing one edge from a complete
        graph of 4 vertices (K_4). It consists of two vertices of degree 3 and
        two vertices of degree 2.

        Note:
            This implementation assumes the universe has neither parallel edges nor self-loops.

        Examples:
            >>> GraphClass.diamond_graphs()
            GraphSet([...])  # All diamond-shaped graphs

        Returns:
            A new GraphSet object containing all diamond graphs.
        """
        # assume that universe has neither parallel edges nor self-loop
        deg_dist = {0: GraphSet.DegreeDistribution_Any, 2: 2, 3: 2}
        gs = GraphSet.degree_distribution_graphs(deg_dist, False)
        return gs

    @staticmethod
    def diamond_free_graphs():
        """Returns a GraphSet of all diamond-free graphs.

        A diamond-free graph is a graph that does not contain any induced
        subgraph isomorphic to the diamond graph (K_4 minus one edge).

        Examples:
            >>> GraphClass.diamond_free_graphs()
            GraphSet([...])  # All graphs without diamond as an induced subgraph

        Returns:
            A new GraphSet object containing all diamond-free graphs.

        See Also:
            diamond_graphs(): For the definition of diamond graphs.

        Notes:
            See https://www.graphclasses.org/classes/gc_441.html for more information.
        """
        return GraphSet.forbidden_induced_subgraphs(GraphClass.diamond_graphs())

    @staticmethod
    def gem_graphs():
        """Returns a GraphSet containing all gem graphs.

        A gem graph is a graph consisting of a path of 4 vertices, plus
        a fifth vertex that is adjacent to all vertices of the path.
        It has one vertex of degree 4, two vertices of degree 3, and two
        vertices of degree 2.

        Note:
            This implementation assumes the universe has neither parallel edges nor self-loops.

        Examples:
            >>> GraphClass.gem_graphs()
            GraphSet([...])  # All gem-shaped graphs

        Returns:
            A new GraphSet object containing all gem graphs.
        """
        # assume that universe has neither parallel edges nor self-loop
        deg_dist = {0: GraphSet.DegreeDistribution_Any, 2: 2, 3: 2, 4: 1}
        gs = GraphSet.degree_distribution_graphs(deg_dist, False)
        return gs

    @staticmethod
    def gem_free_graphs():
        """Returns a GraphSet of all gem-free graphs.

        A gem-free graph is a graph that does not contain any induced subgraph
        isomorphic to the gem graph.

        Examples:
            >>> GraphClass.gem_free_graphs()
            GraphSet([...])  # All graphs without gem as an induced subgraph

        Returns:
            A new GraphSet object containing all gem-free graphs.

        See Also:
            gem_graphs(): For the definition of gem graphs.

        Notes:
            See https://www.graphclasses.org/classes/gc_354.html for more information.
        """
        return GraphSet.forbidden_induced_subgraphs(GraphClass.gem_graphs())

    @staticmethod
    def odd_hole_graphs():
        """Returns a GraphSet containing all odd hole graphs.

        An odd hole is an induced cycle of odd length at least 5. This method
        returns all induced cycles with odd length from 5 up to the number
        of vertices in the universe.

        Examples:
            >>> GraphClass.odd_hole_graphs()
            GraphSet([...])  # All odd hole graphs

        Returns:
            A new GraphSet object containing all odd hole graphs.

        Notes:
            See https://www.graphclasses.org/smallgraphs.html#odd_holes for more information.
        """
        univ = GraphSet.universe()
        vertices = set()
        for e in univ:
            vertices.add(e[0])
            vertices.add(e[1])
        n = len(vertices)

        # cycle with odd length at least 5
        dc = {}
        for v in Universe.vertices:
            dc[v] = range(0, 3, 2)
        return GraphSet.graphs(vertex_groups=[[]], degree_constraints=dc,
                               num_edges=range(5, n + 1, 2))

    @staticmethod
    def odd_hole_free_graphs():
        """Returns a GraphSet of all odd-hole-free graphs.

        An odd-hole-free graph is a graph that does not contain any induced
        subgraph that is an odd hole (a cycle of odd length at least 5).

        Examples:
            >>> GraphClass.odd_hole_free_graphs()
            GraphSet([...])  # All graphs without odd holes as induced subgraphs

        Returns:
            A new GraphSet object containing all odd-hole-free graphs.

        See Also:
            odd_hole_graphs(): For the definition of odd hole graphs.
        """
        return GraphSet.forbidden_induced_subgraphs(GraphClass.odd_hole_graphs())

    @staticmethod
    def chordal_graphs():
        """Returns a GraphSet of all chordal graphs.

        A chordal graph is a graph where every cycle of length at least 4
        has a chord (an edge connecting two non-adjacent vertices in the cycle).
        Equivalently, a graph is chordal if and only if it contains no induced
        cycle of length 4 or greater.

        Examples:
            >>> GraphClass.chordal_graphs()
            GraphSet([[], [(1, 4)], [(4, 5)], [(1, 2)], [(2, 5)], [(2, 3)], [(3, 6)], [( ...

        Returns:
            A new GraphSet object containing all chordal graphs.
        """
        cycles = GraphSet.cycles()
        cycles_length_at_least_4 = cycles.larger(3) # >= 4
        return GraphSet.forbidden_induced_subgraphs(cycles_length_at_least_4)

    @staticmethod
    def cographs():
        """Returns a GraphSet of all cographs.

        A cograph (or complement-reducible graph) is a graph that can be
        generated from a single vertex by complementation and disjoint union.
        Equivalently, a graph is a cograph if and only if it contains no
        induced path of length 4 (P_4).

        Examples:
            >>> GraphClass.cographs()
            GraphSet([...])  # All cographs

        Returns:
            A new GraphSet object containing all cographs.

        Notes:
            See https://www.graphclasses.org/classes/gc_151.html for more information.
        """
        deg_dist = {0: GraphSet.DegreeDistribution_Any, 1: 2, 2: 2}
        p4 = GraphSet.degree_distribution_graphs(deg_dist, False)
        return GraphSet.forbidden_induced_subgraphs(p4)

    @staticmethod
    def chordal_bipartite_graphs():
        """Returns a GraphSet of all chordal bipartite graphs.

        A chordal bipartite graph is a bipartite graph where every cycle of
        length at least 6 has a chord. Note that in a bipartite graph, all
        chords must connect vertices in different partite sets.

        Examples:
            >>> GraphClass.chordal_bipartite_graphs()
            GraphSet([...])  # All chordal bipartite graphs

        Returns:
            A new GraphSet object containing all chordal bipartite graphs.
        """

        cycles = GraphSet.cycles()
        cycles_length_at_least_6 = cycles.larger(5) # >= 6
        chordal = GraphSet.forbidden_induced_subgraphs(cycles_length_at_least_6)
        return chordal & GraphSet.bipartite_graphs()

    @staticmethod
    def split_graphs():
        """Returns a GraphSet of all split graphs.

        A split graph is a graph whose vertices can be partitioned into a clique
        and an independent set. Equivalently, a graph is a split graph if and
        only if it does not contain an induced cycle of length 4 or 5, or a pair
        of disjoint edges (2K_2) as an induced subgraph.

        Examples:
            >>> GraphClass.split_graphs()
            GraphSet([...])  # All split graphs

        Returns:
            A new GraphSet object containing all split graphs.

        Notes:
            See https://www.graphclasses.org/classes/gc_39.html for more information.
        """

        deg_dist = {0: GraphSet.DegreeDistribution_Any, 1: 4}
        graph_2K2 = GraphSet.degree_distribution_graphs(deg_dist, False)
        cycles = GraphSet.cycles()
        cycles_length_4 = cycles.graph_size(4)
        cycles_length_5 = cycles.graph_size(5)

        return GraphSet.forbidden_induced_subgraphs(graph_2K2 | cycles_length_4 | cycles_length_5)

    @staticmethod
    def block_graphs():
        """Returns a GraphSet of all block graphs.

        A block graph is a graph where every biconnected component (block) is a
        clique. Equivalently, a graph is a block graph if and only if it is
        chordal and diamond-free.

        Examples:
            >>> GraphClass.block_graphs()
            GraphSet([...])  # All block graphs

        Returns:
            A new GraphSet object containing all block graphs.

        Notes:
            See https://www.graphclasses.org/classes/gc_93.html for more information.
        """

        return GraphClass.chordal_graphs() & GraphClass.diamond_free_graphs()

    def ptolemaic_graphs():
        """Returns a GraphSet of all ptolemaic graphs.

        A ptolemaic graph is a graph that is both chordal and distance-hereditary.
        Equivalently, a graph is ptolemaic if and only if it is chordal and gem-free.

        Examples:
            >>> GraphClass.ptolemaic_graphs()
            GraphSet([...])  # All ptolemaic graphs

        Returns:
            A new GraphSet object containing all ptolemaic graphs.

        Notes:
            See https://www.graphclasses.org/classes/gc_95.html for more information.
        """

        return GraphClass.chordal_graphs() & GraphClass.gem_free_graphs()

    @staticmethod
    def threshold_graphs():
        """Returns a GraphSet of all threshold graphs.

        A threshold graph is a graph that can be constructed from a single vertex
        by repeatedly adding either an isolated vertex or a vertex connected to all
        existing vertices. Equivalently, a graph is a threshold graph if and only
        if it does not contain a P_4, C_4, or 2K_2 as an induced subgraph.

        Examples:
            >>> GraphClass.threshold_graphs()
            GraphSet([...])  # All threshold graphs

        Returns:
            A new GraphSet object containing all threshold graphs.

        Notes:
            See https://www.graphclasses.org/classes/gc_328.html for more information.
        """

        deg_dist1 = {0: GraphSet.DegreeDistribution_Any, 1: 4}
        graph_2K2 = GraphSet.degree_distribution_graphs(deg_dist1, False)
        cycles = GraphSet.cycles()
        cycles_length_4 = cycles.graph_size(4)
        deg_dist2 = {0: GraphSet.DegreeDistribution_Any, 1: 2, 2: 2}
        p4 = GraphSet.degree_distribution_graphs(deg_dist2, False)

        return GraphSet.forbidden_induced_subgraphs(graph_2K2 | cycles_length_4 | p4)

    @staticmethod
    def gridline_graphs():
        """Returns a GraphSet of all gridline graphs.

        A gridline graph is a graph that does not contain claw, diamond, or
        odd hole as an induced subgraph. These graphs have a special structure
        that makes them representable as a subset of the two-dimensional grid.

        Examples:
            >>> GraphClass.gridline_graphs()
            GraphSet([...])  # All gridline graphs

        Returns:
            A new GraphSet object containing all gridline graphs.

        Notes:
            See https://www.graphclasses.org/classes/gc_736.html for more information.
        """
        cdo = GraphClass.claw_graphs() | GraphClass.diamond_graphs() | GraphClass.odd_hole_graphs()

        return GraphSet.forbidden_induced_subgraphs(cdo)

    @staticmethod
    def domino_graphs():
        """Returns a GraphSet containing all domino graphs.

        A domino graph is a graph that consists of 6 vertices where 4 vertices
        form a cycle of length 4, and each of the remaining 2 vertices is
        adjacent to a distinct pair of consecutive vertices on the cycle.
        Equivalently, it's a graph with 4 vertices of degree 3 and 2 vertices
        of degree 2.

        Examples:
            >>> GraphClass.domino_graphs()
            GraphSet([...])  # All domino graphs

        Returns:
            A new GraphSet object containing all domino graphs.

        Notes:
            See https://www.graphclasses.org/classes/gc_180.html for more information.
        """
        deg_dist = {0: GraphSet.DegreeDistribution_Any, 3: 4, 4: 1}
        w4 = GraphSet.degree_distribution_graphs(deg_dist, False)

        wcg = w4 | GraphClass.claw_graphs() | GraphClass.gem_graphs()

        return GraphSet.forbidden_induced_subgraphs(wcg)

    @staticmethod
    def linear_domino_graphs():
        """Returns a GraphSet of all linear domino graphs.

        A linear domino graph is a graph that does not contain claw or diamond
        as an induced subgraph. These graphs have a special structure related
        to domino graphs but with additional constraints.

        Examples:
            >>> GraphClass.linear_domino_graphs()
            GraphSet([...])  # All linear domino graphs

        Returns:
            A new GraphSet object containing all linear domino graphs.

        Notes:
            See https://www.graphclasses.org/classes/gc_719.html for more information.
        """
        return GraphSet.forbidden_induced_subgraphs(GraphClass.claw_graphs() | GraphClass.diamond_graphs())
