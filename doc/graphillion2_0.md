# Graphillion 2.0 classes

[Japanese version](graphillion2_0_ja.md)

Several classes, in addition to the GraphSet class, have been added to Graphillion 2.0.

GraphSet: Class that represents set of subgraphs by set of edge sets
VertexSetSet: Class that represents a set of vertex sets
DiGraphSet: Class that represents set of subgraphs of directed graph by set of directed edge sets
EdgeVertexSetSet: Class that represents a set of subgraphs comprising sets of edges and vertices connected to those edges 

To maintain notation consistency, GraphSet has been assigned the alias EdgeSetSet, and both the names GraphSet and EdgeSetSet can be used.


## About Universe

Previous versions of Graphillion allowed users to set the Universe (underlying graph) using the GraphSet.set_universe() method. Users can continue to use the GraphSet.set_universe() method in Graphillion; however, they can now set the Universe common to GraphSet, VertexSetSet, and EdgeVertexSetSet using the Universe.set_universe() method. Users are only required to replace GraphSet.set_universe() with Universe.set_universe(), which allows for existing code written for previous versions to continue to function as is. For example, if setting the vertex set of the underlying graph to V = {1,2,3} and the edge set to E = {(1,2),(1,3)}, then Universe is set as follows:

```
Universe.set_universe([(1,2),(1,3)], order = 'as-is')
```

Calling out this method will set the Universe of GraphSet to [(1,2),(1,3)], the Universe of VertexSetSet to [2,1,3], and the Universe of EdgeVertexSetSet to [(1,2),2,(1,3),1,3]. The order of the Universes of VertexSetSet and EdgeVertexSetSet is set automatically. In most cases, no issues should arise with the Universes of VertexSetSet and EdgeVertexSetSet set to the automatically set order. For VertexSetSet, the Universe order can be manually set by calling the VertexSetSet.set_universe() method. Manual setting of the EdgeVertexSetSet order is not supported in the current version.

The Universe of DiGraphSet is independent of the Universe of GraphSet. The Universe of DiGraphSet can be set regardless of the Universe set for GraphSet. It can be set using the DiGraphSet.set_universe() method or Universe.set_digraph_universe(). A setting example is as follows:

```
GraphSet.set_universe([(1,2), (2,3), (3,4)])
DiGraphSet.set_universe([(1,2), (1,3), (2,1)]) # Can be set independently from GraphSet
```

Unlike the undirected case (GraphSet), the underlying graph can contain directed edges in both directions between the two endpoints, such as (1,2) and (2,1).

Classes that represent a set of vertices or a set of edges that corresponds to DiGraphSet do not exist in the current version (Graphillion 2.0).


## Cases where underlying graph is required for VertexSetSet

For example, the VertexSetSet class can generate a set of independent sets. The current version, Graphillion 2.0, requires the user to manually set the same graph as the one set in Universe when generating a set of independent sets. Future versions will not require the user to specify the graph as an argument.

```
# Find the set of all independent sets in graph [(1,2),(1,3)].
# Graph must be specified as an argument
# Note that Universe.set_universe([(1,2),(1,3)]) must be called in advance to set the Universe of VertexSetSet.
vss = VertexSetSet.independent_setsets([(1,2),(1,3)])
```

In Universe.set_universe, only the vertex set is set as the Universe for VertexSetSet.
Therefore, the underlying graph set in Universe.set_universe can differ from the argument graph in VertexSetSet.independent_setsets(). However, as a general rule, setting the same graph for both is a safer option to avoid confusion.


## Isolated vertices

(Graphillion 2.0 does not support isolated vertices. A future version will support it.)

The EdgeVertexSetSet class can manage graphs with isolated vertices (vertices with degree 0). For example, when V = {1,2,3,4,5,6} and E = {(1,2),(1,3)}, then vertices 4, 5, and 6 are isolated vertices. GraphSet manages subgraphs as a set of edge sets; therefore, isolated vertices are meaningless and cannot be set in the previous version.
(For example, GraphSet.trees is a method of returning a set of trees; however, trees are represented by a set of edges, which implies that trees with only one vertex, such as ({4}, {}), cannot be represented.)
Isolated vertices can be set with the isolated argument of the set_universe method.
The isolated argument is provided with a list of isolated vertices.

```
Universe.set_universe([(1,2),(1,3)], isolated = [4, 5, 6], order = 'as-is')
```

When using GraphSet or DiGraphSet, isolated vertices that are set as “isolated” are disregarded. (In the example above, [4,5,6] is considered as non-existent.)
For VertexSetSet and EdgeVertexSetSet, the Universe is set to include isolated vertices.
In the example above, the Universe of VertexSetSet is set to [2,1,3,4,5,6], whereas that of EdgeVertexSetSet is set to [(1,2),2,(1,3),1,3,4,5,6].

In the example below, the set of independent sets is shown on a graph that includes isolated vertices.

```
vss = VertexSetSet.independent_setsets([(1,2),(1,3)], isolated = [4,5,6])
```

Here, vss contains independent sets, such as [2,3,4], [2,3,4,5,6], and [2,3].

## About EdgeVertexSetSet

EdgeVertexSetSet is a class for managing a set of subgraphs, where each subgraph
comprises a set of edges and vertices included in the edges.
For example, when the Universe is V = {1,2,3,4,5,6} and E = {(1,2),(1,3)},
then [(1,2),1,2], [(1,2),1,2,3] and [(1,2),1,2,4] can all be elements of EdgeVertexSetSet. However,
 [(1,2),1] and [(1,2)] cannot be elements of EdgeVertexSetSet (because the former example
does not include vertex 2, and the latter example does not include vertices 1 and 2).

EdgeVertexSetSet in the current version (Graphillion 2.0) has only limited functionality.
The main method for constructing EdgeVertexSetSet is by conversion from the GraphSet class.
The following example converts a set of paths into a set of edges that compose the paths and the vertices in the edges.

```
gs = GraphSet.paths(...)
evss = gs.to_edgevertex_setset() # Convert from GraphSet class to EdgeVertexSetSet
```

Users can use evss to, for example, solve optimization problems for graphs in which the vertices and edges are weighted.



## About Reconfillion

Reconfillion is a library for solving combinatorial reconfiguration problems
using Graphillion. New versions of Graphillion support Reconfillion.

A combinatorial reconfiguration problem is a problem in which, when provided a combinatorial problem, two solutions to the combinatorial problem, and a reconfiguration rule,
the possibility of reconfiguring from one solution to the other must be determined while satisfying the conditions of the combinatorial problem
based on the reconfiguration rule. For example, for an independent set reconfiguration problem, a graph G and two independent sets are provided.
Several possible reconfiguration rules can be considered. However, the most well-known is a rule known as token jumping,
which involves removing one vertex from an independent set S and adding another vertex not included in S.

In Graphillion, users can construct a set of executable solutions (in the example above, the set of all independent sets in graph G), which can then be
used to solve combinatorial reconfiguration problems. Various reconfiguration rules are supported, including the abovementioned token jumping. This allows users to solve the corresponding reconfiguration problem
for any object manageable by Graphillion.


The following code enables users to solve a combinatorial reconfiguration problem (i.e., obtain the reconfiguration sequence that is the solution).

```
graph = [(1, 2), (1, 3), (1, 4), (2, 3), (2, 4), (3, 4)]

from graphillion import GraphSet
from reconfillion import reconf

GraphSet.set_universe(graph)
spanning_trees = GraphSet.trees(is_spanning = True)

s = [(1, 2), (1, 3), (1, 4)]
t = [(1, 4), (2, 4), (3, 4)]

reconf_sequence = reconf.get_reconf_seq(s, t, spanning_trees, model = 'tj')
```

The following example shows the identification of multiple reconfiguration sequences.

```
reconf_sequences = reconf.get_reconf_seq_multi(s, t, spanning_trees, model = 'tj')

for i in reconf_sequences:
    print(i)
```

Here, reconf_sequences is an iterator object. Generally, the number of reconfiguration sequences becomes massive owing to combinatorial
explosion; therefore, enumerating and processing all of them, as shown above, is challenging.
Users should retrieve only the number of sequences they require from reconf_sequences.

The classes that support combinatorial reconfiguration problems are GraphSet, DiGraphSet, VertexSetSet, and setset.
For GraphSet, reconfiguration problems, such as path, spanning tree, matching, and Steiner tree, are supported
(supports all graph classes manageable by GraphSet).
For VertexSetSet, for example, reconfiguration problems, such as independent set, dominating set, vertex cover, and transverse set, are supported.
The setset class represents a set of element sets. If the user can construct an object of the setset class,
then they can solve various combinatorial reconfiguration problems. The following is an example showing how the knapsack reconfiguration problem is addressed.

```
ss = setset({}) # Power set
weights = {...} # Element weights
ss2 = ss.cost_le(weights, b) # Extract only sets with weights less than or equal to b
# Reconfiguration problem can be solved using solution space ss2.
```



## About parallel computation in Graphillion 2.0

Parallel computation was supported in previous versions of Graphillion 2.0. However, in Graphillion 2.0,
users can control the number of threads to be used.
Internally, the corresponding OpenMP functions/methods are called; readers can refer to
the OpenMP manual for more details.

## Acknowledgement

This library is/was partly developed by research supported by JSPS KAKENHI Grant Numbers JP18H04091, JP20H05794, and JP23H04383.

