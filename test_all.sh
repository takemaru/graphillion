#!/bin/sh
OMP_NUM_THREADS=1 python -m unittest discover -s graphillion/test -p "*.py" $@
