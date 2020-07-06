#!/bin/bash

set -ex

root=$(
    cd "$(dirname "$0")" &&
    pwd -P
)

case $(uname -s) in
    Darwin)
        ext=dylib
        ;;
    *)
        ext=so
        ;;
esac

for file ; do
    opt -load "$root"/build/BBPrinter/LLVMBBPrinter.$ext -bbprinter -o /dev/null "$file"
done
#dot -O -Tpdf *.gv
