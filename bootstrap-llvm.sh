#!/bin/bash

set -e

version=$1
shift

mkdir "$1"
cd "$1"

curl -L -O https://github.com/llvm/llvm-project/releases/download/llvmorg-$version/llvm-$version.src.tar.xz
curl -L -O https://github.com/llvm/llvm-project/releases/download/llvmorg-$version/clang-$version.src.tar.xz
curl -L -O https://github.com/llvm/llvm-project/releases/download/llvmorg-$version/compiler-rt-$version.src.tar.xz

tar xf llvm-$version.src.tar.xz
tar xf clang-$version.src.tar.xz
tar xf compiler-rt-$version.src.tar.xz

cmake_args=(
    -DCMAKE_BUILD_TYPE=Debug
    -DLLVM_EXTERNAL_CLANG_SOURCE_DIR="$PWD"/clang-$version.src
    -DLLVM_EXTERNAL_COMPILER_RT_SOURCE_DIR="$PWD"/compiler-rt-$version.src
)
mkdir llvm-$version.src/build
cd llvm-$version.src/build
cmake "${cmake_args[@]}" ..
cmake --build . -- -j 4
cmake -DCMAKE_INSTALL_PREFIX=$HOME/llvm-$version -P cmake_install.cmake
