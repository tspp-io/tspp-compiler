#!/bin/bash
set -e
cd ..
mkdir -p build && cd build
cmake ..
make
./tspp_interpreter ../src/tests/example.tspp
