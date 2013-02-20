Graphillion - A fast, lightweight graphset operation library
================================================================================

* [Overview](#overview "Overview")
* [Installing](#installing "Installing")
* [Tutorial](#tutorial "Tutorial")
* [Library reference](#library-reference "Library reference")
* [References](#references "References")

Overview
--------------------------------------------------------------------------------

Graphillion is a Python library for efficient *graphset operations*.
Unlike existing graph tools such as NetworkX, which are designed to
process just a single graph at a time, Graphilion manipulates a large
*set* of graphs with great efficiency.  Surprisingly, trillions of
trillions of graphs can be handled in a single computer.

You may be curious about an uncommon concept of *graphset*, but it
comes along with any graph or network when you consider multiple
subgraphs cut from a graph; e.g., comparing possible driving paths on
a road map, examining feasible electric flows on a power grid, or
evaluating the structure of chemical reaction networks.  The number of
such subgraphs can be more than googol, 10^100, even in a graph with
just a few hundreds of edges, since the number grows exponentially
with the graph size.  It takes millions of years to examine all
subgraphs with a naive approach as demonstrated in the following
tutorial; Graphillion is our answer to resolve this issue.

Graphillion allows you to exhaustively but efficiently search a given
graphset by specifying complex constraints, to find top-k optimal
graphs from a complicated (even nonconvex) set, and to extract common
properties seen among all graphs in a set.  Thanks to these features,
Graphillion has a variety of applications including graph database,
combinatorial optimization tools, and a graph structure analyzers.  We
will show some practical use cases in the following tutorial, such as
evaluation of power distribution networks.

Install Graphillion and go to the tutorial, and you'll find its power
and ubiquity.

Installing
--------------------------------------------------------------------------------

### Quick install

Just type:

```bash
$ easy_install graphillion
```

and an attempt will be made to find and install an appropriate version
that matches your operating system and Python version.

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

At the beginning, we define our universe.  A graph handled in
Graphillion must be a subgraph of the universal graph.


Library reference
--------------------------------------------------------------------------------

Library reference can be browsed using pydoc:

```bash
$ pydoc graphillion.GraphSet
```

References
--------------------------------------------------------------------------------
