Graphillion - A fast, lightweight graphset operation library
================================================================================

* [Features](#features "Features")
* [Overview](#overview "Overview")
* [Installing](#installing "Installing")
* [Tutorial](#tutorial "Tutorial")
* [Creating graphsets](#creating-graphsets "Creating graphsets")
* [Graphset operations](#graphset-operations "Graphset operations")
* [Dumping and loading graphsets](#dumping-and-loading-graphsets "Dumping and loading graphsets")
* [Working with NetworkX](#working-with-networkx "Working with NetworkX")
* [Library reference](#library-reference "Library reference")
* [References](#references "References")

Features
--------------------------------------------------------------------------------

Graphillion is a Python language software package for the
manipulation, optimization, and study of a large set of graphs.

* Python language data structures for a graphset, a large set of graphs
* Search, optimization, and enumeration on a large graphset
* Several efficient operations designed for graphsets
* Working with existing graph tools like NetworkX
* Open source MIT license
* Well tested: more than 600 unit tests
* Additional benefits from Python: fast prototyping, easy to teach, multi-platform

We provide funny short movies to answer your questions.

* Why Graphillion? [Time with class! Let's
  count!](http://youtu.be/Q4gTV4r0zRs) (viewed more than a million times)
* What's Graphillion? (to appear)

Overview
--------------------------------------------------------------------------------

Graphillion is a Python library for efficient *graphset operations*.
Unlike existing graph tools such as NetworkX, which are designed to
manipulate just a single graph at a time, Graphilion handles a large
*set* of graphs with great efficiency.  Surprisingly, trillions of
trillions of graphs can be processed in a single computer.

You may be curious about an uncommon concept of *graphset*, but it
comes along with any graph or network when you consider multiple
subgraphs cut from the graph; e.g., considering possible driving routes on
a road map, examining feasible electric flows on a power grid, or
evaluating the structure of chemical reaction networks.  The number of
such subgraphs can be more than googol, 10^100, even in a graph with
just a few hundreds of edges, since subgraphs increase exponentially
with the graph size.  It takes millions of years to examine all
subgraphs with a naive approach as demonstrated in the funny movie above;
Graphillion is our answer to resolve this issue.

Graphillion allows you to exhaustively but efficiently search a given
graphset (a set of subgraphs) with complex, even nonconvex, constraints.
In addition, you can find top-k optimal
graphs from the complicated set, and can also extract common
properties seen among all graphs in the set.  Thanks to these features,
Graphillion has a variety of applications including graph database,
combinatorial optimization, and a graph structure analysis.  We
will show some practical use cases in the following tutorial, such as
evaluation of power distribution networks.

Graphillion can be used freely under the MIT license.  Graphillion is
mainly developed in [JST ERATO Minato
project](http://www-erato.ist.hokudai.ac.jp/?language=en).  We would
really appreciate if you would refer to our paper (to appear)
and address our contribution on the use of Graphillion in your paper.

Now, install Graphillion and go to the tutorial, and you'll find its power
and utility.

Installing
--------------------------------------------------------------------------------

### Quick install

Just type:

```bash
$ easy_install graphillion
```

and an attempt will be made to find and install an appropriate version
that matches your operating system and Python version.

### Installing from source

You can install from source by downloading a source archive file
(tar.gz or zip) or by checking out the source files from the GitHub
source code repository.

#### Source archive file

1. Download the source (tar.gz or zip file) from
   https://github.com/takemaru/graphillion
2. Unpack and change directory to the source directory (it should have
   the file setup.py)
3. Run `python setup.py install` to build and install
4. (optional) Run `python setup.py test -q` to execute the tests

#### GitHub repository

1. Clone the Graphillion repostitory
```bash
$ git clone https://github.com/takemaru/graphillion.git
```
2. Change directory to "graphillion"
3. Run `python setup.py install` to build and install
4. (optional) Run `python setup.py test -q` to execute the tests

If you don't have permission to install software on your system, you
can install into another directory using the `-user`, `-prefix`, or
`-home` flags to setup.py.  For example:

```bash
$ python setup.py install --prefix=/home/username/python
  or
$ python setup.py install --home=~
  or
$ python setup.py install --user
```

If you didn't install in the standard Python site-packages directory
you will need to set your `PYTHONPATH` variable to the alternate
location.  See http://docs.python.org/inst/search-path.html for further
details.

### Requirements

#### Python

To use Graphillion, you need Python version 2.6 or later.
http://www.python.org/

#### GCC (The Gnu Compiler Collection)

To compile Graphillion, you need gcc version 4.2 or later.
http://gcc.gnu.org/

#### GMP (The Gnu multiple precision arithmetic library) - optional

With GMP, Graphillion provides arbitrary precision in counting the
number of graphs in a set; otherwise double precision floating-point
numbers are used.  http://gmplib.org/

#### NetworkX and Matplotlib - used just in the tutorial

NetworkX and Matplotlib are used to create and draw a graph.  These
packages are not required for Graphillion, but just used in the
tutorial for helping graph creation and drawings.  They can be
installed by:

```bash
$ easy_install networkx
$ easy_install matplotlib
```

Tutorial
--------------------------------------------------------------------------------

If you haven't seen this funny movie, [Time with class! Let's
count!](http://youtu.be/Q4gTV4r0zRs), please watch it before the
tutorial.  This movie, which has been viewed more than a million
times, will convince you of a need for Graphillion.  The summary of
this tutorial is also provided as a movie (to appear).

We believe you enjoyed the movies and understood the necessity and
features of Graphillion.  Now, let's see Graphillion in more detail.

We first introduce terminology used in Graphillion, as follows:

| Term          | Description                  | Example                                          |
|:--------------|:-----------------------------|:-------------------------------------------------|
| vertex        | any hashable object          | `1`, `'v1'`                                      |
| edge          | tuple of vertices            | `(1, 2)`                                         |
| weighted edge | tuple of vertices and weight | `(1, 2, -1.5)`                                   |
| graph         | list of (weighted) edges     | `[(1, 2), (1, 3)]`                               |
| set of graphs | GraphSet object              | `GraphSet([[(1, 2), (1, 3)], [(1, 2), (2, 3)]])` |

Vertices (or nodes) can be any hashable object; e.g., a number, a text
string, etc.  Edges (or links) are a pair of vertices, and a graph is
defined as a list of edges; currently, Graphillion supports undirected graphs
only.  A GraphSet object stores a set of graphs; we will manipulate
graphs in the object in a variety of ways.

Before anything else, we import Graphillion as well as a helper module,
which provides some functions like graph creation and drawing for the
tutorial.

```python
>>> from graphillion import GraphSet
>>> import graphillion.tutorial as tl  # helper functions just for tutorial
```

### Paths on a grid

At the beginning, we define our universe; a graph handled in
Graphillion must be a subgraph of this universal graph.  In this
tutorial, we use the 8x8 grid graph.

```python
>>> universe = tl.grid(8, 8)
>>> GraphSet.set_universe(universe)
>>> tl.draw(universe)  # show pop-up window for universal graph
```

We find all the simple paths between the opposing corners; it took
four hours with the supercomputer in the movie.

```python
>>> start = 1
>>> goal = 81
>>> paths = GraphSet.paths(start, goal)
>>> len(paths)
3266598486981642
```

It's very quick, isn't it?  Since the `paths` object contains all the
paths, you can enumerate them one by one though it'll take years.

```python
>>> for path in paths:
...     path
... # stop by Ctrl-C
>>> tl.draw(paths.choice())  # show one of the paths
```

Next, we select some of the paths with given conditions, in order to
demonstrate the filtering capability of Graphillion.  Let's assume
that a treasure box and its key are placed on the grid as shown in the
figure.  We consider all paths on which the key is picked up before
reaching the treasure box.  First, search for paths to the key not
through the treasure box, and then select the paths that include the
key's paths and the treasure box.

```python
>>> key = 64
>>> treasure = 18
>>> paths_to_key = GraphSet.paths(start, key).excluding(treasure)
>>> treasure_paths = paths.including(paths_to_key).including(treasure)
>>> len(treasure_paths)
789438891932744
```

Test whether all the treasure paths are a subset of the original paths
between the corners.

```python
>>> treasure_paths < paths  # "<" means subset-of
True
```

We conduct statistical processing with random sampling.  Graphillion
enables you to choose a sample (a graph) from the graphset uniformly
randomly.  Draw a histogram of turn-counts on the treasure paths as
follows:

```python
>>> i = 0
>>> data = []
>>> for path in treasure_paths.rand_iter():
...     data.append(tl.how_many_turns(path))  # count the number of turns on the path
...     if i == 100: break
...     i += 1
...
>>> tl.hist(data)
```

The histogram shows that you might turn about 30-50 times on a path in
most cases.  However, you also find that it is only five turns on the
shortest path, which is derived by method `min_iter()`, an optimizer
provided by Graphillion.

```python
>>> for path in treasure_paths.min_iter():
...     tl.how_many_turns(path)
...     break
...
5
```

### Switch configurations on a power distribution network

Graphillion works on any graphs other than square grids, and handles
other subgraphs as well as simple paths.  Next, we consider a power
distribution network in the figure.  In the network, a vertex is a
house, and an edge is a power line with a switch.  The power is
provided from the four generators at corners.

```python
>>> universe = tl.grid(8, 8, 0.37)  # 37 % of edges are removed from 8x8 grid
>>> GraphSet.set_universe(universe)
>>> tl.draw(universe)
```

The power flow is determined by configuring switches, which are placed
on each line.  If a switch is closed (an edge exists), the power is
transmitted on the line; otherwise, not.  The power must be
transmitted to all houses, while the flow must not have a loop to
protect against short circuit.  The power flow, hence, must be a
*forest*, a set of trees, rooted at generators.  We find all of such
forests as follows:

```python
>>> generators = [1, 9, 73, 81]
>>> forests = GraphSet.forests(roots=generators, is_spanning=True)
>>> len(forests)
54060425088
>>> tl.draw(forests.choice())
```

The amount of power transmitted from a single generator should be
strictly restricted.  The forest shown above may have a very large
tree, which implies that a generator sends much power beyond its
capacity.  Here, we assume that each generator is allowed to provide
less than 23 houses for protection.  We first find all dangerous cases
with too much power, and then select safe power flows without the
dangerous cases.

```python
>>> too_large_trees = GraphSet()
>>> for substation in generators:
...     too_large_trees |= GraphSet.trees(root=substation).larger(23)
...
>>> safe_forests = forests.excluding(too_large_trees)
>>> len(safe_forests)
294859080
>>> tl.draw(safe_forests.choice())
```

Since we found all the safe flows, we try to change the network from
the current configuration to a safe one with optimization.  The
current configuration is given by:

```python
>>> closed_switches = tl.current_config()
>>> tl.draw(closed_switches)
```

Next configuration must be corresponding with one of safe flows, and
must be realized with least switch operations.  We put a penalty on a
switch status if it is inconsistent with the current status, as shown
in the table.

| current \ next | open | closed |
|:---------------|-----:|-------:|
| open           |    0 |     -1 |
| closed         |    0 |      1 |

```python
>>> weights = {}
>>> for switch in universe.edges():
...     weights[switch] = 1 if switch in closed_switches else -1
...
```

Find a flow (forest) with a maximum weight.  The flow has a least
penalty and can be realized with the least switch operations.  Compare
it with the current configuration above, and you'll find them quite
alike.

```python
>>> for forest in safe_forests.max_iter(weights):
...     tl.draw(forest)
...     break
...
```

Finally, we investigte serious failures that prevent the safe power
delivery.  We search for minimal blocking sets, or minimal hitting
sets more generally, to study such failures.  A hitting set is roughly
defined such that all the given sets are *hit* by at least one element
in a hitting set; e.g., given {1, 2}, {2, 3}, and {3}, hitting sets
are {1, 3} and {2, 3}.  If power lines in a hitting set against the
safe flows are broken, all the flows can't be configured.

```python
>>> failures = safe_forests.blocking().minimal()
```

To help your understanding, remove all lines in a hitting set from
each safe flows.  Then, you'll find no safe flow.

```python
>>> failure = failures.choice()
>>> for line in failure:
...     safe_forests = safe_forests.excluding(line)
...
>>> len(safe_forests)
0
```

Small hitting sets (e.g., less than five lines) might imply
vulnerability of the network.  We now find several small failure
patterns, which should be investigated careflly.

```python
>>> len(failures.smaller(5))
767
```

Though actual power distribution networks are much more complicated,
we basically rely on the same idea with Graphillion.  Our power loss
minimization tool, which features a nonconvex optimization, is
available online at [DNET](https://github.com/takemaru/dnet).


Creating graphsets
--------------------------------------------------------------------------------

Graphillion provides three ways to create a GraphSet object; providing
a list of graphs, giving edge constraints, and specifying graph types
like paths and trees.  GraphSet objects created can be modified by
operations described in the next section.

Please don't forget to set the universal graph before working with
GraphSet, as mentioned in the tutorial.

```python
>>> from graphillion import GraphSet
>>> universe = [(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5), (5, 6)]
>>> GraphSet.set_universe(universe)
```

### List of graphs

This is the most stright-forward way to create a GraphSet object.
Specify a list of graphs and get an object that includes the graphs.

In the following example, two graphs, one has a single edge and the
other has two edges, are given, and a GraphSet object with these
graphs is created.

```python
>>> graph1 = [(1, 4)]
>>> graph2 = [(1, 2), (2, 3)]
>>> gs = GraphSet([graph1, graph2])
>>> gs
GraphSet([[(1, 4)], [(1, 2), (2, 3)]])
```

If no argument is given, it is treated as an empty list `[]` and an
empty GraphSet is returned.

```python
>>> gs = GraphSet()
>>> gs
GraphSet([])
```

### Edge constraints

Edge constraints specify edges to be included or not in the object to
be created.  The constraints must be represented by a dict of included
or excluded edge lists.  Edges not specified in the dict are
"don't-care"; they can be included and exculded in the object.

In the following example, edge (1, 4) must be included while edges (1,
2) and (2, 3) must not be.

```python
>>> edges1 = [(1, 4)]
>>> edges2 = [(1, 2), (2, 3)]
>>> GraphSet({'include': edges1, 'exclude': edges2})
GraphSet([[(1, 4)], [(1, 4), (2, 5)], [(1, 4), (3, 6)], ...
```

An empty dict `{}` means that no constraint is specified, and so a
GraphSet including all possible graphs in the universe is returned
(let N the number of edges in the universe, 2^N graphs are stored).

```python
>>> gs = GraphSet({})
>>> len(gs)
128  # 2^7
```

### Graph types

You can specify a graph type, such as paths and trees, and create a
GraphSet object that stores all graphs matching the specified type.
Graphillion supports the following graph types:

- connected components,
- cliques,
- trees,
- forests,
- cycles, and
- paths.

For example, `paths()` method takes two arguments, two end vertices,
and finds all paths between the vertices.

```python
>>> paths = GraphSet.paths(1, 6)
>>> paths
GraphSet([[(1, 2), (2, 3), (3, 6)], [(1, 2), (2, 5), (5, 6)], [(1, 4), (4, 5 ...
```

The arguments are defined for each graph type, please see the library
reference in detail.

Graphillion also provides low-level interface `graphs()` to specify
more complicated graph types; actually, the above methods call this
low-level interface internally.

The following example is the same with `paths(1, 6)`, paths between 1
and 6.

```python
>>> start = 1
>>> end = 6
>>> zero_or_two = xrange(0, 3, 2)
>>> degree_constraints = {start: 1, end: 1,
...                       2: zero_or_two, 3: zero_or_two,
...                       4: zero_or_two, 5: zero_or_two}
>>> GraphSet.graphs(vertex_groups=[[start, end]],
...                 degree_constraints=degree_constraints,
...                 no_loop=True)
GraphSet([[(1, 2), (2, 3), (3, 6)], [(1, 2), (2, 5), (5, 6)], [(1, 4), (4, 5 ...
```

If these methods are called as object methods, `gs.paths(1, 6)`,
graphs to be found are selected from the object.  Please see the
library reference for more detail.

Graphset operations
--------------------------------------------------------------------------------

Graphillion provides many operations to manipulate graphs in a
GraphSet object.  These operations are classified into selection,
modification, and comparison; some of them are derived from Python's
set methods.  Graphillon also provides some iterators to traverse the
set.  Please see the library reference for details of each method.

### Selection methods

The following methods select graphs from a given GraphSet object (or
from two given GraphSet objects if binary operation).  No new graphs
are generated during the operation.

- `union(gs)`, `|`,
- `intersection(gs)`, `&`,
- `difference(gs)`, `-`,
- `symmetric_difference(gs)`, `^`,
- `update(gs)`,
- `including(gs, g, e, or v)`,
- `excluding(gs, g, e, or v)`,
- `included(gs)`,
- `larger(n)`,
- `smaller(n)`,
- `len(n)`,
- `minimal()`, and
- `maximal()`.

### Modification or generation methods

The following methods generate new graphs.  Some store new graphs into
`self`, while others return new GraphSet.

#### Modifying self

- `add(g or e)`,
- `remove(g, e, or v)`, `discard(g, e, or v)`,
- `flip(e)`,
- `pop()`, and
- `clear()`.

#### Returning new GraphSet

- `~`,
- `complement()`, and
- `blocking()`.

### Comparison or evaluation methods

The following methods provide comparison or evaluation.

- `isdisjoint(gs)`,
- `issubset(gs)`,
- `issuperset(gs)`,
- `in`, and
- `len`.

### Python's set methods

Graphillion supports Python's set methods.  These methods treat a
graph just an element of the set and don't care the graph structure.

- `union(gs)`, `|`,
- `intersection(gs)`, `&`,
- `difference(gs)`, `-`,
- `symmetric_difference(gs)`, `^`,
- `update(gs)`,
- `add(g)`,
- `remove(g)`, `discard(g)`,
- `pop()`,
- `clear()`,
- `isdisjoint(gs)`,
- `issubset(gs)`,
- `issuperset(gs)`,
- `in`,
- `len`, and
- `copy()`.

### Iterators

Graphillion provides various iterators.  `rand_iter()` generates a
random sequence of graphs.  `min_iter()` and `max_iter()` provides
optimization by iterating graphs in the weight order.  `choice()` is
not an iterator, but it returns a graph in it.

- `(default iterator)`,
- `rand_iter()`,
- `min_iter()`,
- `max_iter()`, and
- `choice()`.


Dumping and loading graphsets
--------------------------------------------------------------------------------

Working with NetworkX
--------------------------------------------------------------------------------

Library reference
--------------------------------------------------------------------------------

Library reference can be browsed using pydoc:

```bash
$ pydoc graphillion.GraphSet
```

References
--------------------------------------------------------------------------------
