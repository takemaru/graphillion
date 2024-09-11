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

"""Graph classes.
"""

from graphillion import GraphSet

class GraphClass:

    @staticmethod
    def claw_graphs():
        deg_dist = {0: GraphSet.DegreeDistribution_Any, 1: 3, 3: 1}
        gs = GraphSet.degree_distribution_graphs(deg_dist, False)
        return gs

    @staticmethod
    def claw_free_graphs():
        return GraphSet.forbidden_induced_subgraphs(GraphClass.claw_graphs())

    @staticmethod
    def diamond_graphs():
        # assume that universe has neither parallel edges nor self-loop
        deg_dist = {0: GraphSet.DegreeDistribution_Any, 2: 2, 3: 2}
        gs = GraphSet.degree_distribution_graphs(deg_dist, False)
        return gs

    @staticmethod
    def diamond_free_graphs():
        """See https://www.graphclasses.org/classes/gc_441.html .
        
        """
        return GraphSet.forbidden_induced_subgraphs(GraphClass.diamond_graphs())

    @staticmethod
    def gem_graphs():
        # assume that universe has neither parallel edges nor self-loop
        deg_dist = {0: GraphSet.DegreeDistribution_Any, 2: 2, 3: 2, 4: 1}
        gs = GraphSet.degree_distribution_graphs(deg_dist, False)
        return gs

    @staticmethod
    def gem_free_graphs():
        """See https://www.graphclasses.org/classes/gc_354.html .
        
        """
        return GraphSet.forbidden_induced_subgraphs(GraphClass.gem_graphs())

    @staticmethod
    def odd_hole_graphs():
        """See https://www.graphclasses.org/smallgraphs.html#odd_holes .

        """

        univ = GraphSet.universe()
        vertices = set()
        for e in univ:
            vertices.add(e[0])
            vertices.add(e[1])
        n = len(vertices)

        # cycle with odd length at least 5
        dc = {}
        for v in GraphSet._vertices:
            dc[v] = range(0, 3, 2)
        return GraphSet.graphs(vertex_groups=[[]], degree_constraints=dc,
                               num_edges=range(5, n + 1, 2))

    @staticmethod
    def odd_hole_free_graphs():

        return GraphSet.forbidden_induced_subgraphs(GraphClass.odd_hole_graphs())

    @staticmethod
    def chordal_graphs():
        """Returns a GraphSet with chordal graphs.

        Examples:
            >>> GraphSet.chordal_graphs()
            GraphSet([[], [(1, 4)], [(4, 5)], [(1, 2)], [(2, 5)], [(2, 3)], [(3, 6)], [( ...

        Returns:
            A new GraphSet object.
        """
        cycles = GraphSet.cycles()
        cycles_length_at_least_4 = cycles.larger(3) # >= 4
        return GraphSet.forbidden_induced_subgraphs(cycles_length_at_least_4)

    @staticmethod
    def cographs():
        """Returns a GraphSet with cographs.
        See https://www.graphclasses.org/classes/gc_151.html .

        Examples:
            >>> GraphSet.cographs()

        Returns:
            A new GraphSet object.
        """
        deg_dist = {0: GraphSet.DegreeDistribution_Any, 1: 2, 2: 2}
        p4 = GraphSet.degree_distribution_graphs(deg_dist, False)
        return GraphSet.forbidden_induced_subgraphs(p4)

    @staticmethod
    def chordal_bipartite_graphs():
        """Returns a GraphSet of chordal bipartite subgraphs.

        Example:
          >>> GraphSet.chordal_bipartite_graphs()

        Returns:
          A new GraphSet object.
        """

        cycles = GraphSet.cycles()
        cycles_length_at_least_6 = cycles.larger(5) # >= 6
        chordal = GraphSet.forbidden_induced_subgraphs(cycles_length_at_least_6)
        return chordal & GraphSet.bipartite_graphs()

    @staticmethod
    def split_graphs():
        """Returns a GraphSet of split subgraphs.
        See https://www.graphclasses.org/classes/gc_39.html .

        Example:
          >>> GraphSet.split_graphs()

        Returns:
          A new GraphSet object.
        """

        deg_dist = {0: GraphSet.DegreeDistribution_Any, 1: 4}
        graph_2K2 = GraphSet.degree_distribution_graphs(deg_dist, False)
        cycles = GraphSet.cycles()
        cycles_length_4 = cycles.graph_size(4)
        cycles_length_5 = cycles.graph_size(5)

        return GraphSet.forbidden_induced_subgraphs(graph_2K2 | cycles_length_4 | cycles_length_5)

    @staticmethod
    def block_graphs():
        """Returns a GraphSet of block subgraphs.
            See https://www.graphclasses.org/classes/gc_93.html .

        Example:
          >>> GraphSet.block_graphs()

        Returns:
          A new GraphSet object.
        """

        return GraphClass.chordal_graphs() & GraphClass.diamond_free_graphs()

    def ptolemaic_graphs():
        """Returns a GraphSet of ptolemaic subgraphs.
            See https://www.graphclasses.org/classes/gc_95.html .

        Example:
          >>> GraphSet.ptolemaic_graphs()

        Returns:
          A new GraphSet object.
        """

        return GraphClass.chordal_graphs() & GraphClass.gem_free_graphs()

    @staticmethod
    def threshold_graphs():
        """Returns a GraphSet of threshold subgraphs.
        See https://www.graphclasses.org/classes/gc_328.html .

        Example:
          >>> GraphSet.threshold_graphs()

        Returns:
          A new GraphSet object.
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
        """See https://www.graphclasses.org/classes/gc_736.html .

        """
        cdo = GraphClass.claw_graphs() | GraphClass.diamond_graphs() | GraphClass.odd_hole_graphs()

        return GraphSet.forbidden_induced_subgraphs(cdo)

    @staticmethod
    def domino_graphs():
        """See https://www.graphclasses.org/classes/gc_180.html .

        """
        deg_dist = {0: GraphSet.DegreeDistribution_Any, 3: 4, 4: 1}
        w4 = GraphSet.degree_distribution_graphs(deg_dist, False)

        wcg = w4 | GraphClass.claw_graphs() | GraphClass.gem_graphs()

        return GraphSet.forbidden_induced_subgraphs(wcg)

    @staticmethod
    def linear_domino_graphs():
        """See https://www.graphclasses.org/classes/gc_719.html .

        """
        return GraphSet.forbidden_induced_subgraphs(GraphClass.claw_graphs() | GraphClass.diamond_graphs())
