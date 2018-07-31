#!/bin/bash

set -ex

realpath () {
    # reimplementation of "readlink -fv" for darwin
    python -c "import os.path, sys; print os.path.realpath(sys.argv[1])" "$1"
}

case $(uname -s) in
    Darwin)
        ext=dylib
        ;;
    *)
        ext=so
        ;;
esac

root="$(dirname "$(realpath "$0")")"
for file ; do
    opt -load "$root"/build/BBPrinter/LLVMBBPrinter.$ext -bbprinter -o /dev/null "$file"
done
#dot -O -Tpdf *.gv
