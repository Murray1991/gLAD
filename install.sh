#!/bin/bash

mkdir external
cd external
git commit https://github.com/simongog/sdsl-lite.git
cd ..
./external/sdsl-lite/install.sh ./sdsl
