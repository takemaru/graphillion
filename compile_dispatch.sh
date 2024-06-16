#!/bin/bash

# This script runs clang for c++ if a *.cc file is given as an argument.
# Otherwise it just runs clang.
# This script will be used by setup.py in macOS environment.
# 
# ./compile_dispatch.sh source.cc <args>
# executes
# clang -x c++ source.cc <args> -std=c++17 -lstdc++
# 
# ./compile_dispatch.sh source.c <args>
# executes
# clang source.c <args>

# initialization
ARGS=()
USE_CXX11=false

# check whether the name of the source code is *.cc
for arg in "$@"; do
    if [[ "$arg" == *.cc ]]; then
        USE_CXX11=true
    fi
done

# add options for C++
if $USE_CXX11; then
    ARGS+=("-x")
    ARGS+=("c++")
fi

for arg in "$@"; do
    ARGS+=("$arg")
done

# add options for C++
if $USE_CXX11; then
    ARGS+=("-std=c++17")
    ARGS+=("-lstdc++")
fi

# run clang
/usr/bin/clang "${ARGS[@]}"

