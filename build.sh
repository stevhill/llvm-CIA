#!/bin/bash

set -e

realpath () {
    # reimplementation of "readlink -fv" for darwin
    python -c "import os.path, sys; print os.path.realpath(sys.argv[1])" "$1"
}

root="$(dirname "$(realpath "$0")")"
rm -rf "$root"/build
mkdir "$root"/build
cd "$root"/build
cmake -DCMAKE_PREFIX_PATH="$1" ..
cmake --build .
