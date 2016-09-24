#!/bin/bash

BIN1="./build/top_k_queries.tst1.bin 5"
QUER="./test/test_cases/big_queries.txt"
FILE="./data/enwiki-20160601-all-titles.nozero.unique.1.sdsl"

perf record $BIN1 $QUER $FILE
