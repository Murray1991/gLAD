#!/bin/bash

DIRECTORY=./external

[ ! -d $DIRECTORY ] && mkdir $DIRECTORY
git clone https://github.com/simongog/sdsl-lite.git $DIRECTORY/sdsl-lite
$DIRECTORY/sdsl-lite/install.sh ./sdsl
rm -rf $DIRECTORY
