#!/bin/bash

set -e

version=$1
shift

mkdir "$1"
cd "$1"

curl -O http://releases.llvm.org/$version/llvm-$version.src.tar.xz
curl -O http://releases.llvm.org/$version/cfe-$version.src.tar.xz
curl -O http://releases.llvm.org/$version/compiler-rt-$version.src.tar.xz

tar xf llvm-$version.src.tar.xz
tar xf cfe-$version.src.tar.xz
tar xf compiler-rt-$version.src.tar.xz
mv cfe-$version.src llvm-$version.src/tools/clang
mv compiler-rt-$version.src llvm-$version.src/projects/compiler-rt

mkdir llvm-$version.src/build
cd llvm-$version.src/build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -- -j 4
cmake -DCMAKE_INSTALL_PREFIX=$HOME/llvm-$version -P cmake_install.cmake
