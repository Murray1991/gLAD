#!/bin/bash

DIR="./perf"
BIN1="./build/top_k_queries.tst1.bin 5"
BIN2="./build/top_k_queries.tst2.bin 5"
QUER="./test/test_cases/medium_queries.txt"
FILE="./data/enwiki-20160601-all-titles"
SUF1="1.sdsl"
SUF2="2.sdsl"

perf record -g -e cycles --output=$DIR/perf.record.tst1.data $BIN1 $QUER $FILE.$SUF1
perf record -g -e cycles --output=$DIR/perf.record.tst2.data $BIN2 $QUER $FILE.$SUF2
