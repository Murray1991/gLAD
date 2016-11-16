#!/bin/bash

for i in 1 2 3 4 
do
    echo "building tst$i..."
    echo -e "\n.\n" | build/main.tst$i.bin $1 &> /dev/null
done
