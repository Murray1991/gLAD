#!/bin/bash

DIRECTORY=./external

[ ! -d $DIRECTORY ] && mkdir $DIRECTORY
git clone https://github.com/simongog/sdsl-lite.git $DIRECTORY
./external/sdsl-lite/install.sh ./sdsl
