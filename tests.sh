#!/bin/bash

#perf stuff
PERF_OPTS="-g --call-graph dwarf -F 90 -e cycles,instructions:u,L1-dcache-loads,L1-dcache-loads-misses,cache-references,cache-misses -o"
PERF_REC="perf record"
PERF="$PERF_REC $PERF_OPTS"

#perf's files for report
DIR="./perf"
FILE_TST1="$DIR/perf.record.tst1.data"
FILE_TST2="$DIR/perf.record.tst2.data"

#perf1 & perf2
PERF1="$PERF $FILE_TST1"
PERF2="$PERF $FILE_TST2"

#test's executables and parameters
BIN1="./build/top_k_queries.tst1.bin 5"
BIN2="./build/top_k_queries.tst2.bin 5"
QUER="./test/test_cases/medium_half_queries.txt"
QUER2="./test/test_cases/big_queries.txt"
FILE="./data/enwiki-20160601-all-titles"
SUF1="1.sdsl"
SUF2="2.sdsl"

#profile the tests...
#$PERF1 $BIN1 $QUER $FILE.$SUF1
#$PERF2 $BIN2 $QUER $FILE.$SUF2

#perf stat
PERF_OPTS_STAT="-e instructions:u,L1-dcache-loads,L1-dcache-loads-misses,cache-references,cache-misses "
PERF_STAT="perf stat"

$PERF_STAT $PERF_OPTS_STAT $BIN1 $QUER $FILE.$SUF1
$PERF_STAT $PERF_OPTS_STAT $BIN2 $QUER $FILE.$SUF2
