#!/bin/sh
python -m unittest discover -s graphillion/test -p "*.py" $@
