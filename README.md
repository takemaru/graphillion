Graphillion - A fast, lightweight graphset operation library in Python
================================================================================

Overview
--------------------------------------------------------------------------------

Graphillion is a Python language software package for "graphset
operations".  Unlike existing graph tools such as networkx, which are
designed to process just a single graph at once, Graphilion
manipulates a large *set* of graphs with great efficiency.
Surprisingly, more than googol, 10^100, graphs can be handled in a
single computer in extreme cases.

Graphillion allows users to create a new graphset from the original
set by specifying complex constraints, to search for the optimal
graphs from the complicated (even nonconvex) set, and to extract
common elements found among all graphs in the set.  Thanks to these
features, Graphillion has a variety of use cases including a graph
database, a combinatorial optimization tool, and a graph structure
analyzer.  We will show some practical use cases in the tutorial, such
as evaluation of power distribution networks.

Installing
--------------------------------------------------------------------------------

### Quick install

Just type:

    $ pip install graphillion

and an attempt will be made to find and install an appropriate version
that matches your operating system and Python version.

### Requirements

#### Python

To use Graphillion, you need Python version 2.6 or later.
http://www.python.org/

#### GMP (The Gnu multiple precision arithmetic library) - optional

With GMP, Graphillion provides arbitrary precision in counting the
number of graphs in a set; otherwise double precision floating-point
numbers are used.  http://gmplib.org/

#### NetworkX and Matplotlib - used just in the tutorial

NetworkX and Matplotlib are used to create and draw a graph.  These
packages are not required in Graphillion, but just used in the
tutorial.  They can be installed by:

    $ pip install networkx
    $ pip install matplotlib

Tutorial
--------------------------------------------------------------------------------

At the beginning, we define our universe.  A graph handled in
Graphillion must be a subgraph of the universal graph.

References
--------------------------------------------------------------------------------
