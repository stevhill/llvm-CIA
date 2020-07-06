#!/bin/bash

set -e

root=$(
    cd "$(dirname "$0")" &&
    pwd -P
)

rm -rf "$root"/build
mkdir "$root"/build
cd "$root"/build
cmake -DCMAKE_PREFIX_PATH="$1" ..
cmake --build .
