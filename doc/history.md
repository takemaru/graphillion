# History

# Key Changes from Version 0.99 to 2.0 in Graphillion Software

- **New Classes Added**
  - VertexSetSet class for handling sets of vertex sets
  - DiGraphSet class (merged from DiGraphillion) for handling sets of directed graphs
  - EdgeVertexSetSet class for handling sets of subgraphs represented by both edges and vertices

- **VertexSetSet Class for Handling Sets of Vertex Sets**
  - Implements standard set operations: union, intersection, difference, symmetric difference, quotient, remainder, complement, join, meet
  - Implements graph-specific operations: subgraphs, supergraphs, non_subgraphs, non_supergraphs
  - Provides methods to add, remove, or discard vertices or vertex sets
  - Implements flip operation to toggle vertex presence
  - Provides including/excluding operations to filter sets containing/not containing specific vertices
  - Compares sets with equality, subset, superset, and disjoint checks
  - Filters by size with smaller(), larger(), graph_size(), len() methods
  - Supports saving/loading sets to/from files or strings with dump/load and dumps/loads
  - Provides various iterators: standard, random, min-cost, and max-cost
  - Provides minimal() and maximal() operations to find extremal sets
  - Implements cost_le(), cost_ge(), and cost_eq() to filter by vertex costs
  - Supports weighted vertex operations
  - Provides remove_some_vertex(), add_some_vertex(), remove_add_some_vertices() to return sets with vertices removed, added, or both
  - Computes sets of independent_sets(), dominating_sets(), vertex_covers(), cliques() in a graph
  - Converts between GraphSet and VertexSetSet using to_vertexsetset()

- **DiGraphSet Class for Handling Sets of Directed Graphs (Merged from DiGraphillion)**
  - Supports many set operations similar to GraphSet and VertexSetSet
  - Returns sets of directed_cycles() in a graph
  - Returns sets of directed_hamiltonian_cycles() in a graph
  - Returns sets of directed_st_paths() from source s to target t
  - Returns sets of rooted_forests() (possibly with specified roots)
  - Returns sets of rooted_trees() with a specified root node
  - Returns sets of rooted_spanning_forests() with specified root nodes
  - Returns sets of rooted_spanning_trees() with a specified root node

- **EdgeVertexSetSet Class for Handling Sets of Subgraphs Represented by Both Edges and Vertices**
  - Supports many set operations similar to GraphSet and VertexSetSet
  - Converts between GraphSet and EdgeVertexSetSet using to_edgevertexsetset()

- **Improved Universe Management**
  - Integrated universe management for GraphSet, VertexSetSet, and EdgeVertexSetSet into the Universe class
  - Modified so that calling VertexSetSet.set_universe() is no longer necessary

- **Enhanced Graph Creation Methods**
  - bicliques
  - matchings, perfect_matchings, k_matchings, b_matchings
  - k_factors, f_factors
  - regular_graphs, regular_bipartite_graphs
  - steiner_subgraphs, steiner_trees, steiner_cycles, steiner_paths
  - degree_distribution_graphs
  - letter_P_graphs
  - induced_graphs, weighted_induced_graphs
  - chordal_graphs, bipartite_graphs
  - partitions, balanced_partitions
  - Claw graphs and claw-free graphs
  - Diamond graphs and diamond-free graphs
  - Gem graphs and gem-free graphs
  - Odd hole graphs and odd-hole-free graphs
  - Cographs
  - Chordal bipartite graphs
  - Split graphs
  - Block graphs
  - Ptolemaic graphs
  - Threshold graphs
  - Gridline graphs
  - Domino graphs
  - Linear domino graphs

- **Enhanced Graph Operations**
  - Added cost-based filtering methods:
    - cost_le(): Returns graphs with total edge costs less than or equal to a bound
    - cost_ge(): Returns graphs with total edge costs greater than or equal to a bound
    - cost_eq(): Returns graphs with total edge costs equal to a bound
  - Added graph modification methods:
    - remove_some_edge(): Returns sets of graphs with one edge removed
    - add_some_edge(): Returns sets of graphs with one edge added
    - remove_add_some_edges(): Returns sets of graphs with one edge removed and another added
  - Added reliability() method for network reliability calculation

- **Python Language Support Changes**
  - Added support for Python 3
  - Ended support for Python 2.7
  - Modified error raising syntax to use exception classes instead of string errors

- **Performance Improvements**
  - Improved edge traversal with a new 'greedy' algorithm
  - Enhanced serialization/deserialization with proper pickle protocol
  - Added OpenMP parallelization support for graph generation methods

- **Bug Fixes and Maintenance**
  - Fixed bugs in multithreaded environments
  - Distinguished GraphSet from setset class (both can now be used independently)
  - Fixed conversion issues between graphset and string representations
  - Improved exception handling for StopIteration in iterator methods
  - Added support for NetworkX 2.0
  - Changed package name to lowercase following PyPI conventions
  - Added pyproject.toml for Python packaging


# Change detail

## Version 2.0 (Mar 31, 2025)

- Added DiGraphSet class (Merged from [DiGraphillion](https://github.com/ComputerAlgorithmsGroupAtKyotoU/digraphillion))
- Added EdgeVertexSetSet class
- Distinguished GraphSet class from setset class (Both the classes can be used independently)
- Integrated universe management for GraphSet, VertexSetSet, and EdgeVertexSetSet into the Universe class
  - Modified so that calling VertexSetSet.set_universe() is no longer necessary
- Added pyproject.toml
- Changed the package name of graphillion into the lowercase
- Finished the support of Python 2.7

## Version 1.11 (Mar 27, 2025)

- Fixed a bug under the multithread environment.

## Version 1.10 (Feb 6, 2025)

- Fixed minor bugs.

## Version 1.9 (Oct 2, 2024)

- Introduction of GraphClass module
   - Added a new class GraphClass for implementing various graph classification algorithms
   - Moved `chordal_graphs` implementation from GraphSet to GraphClass
   - Added support for various graph classes following standard definitions from graph theory (Most graph classes follow definitions from the Graph Classes website (https://www.graphclasses.org/):
     - Claw graphs and claw-free graphs
     - Diamond graphs and diamond-free graphs
     - Gem graphs and gem-free graphs
     - Odd hole graphs and odd-hole-free graphs
     - Cographs
     - Chordal bipartite graphs
     - Split graphs
     - Block graphs
     - Ptolemaic graphs
     - Threshold graphs
     - Gridline graphs
     - Domino graphs
     - Linear domino graphs

- Enhanced Graph Operations
   - Added new graph creation methods:
     - `bicliques`: Complete bipartite graphs
     - `matchings`, `perfect_matchings`, `k_matchings`, `b_matchings`: Various matchings
     - `k_factors`, `f_factors`: Factors
     - `regular_graphs`, `regular_bipartite_graphs`: Regular graphs
     - `steiner_subgraphs`, `steiner_trees`, `steiner_cycles`, `steiner_paths`: Steiner graphs
     - `degree_distribution_graphs`: Graphs with specified degree distributions
     - `letter_P_graphs`: Graphs shaped like the letter 'P'

## Version 1.8 (Jun 17, 2024)

- Added VertexSetSet class

## Version 1.7 (Apr 12, 2024)

- Several new features were added to the GraphSet class:

  - Added cost-based filtering methods:

    - cost_le(): Returns graphs with total edge costs less than or equal to a bound
    - cost_ge(): Returns graphs with total edge costs greater than or equal to a bound
    - cost_eq(): Returns graphs with total edge costs equal to a bound


  - Added graph modification methods:

    - remove_some_edge(): Returns graphs with one edge removed
    - add_some_edge(): Returns graphs with one edge added
    - remove_add_some_edges(): Returns graphs with one edge removed and another added


  - Added graph creation methods:

    - matchings(): Returns all matchings in the graph
    - perfect_matchings(): Returns all perfect matchings in the graph
    - induced_graphs(): Returns all connected induced graphs
    - weighted_induced_graphs(): Returns weighted connected induced graphs
    - chordal_graphs(): Returns all chordal graphs
    - bipartite_graphs(): Returns all bipartite subgraphs
    - partitions(): Returns partitions with specified numbers of connected components
    - balanced_partitions(): Returns partitions with balanced distribution of vertex weights
  - Added a network reliability calculation method:

    - reliability(): Calculates connection reliability between terminals with edge probabilities

## Version 1.5 (Jul 9, 2021)

- Fixed a bug in the static methods load() and loads() where the returned value from _graphillion.load(fp) and _graphillion.loads(s) was not properly wrapped in a setset instance. This ensured proper type conversion of loaded data.

## Version 1.4 (Nov 4, 2020)

- Fix a bug of converting the graphset [[]] into the string '[[]]'

## Version 1.3 (Jan 25, 2019):
- Added exception handling for StopIteration in iterator methods:
  - Modified `__iter__()` to catch StopIteration exceptions and return gracefully
  - Added similar exception handling in rand_iter(), min_iter(), and max_iter() methods
  - This change makes the iterators more robust when iteration is interrupted

## Version 1.1 (Jun 28, 2018)

- Support NetworkX 2.0

## Version 1.0 (from 0.99) (Mar 2, 2017)

- Added support for Python 3 with compatibility imports from builtins and future.utils
- Improved edge traversal with a new 'greedy' algorithm
- Added full implementation of quotient, remainder, join, meet, subgraphs, supergraphs, non_subgraphs, and non_supergraphs operations
- Added division operators (__div__, __truediv__, __floordiv__) and their in-place versions
- Enhanced serialization/deserialization by setting proper pickle protocol
- Improved the performance of the excluding() method
- Added OpenMP parallelization support for graph generation methods
- Changed syntax for range/xrange to be compatible with Python 3
- Modified error raising syntax to use exception classes instead of string errors
- Updated docstrings to reflect the actual behavior and capabilities
