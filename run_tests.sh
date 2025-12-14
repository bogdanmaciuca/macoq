#!/usr/bin/sh

cd build
make

cd tests
ctest --output-on-failure

