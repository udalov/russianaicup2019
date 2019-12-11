#!/usr/bin/env bash

shopt -s globstar

target=$(realpath out/solution.zip)
tmp=$(realpath out/solution-tmp)
base=$(realpath raic-2019/clients/cpp)

mkdir -p $(dirname target)
cd src

rm -rf cmake-build-debug $target $tmp
zip $target **/*.cpp **/*.hpp **/*.h CMakeLists.txt >/dev/null

<< EOF
mkdir -p $tmp/model
cp $base/Debug.* $tmp/
cp $base/Stream.* $tmp/
cp $base/TcpStream.* $tmp/
cp $base/model/PlayerMessageGame.* $tmp/model/
cp $base/model/PlayerView.* $tmp/model/
cp $base/model/ServerMessageGame.* $tmp/model/
cp $base/model/Versioned.* $tmp/model/
sed 's|game.units|game.world.units|' $base/main.cpp > $tmp/main.cpp
sed 's|result.data =|// result.data =|' $base/model/PlayerMessageGame.cpp > $tmp/model/PlayerMessageGame.cpp
sed 's|game\.|// game.|' $base/model/PlayerView.cpp > $tmp/model/PlayerView.cpp
sed 's|innerValue =|// innerValue =|' $base/model/Versioned.cpp > $tmp/model/Versioned.cpp
echo "#pragma once" > $tmp/model/Vec2Double.hpp
echo "#pragma once" > $tmp/model/Vec2Float.hpp

cd $tmp
zip $target **/*.cpp **/*.hpp >/dev/null
EOF
