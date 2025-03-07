#!/bin/bash

rm -rf build_sh
mkdir build_sh
cd build_sh
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release 
ninja 
