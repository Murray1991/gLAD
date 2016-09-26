#!/bin/bash

DIR="./perf"
BIN1="./build/top_k_queries.tst1.bin 5"
BIN2="./build/top_k_queries.tst2.bin 5"
QUER="./test/test_cases/medium_queries.txt"
FILE="./data/enwiki-20160601-all-titles.nozero.unique.1.sdsl"

perf record -g -e cycles --output=$DIR/perf.record.tst1.data $BIN1 $QUER $FILE

#perf record -g -e cycles --output=$DIR/perf.record.tst2.data $BIN2 $QUER $FILE

#perf record -b --output=$DIR/perf.record.tst1.b.data $BIN1 $QUER $FILE
#perf record -b --output=$DIR/perf.record.tst2.b.data $BIN2 $QUER $FILE
