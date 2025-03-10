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
    opt -load-pass-plugin "$root"/build/BBPrinter/LLVMBBPrinter.$ext -passes=bbprinter -disable-output "$file"
done
#dot -O -Tpdf *.gv
