#!/usr/bin/env bash

shopt -s globstar
set -e

target=$(realpath out/solution.zip)

mkdir -p $(dirname target)
cd src

rm -rf cmake-build-debug $target
zip $target **/*.cpp **/*.hpp **/*.h CMakeLists.txt >/dev/null
