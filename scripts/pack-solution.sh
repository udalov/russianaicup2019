#!/usr/bin/env bash

shopt -s globstar

cd src

mkdir -p ../out
rm -rf cmake-build-debug ../out/solution.zip
zip ../out/solution.zip **/*.cpp **/*.hpp **/*.h CMakeLists.txt
