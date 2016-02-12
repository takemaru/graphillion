Graphillion - Fast, lightweight library for a huge number of graphs
================================================================================

[Japanese page](http://github.com/takemaru/graphillion/wiki)

* [News](#news "News")
* [Features](#features "Features")
* [Overview](#overview "Overview")
* [Installing](#installing "Installing")
* [Tutorial](#tutorial "Tutorial")
* [Creating graphsets](#creating-graphsets "Creating graphsets")
* [Manipulating graphsets](#manipulating-graphsets "Manipulating graphsets")
* [Working with NetworkX](#working-with-networkx "Working with NetworkX")
* [Library reference](#library-reference "Library reference")
* [Example codes](http://github.com/takemaru/graphillion/wiki/Example-codes)
* [Future work](#future-work "Future work")
* [References](#references "References")

News
--------------------------------------------------------------------------------

- Graphillion [book] is published in April 2015 (sorry, written in
  Japanese).
- Graphillion was used in the lecture by Prof. Jun Kawahara at Nara
  Institute of Science and Technology in January and February 2015.

Features
--------------------------------------------------------------------------------

Graphillion is a Python software package on search, optimization, and
enumeration for a *graphset*, or a set of graphs.

* Lightweight data structures for handling *x-illions* of graphs
* Search, optimization, and enumeration for large and complex graph
  sets
* Used for power network evaluation ([DNET]), railway analysis
  ([Ekillion]), etc.
* Efficient implementation extending Python with C/C++
* Working with existing graph tools like [NetworkX]
* Open source MIT license
* Well tested: more than 600 unit tests
* Additional benefits from Python: fast prototyping, easy to teach,
  and multi-platform

We provide fun short movies to answer the following questions.

* Why Graphillion? Watch [Time with class! Let's count!], which is
  watched more than a million times!

    [![][lets_count-thumbnail]][Time with class! Let's count!]

* What's Graphillion?  Watch [Graphillion: Don't count naively].

    [![][dont_count-thumbnail]][Graphillion: Don't count naively]


Overview
--------------------------------------------------------------------------------

Graphillion is a Python library for efficient *graphset* operations.
Unlike existing graph tools such as [NetworkX], which are designed to
manipulate just a single graph at a time, Graphillion handles a large
*set* of graphs very efficiently.  Surprisingly, trillions of
trillions of graphs can be processed on a single computer with
Graphillion.

You may be curious about an uncommon concept of *graphset*, but it
comes along with any graph or network when you consider multiple
subgraphs cut from the graph; e.g., considering possible driving
routes on a road map, examining feasible electric flows on a power
grid, or evaluating the structure of chemical reaction networks.  The
number of such subgraphs can be trillions even in a graph with just a
few hundreds of edges, since subgraphs increase exponentially with the
graph size.  It takes millions of years to examine all subgraphs with
a naive approach as demonstrated in the fun movie above; Graphillion
is our answer to resolve this issue.

Graphillion allows you to exhaustively but efficiently search a
graphset with complex, even nonconvex, constraints.  In addition, you
can find top-k optimal graphs from the complex graphset, and can also
extract common properties among all graphs in the set.  Thanks to
these features, Graphillion has a variety of applications including
graph database, combinatorial optimization, and a graph structure
analysis.  We will show some practical use cases in the following
[tutorial], including evaluation of power distribution networks.

Graphillion can be used freely under the MIT license.  It is mainly
developed by [JST ERATO Minato project].  We would really appreciate
if you would refer to our paper and address our contribution on the
use of Graphillion in your paper.

> Takeru Inoue, Hiroaki Iwashita, Jun Kawahara, and Shin-ichi Minato:
  "Graphillion: Software Library Designed for Very Large Sets of
  Labeled Graphs," International Journal on Software Tools for
  Technology Transfer, Springer, vol.18, issue 1, pp.57-66, February 2016. ([pdf](http://dx.doi.org/10.1007/s10009-014-0352-z))

Graphillion is still under the development.  We really appreciate any
pull request and patch if you add some changes that benefit a wide
variety of people.

Now, install Graphillion and go to the tutorial.  You'll find its
power and utility.

Installing
--------------------------------------------------------------------------------

### Requirements

Graphillion requires 64-bit machines to efficiently handle large numbers.

#### Python

To build Graphillion, you need Python version 2.6 or later, with
development environment (Python.h is required).
http://www.python.org/

#### GCC (The Gnu Compiler Collection) or Clang

To build Graphillion, you need gcc version 4.2 or later.
http://gcc.gnu.org/

For MacOSX: use Apple official Clang included in XCode, or a memory allocation
error might occur.

#### NetworkX and Matplotlib - optional for the tutorial

[NetworkX] and Matplotlib are Python modules for creating and drawing
a graph.  These packages are not required for Graphillion, but used in
[tutorial].  They can be installed by:

```bash
$ sudo easy_install networkx
$ sudo easy_install matplotlib
```

For MacOSX: if installing matplotlib fails, use MacPorts, `sudo port
install py-matplotlib`.
For Ubuntu: if it fails, use APT, `sudo apt-get install python-matplotlib`.

### Quick install

Just type:

```bash
$ sudo easy_install graphillion
```

and an attempt will be made to find and install an appropriate version
that matches your operating system and Python version.

For FreeBSD: Graphillion can also be installed by [FreeBSD
Ports](http://www.freshports.org/math/py-graphillion/).

### Installing on Windows with Anaconda

1. Download and install Anaconda (fetch
Anaconda-2.1.0-Windows-x86_64.exe from
http://repo.continuum.io/archive/, since the later versions are not
tested)
2. Add the following paths to the %PATH% variable (Your Anaconda paths may be different):
```
set PATH=%PATH%;C:\Anaconda;C:\Anaconda\Scripts;C:\Anaconda\DLLs;C:\Anaconda\MinGW\bin;C:\Anaconda\MinGW\x86_64-w64-mingw32\lib
```
3. Download the source (tar.gz or zip file) from Github (Do not use the Pypi tarball):
   https://github.com/takemaru/graphillion
4. Run IPython
5. Change directory to the source directory
   (it should have the file setup.py)
6. Run ```run setup.py build``` to build
7. (optional) Run ```run setup.py test -q``` to execute the tests
8. Run ```run setup.py install``` to install

If you have the wheel package installed you can create a redistributable wheel with:
```run setup.py bdist_wheel```
or an egg with:
```run setup.py bdist_egg```

### Installing from source

You can install from source by downloading a source archive file
(tar.gz or zip) or by checking out the source files from the GitHub
source code repository.

#### Source archive file

1. Download the source (tar.gz or zip file) from
   https://github.com/takemaru/graphillion
2. Unpack and change directory to the source directory (it should have
   the file setup.py)
3. Run `python setup.py build` to build
4. (optional) Run `python setup.py test -q` to execute the tests
5. Run `sudo python setup.py install` to install

#### GitHub repository

1. Clone the Graphillion repository
   `git clone https://github.com/takemaru/graphillion.git`
2. Change directory to "graphillion"
3. Run `python setup.py build` to build
4. (optional) Run `python setup.py test -q` to execute the tests
5. Run `sudo python setup.py install` to install

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
location.  See http://docs.python.org/inst/search-path.html for
further details.


Tutorial
--------------------------------------------------------------------------------

If you haven't seen our fun movie, [Time with class! Let's count!],
please watch it before beginning the tutorial.  This movie, which has
been watched more than a million times, will convince you of a need for
Graphillion.  The summary of this tutorial is also provided as a
movie, [Graphillion: Don't count naively].

We believe that you enjoyed the movies and understood the necessity
and features of Graphillion.  Now, let's see Graphillion in more
detail.

We first introduce terminology used in Graphillion, as follows:

| Term          | Description                   | Example                                          |
|:--------------|:------------------------------|:-------------------------------------------------|
| vertex        | any hashable object           | `1`, `'v1'`, `(x, y)`                            |
| edge          | tuple of vertices             | `(1, 2)`                                         |
| weighted edge | tuple of vertices with weight | `(1, 2, -1.5)`                                   |
| graph         | list of (weighted) edges      | `[(1, 2, -1.5), (1, 3)]`                               |
| set of graphs | GraphSet object               | `GraphSet([[(1, 2), (1, 3)], [(1, 2), (2, 3)]])` |

Vertices (or nodes) can be any hashable object; e.g., a number, a text
string, etc.  Edges (or links) are defined as a pair of vertices, and
a graph is a list of edges; currently, Graphillion supports
*undirected* graphs only.  A GraphSet object stores a set of graphs.

Before anything else, we start the Python interpreter and import
Graphillion and a helper module; the latter provides some functions
like graph creation and drawing for the tutorial.

```python
$ python
>>> from graphillion import GraphSet
>>> import graphillion.tutorial as tl  # helper functions just for the tutorial
```

### Paths on a grid graph

At the beginning, we define our *universe*.  The universe can be any
graph, and a graph handled by Graphillion must be a subgraph of this
graph.  In this tutorial, we use the 8x8 grid graph as our universe
(the graph size should be regarded as 9x9, but we follow the
definition in the movie).

```python
>>> universe = tl.grid(8, 8)
>>> GraphSet.set_universe(universe)
>>> tl.draw(universe)  # show a pop-up window of our universe
```

![A grid graph](https://github.com/takemaru/graphillion/blob/master/doc/fig1.png?raw=true)

We find all the simple paths between the opposing corners; it took
four hours with the supercomputer in the movie.

```python
>>> start = 1
>>> goal = 81
>>> paths = GraphSet.paths(start, goal)
>>> len(paths)  # or paths.len() for very large set
3266598486981642
```

It's very quick, isn't it?  (If you get 980466698, check whether your
machine is 32-bit; Graphillion requires 64-bit machines.)  Since the
`paths` object contains all the paths, you can enumerate them one by
one.

```python
>>> for path in paths:
...     path
... # stop by Ctrl-C because it'll take years
>>> tl.draw(paths.choice())  # show one of the paths
```

![A path from start to goal](https://github.com/takemaru/graphillion/blob/master/doc/fig2.png?raw=true)

Next, in order to demonstrate the filtering or search capability of
Graphillion, we choose paths with given conditions.  Let's assume that
a treasure box and its key are placed on the grid as shown in the
figure.

![Key and treasure box](https://github.com/takemaru/graphillion/blob/master/doc/fig3.png?raw=true)

We consider all paths on which the key is picked up before reaching
the treasure box.  We're not allowed to pass thorugh the same place
twice.  First, search for the paths to the key not through the
treasure box, and then select the paths including the key's paths and
the treasure box.

```python
>>> key = 64
>>> treasure = 18
>>> paths_to_key = GraphSet.paths(start, key).excluding(treasure)  # paths to the key not through the treasure box
>>> treasure_paths = paths.including(paths_to_key).including(treasure)  # paths to goal via the key and treasure box
>>> len(treasure_paths)
789438891932744
>>> tl.draw(treasure_paths.choice())  # show one of the paths
```

![A path on which the box is opened](https://github.com/takemaru/graphillion/blob/master/doc/fig4.png?raw=true)

Test if all the treasure paths are a subset of the original paths,
which connect between the corners.

```python
>>> treasure_paths < paths  # "<" means "subset-of" in Graphillion
True
```

We conduct statistical processing with random sampling.  Graphillion
enables you to choose a sample (a graph) from the graphset uniformly
randomly.  Draw a histogram of "how many turns on the treasure paths"
as follows:

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

![Histogram of turn counts](https://github.com/takemaru/graphillion/blob/master/doc/fig5.png?raw=true)

The histogram shows that we make a turn at a corner usually 30-50
times through a single path.  Without Graphillion, it would be very
hard to investigate such a complicated property for a very large set
with 10^14 paths.  We also find that the shortest path involves only
five turns, which is derived by method `min_iter()`, an optimizer
provided by Graphillion.

```python
>>> for path in treasure_paths.min_iter():
...     tl.how_many_turns(path)
...     break  # if not break, multiple paths can be yielded in the ascending order
...
5
```

As an application of path enumeration, you'll find [Ekillion] very
interesting, which enumerates all JR train paths in Japan metropolitan
and suburbs from startpoint to endpoint.

### Power flows on a distribution network

Graphillion works on any graphs other than square grids, and handles
other subgraphs than simple paths.  Next, we consider a power
distribution network in the figure.  In this network, we asuume that a
vertex is a house and an edge is a power line with a switch.  The
power is provided from the four generators at corners.

```python
>>> universe = tl.grid(8, 8, 0.37)  # 37 % of edges are removed from 8x8 grid
>>> GraphSet.set_universe(universe)
>>> tl.draw(universe)
```

![A power distribution network](https://github.com/takemaru/graphillion/blob/master/doc/fig6.png?raw=true)

The power flow is determined by configuring switches, which are placed
on each line.  If a switch is closed (an edge exists on a graph), the
power is transmitted on the line; otherwise, not.  The power must be
transmitted to all houses, while the flow must not have a loop to
protect against short circuit.  The power flow, hence, must form a
*forest*, a set of trees, rooted at generators.  We find all of such
forests as follows:

```python
>>> generators = [1, 9, 73, 81]
>>> forests = GraphSet.forests(roots=generators, is_spanning=True)  # a forest represents a power flow covering all houses without loop
>>> len(forests)
54060425088
>>> tl.draw(forests.choice())
```

![An unsafe power flow](https://github.com/takemaru/graphillion/blob/master/doc/fig7.png?raw=true)

The amount of power transmitted from a single generator should be
strictly restricted, so as not to exceed the capacity.  The forest
shown above may have a very large tree, which implies that the
generator sends too much power beyond its capacity.  Here, we assume
that each generator is allowed to provide power to less than 23
houses.  We first find all dangerous cases of too much power, and then
select safe flows without the dangerous cases.

```python
>>> too_large_trees = GraphSet()  # empty graphset
>>> for substation in generators:
...     too_large_trees |= GraphSet.trees(root=substation).larger(23)  # unsafe power flows
...
>>> safe_forests = forests.excluding(too_large_trees)  # power flows without the unsafe ones
>>> len(safe_forests)
294859080
>>> tl.draw(safe_forests.choice())
```

![A safe power flow](https://github.com/takemaru/graphillion/blob/master/doc/fig8.png?raw=true)

Since we found all the safe flows, we try to change the network from
the current configuration to a safe one using an optimization
technique.  The current configuration is given by:

```python
>>> closed_switches = (forests - safe_forests).choice()  # sets of closed switches in unsafe power flows
>>> tl.draw(closed_switches)
```

![Current unsafe configuration](https://github.com/takemaru/graphillion/blob/master/doc/fig9.png?raw=true)

New configuration must be one of the safe flows, and must be realized
with least switch operations.  We put a *score* (edge weight) on a new
switch status if it is inconsistent with the current status, as shown
in the table.

| current \ next | open | closed |
|:---------------|-----:|-------:|
| open           |    0 |     -1 |
| closed         |    0 |      1 |

```python
>>> scores = {}  # scores for closed switches in the new configuration (default is 0)
>>> for switch in universe:
...     # if current status is closed then the score is 1, else -1
...     scores[switch] = 1 if switch in closed_switches else -1
...
```

We try to find a new configuration (forest) with a maximum score.  The
configuration has a maximum score and can be realized with the least
switch operations.  Compare it with the current configuration above,
and you'll find them quite alike; only eight switch operations are
required from the terrible unsafe configuration to a safe one.

```python
>>> for forest in safe_forests.max_iter(scores):
...     tl.draw(forest)
...     break  # if not break, multiple configs are yielded from the highest score
...
```

![Similar but safe configuration](https://github.com/takemaru/graphillion/blob/master/doc/fig10.png?raw=true)

Finally, we investigate serious failures that prevent the safe power
delivery.  We search for minimal blocking sets, or minimal hitting
sets more generally, to study such failures.  A hitting set is roughly
defined such that all the given sets are *hit* by at least one element
in the hitting set; e.g., given {1, 2}, {2, 3}, and {3}, minimal
hitting sets are {1, 3} and {2, 3}.  A hitting set indicates a
critical failure pattern; if power lines in a hitting set are broken,
all the flows can't be configured.

```python
>>> failures = safe_forests.blocking().minimal()  # a set of all minimal blocking sets
```

To help your understanding, remove all lines in a hitting set from the
network, and you'll find no safe flow.

```python
>>> failure = failures.choice()  # a hitting set (a set of critical power lines)
>>> for line in failure:
...     safe_forests = safe_forests.excluding(line)  # remove a line in the hitting set
...
>>> len(safe_forests)
0
```

Small hitting sets (e.g., less than five lines) might imply
vulnerability of the network.  We now find 767 small failure patterns,
which should be investigated carefully.

```python
>>> len(failures.smaller(5))
767
```

Though actual power distribution networks are much more complicated,
we basically rely on the same idea in the study of power distribution
networks.  Our power loss minimization tool, which optimizes a
network with nonlinear objective function with nonconvex constraints,
is available online at [DNET].


Creating graphsets
--------------------------------------------------------------------------------

Graphillion provides three ways to create a GraphSet object; with a
graph list, edge constraints, and graph types like paths and trees.

Please don't forget to set the universe before working with GraphSet,
as mentioned in [tutorial].  We use the following universe in this
section.

```python
>>> from graphillion import GraphSet
>>> universe = [(1, 2), (1, 4), (2, 3), (2, 5), (3, 6), (4, 5), (5, 6)]
>>> GraphSet.set_universe(universe)
```

### Graph list

This is the most straight-forward way to create a GraphSet object.
Specify a list of graphs and get an object with the graphs.

In the following example, two graphs, one has a single edge and the
other has two edges, are given.  A GraphSet object with the two graphs
is created.

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

Edge constraints specify edges to be included or not included in the
object.  These constraints must be represented by a dict of included
or excluded edge lists.  Edges not specified in the dict are
"don't-care"; they can be included and excluded in the object.

In the following example, edge (1, 4) is included while edges (1, 2)
and (2, 3) aren't.

```python
>>> edges1 = [(1, 4)]
>>> edges2 = [(1, 2), (2, 3)]
>>> GraphSet({'include': edges1, 'exclude': edges2})
GraphSet([[(1, 4)], [(1, 4), (2, 5)], [(1, 4), (3, 6)], ...
```

An empty dict `{}` means that no constraint is specified, and so a
GraphSet including all possible graphs in the universe is returned
(let N the number of edges in the universe, 2^N graphs are stored in
the new object).

```python
>>> gs = GraphSet({})
>>> len(gs)
128  # 2^7
```

### Graph types

You can specify a graph type, such as paths and trees, and create a
GraphSet object that stores all graphs matching the type.  Graphillion
supports the following graph types:

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

The arguments are defined for each type, please see the library
reference in detail.

Graphillion also provides low-level interface `graphs()` to specify
more complicated graph types; actually, the specific methods call this
low-level interface internally.  The following example is the same
with `paths(1, 6)`.

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

If these methods are called as object methods, like `gs.paths(1, 6)`,
graphs are selected only from the GraphSet object.  Please see the
library reference for more detail.  The internal implementation of
`graphs()` is independently available as [TdZdd].

Manipulating graphsets
--------------------------------------------------------------------------------

Graphillion provides many operations to manipulate graphs in a
GraphSet object.  These operations are classified into selection,
modification, and comparison; some of them are derived from Python's
set methods.  Graphillion also provides some iterators and
serialization.  Please see the library reference for details of each
method.

### Selection methods

The following methods select graphs from a given GraphSet object (or
two given GraphSet objects if binary operation).  No new graphs are
generated during the operation.

| Method                                            | Description                                                                                  |
| :------------------------------------------------ | :------------------------------------------------------------------------------------------- |
| `gs.union(other(s))`, `gs (pipe) other`           | Returns a new GraphSet with graphs from `gs` and all others                                |
| `gs.intersection(other(s))`, `gs & other`         | Returns a new GraphSet with graphs common to `gs` and all others                           |
| `gs.difference(other(s))`, `gs - other`           | Returns a new GraphSet with graphs in `gs` that are not in the others                      |
| `gs.symmetric_difference(other(s))`, `gs ^ other` | Returns a new GraphSet with graphs in either `gs` or `other` but not both                  |
| `gs.update(other(s))`                             | Updates `gs`, adding graphs from all others                                                |
| `gs.including(obj)`                               | Returns a new GraphSet that includes supergraphs of `obj` (graphset, graph, edge, or vertex) |
| `gs.excluding(obj)`                               | Returns a new GraphSet that doesn't include `obj`  (graphset, graph, edge, or vertex)        |
| `gs.included(obj)`                                | Returns a new GraphSet with subgraphs of a graph in `obj` (graphset or graph)                |
| `gs.larger(size)`                                 | Returns a new GraphSet with graphs that have more than `size` edges                          |
| `gs.smaller(size)`                                | Returns a new GraphSet with graphs that have less than `size` edges                          |
| `gs.graph_size(size)`                             | Returns a new GraphSet with `size` edges                                                     |
| `gs.minimal()`                                    | Returns a new GraphSet of minimal graphs                                                     |
| `gs.maximal()`                                    | Returns a new GraphSet of maximal graphs                                                     |

Creation methods specifying graph types also work as selection methods.

| Method                                        | Description                                            |
| :-------------------------------------------- | :----------------------------------------------------- |
| `gs.graphs(constraints)`                      | Returns a GraphSet with graphs under given constraints |
| `gs.connected_components(vertices)`           | Returns a GraphSet of connected components           |
| `gs.cliques(k)`                               | Returns a GraphSet of k-cliques                      |
| `gs.trees(root, is_spanning)`                 | Returns a GraphSet of trees                          |
| `gs.forests(roots, is_spanning)`              | Returns a GraphSet of forests, sets of trees         |
| `gs.cycles(is_hamilton)`                      | Returns a GraphSet of cycles                         |
| `gs.paths(terminal1, terminal2, is_hamilton)` | Returns a GraphSet of paths                          |

### Modification or generation methods

The following methods generate new graphs.  Some methods modify graphs
stored in `gs` (`self`), while others return a GraphSet with the newly
generated graphs.

#### Modifying graphs in gs (self)

| Method                              | Description                                                              |
| :---------------------------------- | :----------------------------------------------------------------------- |
| `gs.add(graph_or_edge)`             | Adds a given graph to `gs`, or grafts a given edge to graphs in `gs` |
| `gs.remove(obj)`, `gs.discard(obj)` | Removes a given graph, edge, or vertex from `gs`                       |
| `gs.flip(edge)`                     | Flips the state of a given edge over all graphs in `gs`                |
| `gs.clear()`                        | Removes all graphs from `gs`                                           |

#### Generating new graphs

| Method            | Description                                             |
| :---------------- | :------------------------------------------------------ |
| `~gs`             | Returns a new GraphSet with graphs not stored in `gs` |
| `gs.complement()` | Returns a new GraphSet with complement graphs of `gs` |
| `gs.blocking()`   | Returns a new GraphSet of all blocking (hitting) sets   |
| `gs.hitting()`    | Same as `gs.blocking()` |

### Comparison and evaluation methods

The following methods provide comparison or evaluation for GraphSet
objects.

| Method                          | Description                                                                      |
| :------------------------------ | :------------------------------------------------------------------------------- |
| `gs.isdisjoint(other)`          | Returns True if `gs` has no graphs in common with `other`                      |
| `gs.issubset(other)`            | Tests if every graph in `gs` is in `other`                                     |
| `gs.issuperset(other)`          | Tests if every graph in `other` is in `gs`                                     |
| `obj in gs`                     | Returns True if `obj` (graph, edge, or vertex) is in the `gs`, False otherwise |
| `len(gs)`, `gs.len()`           | Returns the number of graphs in `gs`                                           |
| `gs.probability(probabilities)` | Returns the probability of `gs` with given `probabilities`.                    |

### Iterators

Graphillion provides various iterators.  `rand_iter()` can be used for
random sampling in statistical analysis.  `min_iter()` and
`max_iter()` can be used as optimizers, and they yield not just an
optimal graph but top-k graphs.  `pop()` and `choice()` return a graph
in the GraphSet object, though they aren't iterators.

| Method           | Description                                             |
| :--------------- | :------------------------------------------------------ |
| `iter(gs)`       | Iterates over graphs                                    |
| `gs.rand_iter()` | Iterates over graphs uniformly randomly                 |
| `gs.min_iter()`  | Iterates over graphs in the ascending order of weights  |
| `gs.max_iter()`  | Iterates over graphs in the descending order of weights |
| `gs.pop()`       | Removes and returns an arbitrary graph from `gs`      |
| `gs.choice()`    | Returns an arbitrary graph from `gs`                  |

### Dumping and loading methods

Graphillion allows you to dump a graphset to a file, and to load it
from the file.  Dumping and loading operations must be done together
with pickling the universe; see the library reference in detail.

| Method              | Description                                         |
| :------------------ | :-------------------------------------------------- |
| `gs.dump(fp)`       | Serialize `gs` to a file `fp`                     |
| `GraphSet.load(fp)` | Deserialize a file `fp` and return the new GraphSet |

### Python's set methods

Graphillion supports Python's set methods.  These methods treat a
graph just as an element of the set and don't care the graph
structure.

- `gs.union(other)`, `gs | other`,
- `gs.intersection(other)`, `gs & other`,
- `gs.difference(other)`, `gs - other`,
- `gs.symmetric_difference(other)`, `gs ^ other`,
- `gs.update(other)`, `gs |= other`,
- `gs.add(graph)`,
- `gs.remove(graph)`, `gs.discard(graph)`,
- `gs.clear()`,
- `gs.isdisjoint(gs)`,
- `gs.issubset(gs)`,
- `gs.issuperset(gs)`,
- `graph in gs`,
- `len(gs)`,
- `gs.pop()`, and
- `gs.copy()`.


Working with NetworkX
--------------------------------------------------------------------------------

Graphillion transparently works with existing graph tools like
[NetworkX].  Any object like `networkx.Graph` can be recognized as a
graph in Graphillion, while an edge list is a graph by default.

Define two methods that associate a new graph object with an edge
list; one method is used for converting an edge list into a graph
object, and the other is vice versa.  We show an example for NetworkX.

```python
>>> import networkx as nx
>>> GraphSet.converters['to_graph'] = nx.Graph
>>> GraphSet.converters['to_edges'] = nx.Graph.edges
```

We can now pass NetworkX's graph objects to Graphillion like this.

```python
>>> g = nx.Graph(...)  # create a graph by NetworkX
>>> GraphSet.set_universe(g)
```

We also receive NetworkX's graph objects from Graphillion.

```python
>>> gs.choice()  # return a NeworkX's graph object
<networkx.classes.graph.Graph object at 0x100456d10>
```

For visualizing graphs, NetworkX provides an interface to Matplotlib
plotting package along with several node positioning algorithms.

```python
>>> nx.draw(gs.choice())
>>> import matplotlib.pyplot as plt
>>> plt.show()  # show a pop-up window
```

Library reference
--------------------------------------------------------------------------------

The library reference can be browsed using pydoc in your terminal
window:

```bash
$ pydoc graphillion.GraphSet
```

Or in HTML:
```bash
$ pydoc -w graphillion.GraphSet
```

Example codes
--------------------------------------------------------------------------------

Example codes are found [here](http://github.com/takemaru/graphillion/wiki/Example-codes).

Future work
--------------------------------------------------------------------------------

- More efficient internal data conversion
- More efficient search algorithms for optimization
- Nonlinear objective functions in optimization
- More efficient algorithms for hitting sets and cliques
- Sync the internal random seed with Python's random
- Documentation on performance
- Multithreading
- Garbage collections
- Mailing lists
- Coding rules
- Deveroper documents

References
--------------------------------------------------------------------------------

- JST ERATO Minato Discrete Structure Manipulation System Project,
  "High-Speed Graph Enumeration Algorithms," Morikita Publishing, April
  2015. ([amazon](http://www.amazon.co.jp/dp/4627852614), in Japanese)
- Jun Kawahara, "graphillion: Python library handling a huge number of
  subgraphs," Python Seminer, December
  2014. ([pdf](http://www.logopt.com/download/graph.pdf), in Japanese)
- Takeru Inoue, Hiroaki Iwashita, Jun Kawahara, and Shin-ichi Minato:
  "Graphillion: Software Library Designed for Very Large Sets of
  Labeled Graphs," International Journal on Software Tools for
  Technology Transfer, Springer, vol.18, issue 1, pp.57-66, February 2016. ([pdf](http://dx.doi.org/10.1007/s10009-014-0352-z))
- Takeru Inoue, Norihito Yasuda, Shunsuke Kawano, Yuji Takenobu,
  Shin-ichi Minato, and Yasuhiro Hayashi, "Distribution Network
  Verification for Secure Restoration by Enumerating All Critical
  Failures," IEEE Transactions on Smart Grid, October 2014. ([pdf](http://dx.doi.org/10.1109/TSG.2014.2359114))
- Jun Kawahara, Takeru Inoue, Hiroaki Iwashita and Shin-ichi Minato,
  "Frontier-based Search for Enumerating All Constrained Subgraphs
  with Compressed Representation," Hokkaido University, Division of
  Computer Science, TCS Technical Reports, TCS-TR-A-14-76, September 2014. ([pdf](http://www-alg.ist.hokudai.ac.jp/~thomas/TCSTR/tcstr_14_76/tcstr_14_76.pdf))
- Takeru Inoue, "Graphillion updates," JST ERATO Minato project
  seminar, February 2014.  ([pdf](http://www-erato.ist.hokudai.ac.jp/docs/seminar/20140226-erato-pub.pdf))
- Takeru Inoue, Keiji Takano, Takayuki Watanabe, Jun Kawahara, Ryo
  Yoshinaka, Akihiro Kishimoto, Koji Tsuda, Shin-ichi Minato, and
  Yasuhiro Hayashi, "Distribution Loss Minimization with Guaranteed
  Error Bound," IEEE Transactions on Smart Grid, vol.5, issue.1,
  pp.102-111, January 2014.
  ([pdf](http://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=6693788))
- Hiroaki Iwashita and Shin-ichi Minato, "Efficient Top-Down ZDD
  Construction Techniques Using Recursive Specifications," TCS
  Technical Reports, TCS-TR-A-13-69, December 2013. ([pdf](http://www-alg.ist.hokudai.ac.jp/~thomas/TCSTR/tcstr_13_69/tcstr_13_69.pdf))
- Takeru Inoue, "Graphillion: Python module for very large sets of graphs,"
  PyCon APAC, September 2013.

    [![][pycon-thumbnail]][PyCon]
- Takeru Inoue, "[Invited Talk] Graphillion: Software Library for Very
  Large Sets of Graphs," Technical Report of IEICE, vol.113, no.140,
  IN2013-43, pp.43-47, July
  2013. ([pdf](https://sites.google.com/site/takeruinoue/home/publications/inoue13graphillion-ja.pdf?attredirects=0),
  in Japanese)
- Ryo Yoshinaka, Toshiki Saitoh, Jun Kawahara, Koji Tsuruma, Hiroaki
  Iwashita, and Shin-ichi Minato, "Finding All Solutions and Instances
  of Numberlink and Slitherlink by ZDDs," Algorithms 2012, 5(2),
  pp.176-213, 2012.  ([doi](http://dx.doi.org/10.3390/a5020176))
- [DNET - Distribution Network Evaluation Tool][DNET]
- [Ekillion - Enumerating all JR train paths in Japan][Ekillion]
- [GGCount - A C++ program for fast computation of the number of paths in a grid graph][GGCount]
- [TdZdd - A top-down/breadth-first decision diagram manipulation framework][TdZdd]
- [Graphillion for Windows](https://github.com/junkawahara/graphillion_for_windows)

[JST ERATO Minato project]: http://www-erato.ist.hokudai.ac.jp/?language=en
[DNET]: https://github.com/takemaru/dnet#dnet---distribution-network-evaluation-tool
[Ekillion]: http://www.nysol.jp/en/home/apps/ekillion
[NetworkX]: http://networkx.github.io/
[Time with class! Let's count!]: http://youtu.be/Q4gTV4r0zRs "Time with class! Let's count!"
[lets_count-thumbnail]: http://i.ytimg.com/vi/Q4gTV4r0zRs/default.jpg
[Graphillion: Don't count naively]: http://youtu.be/R3Hp9k876Kk "Graphillion: Don't count naively"
[dont_count-thumbnail]: http://i.ytimg.com/vi/R3Hp9k876Kk/default.jpg
[PyCon]: http://youtu.be/DjPyLSNxmqI "PyCon APAC 2013"
[pycon-thumbnail]: http://i.ytimg.com/vi/DjPyLSNxmqI/3.jpg
[tutorial]: #tutorial
[GGCount]: https://github.com/kunisura/GGCount
[TdZdd]: https://github.com/kunisura/TdZdd
[book]: http://www.amazon.co.jp/dp/4627852614
