#!/bin/sh

set -e

LLVM_CONFIG=/llvm-project/out/bin/llvm-config
LLVM_INCLUDE_DIR=$($LLVM_CONFIG --includedir)
LLVM_LIBS=$($LLVM_CONFIG --libfiles --link-static)
CLANG_LIBS=$(find /llvm-project/out/lib/ -name "libclang*\.a")

mkdir -p /work/build/tmp

# one static library to rule them all
cd /work/build/tmp
for LIB in $CLANG_LIBS $LLVM_LIBS;
do
        # some .cpp.o filenames collide when unpacking
        LIB_DIR=$(basename $LIB)
        mkdir -p $LIB_DIR && cd $LIB_DIR
        ar -x $LIB
        cd /work/build/tmp
done

$CXX /work/src/mega_clang.cpp -c -o /work/build/tmp/mega_clang.o -O2 -I $LLVM_INCLUDE_DIR
ar rcs /work/build/libmega_clang.a /work/build/tmp/mega_clang.o /work/build/tmp/*/*.o

# env var workaround for go
CC=/work/scripts/zcc
CXX=/work/scripts/zxx

cd /work
CGO_ENABLED=1 GOOS=linux GOARCH=amd64 go build -o clang-bpf-test main.go
