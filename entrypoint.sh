#!/bin/sh

cd /github/workspace
mkdir build
cd build
cmake -DBUILD_SHARED_LIBS=ON ..
cpack
