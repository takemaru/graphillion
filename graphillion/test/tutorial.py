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

from graphillion import GraphSet
import graphillion.tutorial as tl
import unittest
from sys import stderr

class TestTutorial(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_paths(self):
        try:
            universe = tl.grid(8, 8)
            GraphSet.set_universe(universe, traversal='bfs')

            start = 1
            goal = 81
            paths = GraphSet.paths(start, goal)
            if len(paths) == 980466698:
                stderr.write("Warning: Graphillion requires 64-bit machines, though your machine might be 32-bit.\n")
            self.assertEqual(len(paths), 3266598486981642)

            key = 64
            treasure = 18
            paths_to_key = GraphSet.paths(start, key).excluding(treasure)
            treasure_paths = paths.including(paths_to_key).including(treasure)
            self.assertEqual(len(treasure_paths), 789438891932744)

            self.assertTrue(treasure_paths < paths)

            i = 0
            data = []
            for path in treasure_paths.rand_iter():
                data.append(tl.how_many_turns(path))
                if i == 100: break
                i += 1

            for path in treasure_paths.min_iter():
                min_turns = tl.how_many_turns(path)
                break
            self.assertEqual(min_turns, 5)
        except ImportError:
            pass

    def test_forests(self):
        try:
            #universe = tl.grid(8, 8, 0.37)
            universe = [(1, 2), (1, 10), (2, 3), (2, 11), (3, 12), (4, 5), (5, 6), (5, 14), (6, 15), (7, 8), (8, 9), (8, 17), (9, 18), (10, 11), (11, 12), (11, 20), (13, 22), (14, 15), (14, 23), (16, 25), (17, 26), (18, 27), (19, 20), (19, 28), (20, 21), (21, 22), (21, 30), (22, 23), (22, 31), (23, 24), (24, 25), (25, 26), (26, 27), (26, 35), (27, 36), (28, 29), (28, 37), (29, 30), (29, 38), (30, 31), (31, 40), (32, 33), (32, 41), (33, 42), (34, 35), (37, 38), (37, 46), (38, 39), (40, 41), (41, 42), (41, 50), (42, 51), (43, 52), (44, 45), (45, 54), (46, 55), (47, 48), (47, 56), (48, 49), (49, 58), (50, 59), (51, 52), (51, 60), (52, 53), (53, 54), (53, 62), (55, 56), (56, 65), (57, 58), (57, 66), (59, 60), (59, 68), (61, 62), (61, 70), (62, 63), (64, 65), (64, 73), (65, 74), (66, 75), (67, 76), (68, 69), (69, 70), (69, 78), (70, 71), (70, 79), (71, 80), (72, 81), (74, 75), (75, 76), (76, 77), (80, 81)]
            GraphSet.set_universe(universe)

            generators = [1, 9, 73, 81]
            forests = GraphSet.forests(roots=generators, is_spanning=True)
            self.assertEqual(len(forests), 54060425088)

            too_large_trees = GraphSet()
            for substation in generators:
                too_large_trees |= GraphSet.trees(root=substation).larger(23)
            safe_forests = forests.excluding(too_large_trees)
            self.assertEqual(len(safe_forests), 294859080)

            closed_switches = (forests - safe_forests).choice()
            scores = {}
            for switch in universe:
                scores[switch] = 1 if switch in closed_switches else -1

            failures = safe_forests.blocking().minimal()
            self.assertEqual(len(failures), 1936)
            failure = failures.choice()
            for line in failure:
                safe_forests = safe_forests.excluding(line)
            self.assertEqual(len(safe_forests), 0)
        except ImportError:
            pass

if __name__ == '__main__':
    unittest.main()
