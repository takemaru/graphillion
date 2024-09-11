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
    def threshold_graphs():
        """Returns a GraphSet of threshold subgraphs.

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
