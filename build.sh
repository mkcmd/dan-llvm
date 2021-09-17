#!/bin/sh

set -e

docker build --ulimit nofile=10000:10000 -t dan-llvm .
docker run -v ${PWD}:/work -t dan-llvm
