#!/bin/bash

#perf stuff
PERF_OPTS="-g --call-graph dwarf -F 90 -e cycles,instructions:u,L1-dcache-loads,L1-dcache-loads-misses,cache-references,cache-misses -o"
PERF_REC="perf record"
PERF="$PERF_REC $PERF_OPTS"

#perf's files for report
DIR="./perf"
FILE_TST1="$DIR/perf.record.tst1.data"
FILE_TST2="$DIR/perf.record.tst2.data"
FILE_TST3="$DIR/perf.record.tst3.data"

#perf1 & perf2
PERF1="$PERF $FILE_TST1"
PERF2="$PERF $FILE_TST2"
PERF3="$PERF $FILE_TST3"

#test's executables and parameters
BIN1="./build/top_k_queries.tst1.bin 5"
BIN2="./build/top_k_queries.tst2.bin 5"
BIN3="./build/top_k_queries.tst3.bin 5"
QUER="./test/test_cases/medium_queries.txt"
QUER2="./test/test_cases/big_queries.txt"
FILE="./data/enwiki-20160601-all-titles"
SUF1="1.sdsl"
SUF2="2.sdsl"
SUF3="3.sdsl"

arg=$1
[ -n "$arg" ] && PERF1="" && PERF2="" && PERF3="" && PERF_STAT="" && PERF_OPTS_STAT=""

#profile the tests...
EXE1="$PERF1 $BIN1 $QUER $FILE.$SUF1"
echo "$EXE1 ..."; $EXE1
EXE2="$PERF2 $BIN2 $QUER $FILE.$SUF2"
echo "$EXE2 ..."; $EXE2
EXE3="$PERF3 $BIN3 $QUER $FILE.$SUF3"
echo "$EXE3 ..."; $EXE3

#perf stat
PERF_OPTS_STAT="-e instructions:u,L1-dcache-loads,L1-dcache-loads-misses,cache-references,cache-misses "
PERF_STAT="perf stat"
STAT_EXE1="$PERF_STAT $PERF_OPTS_STAT $BIN1 $QUER $FILE.$SUF1"
STAT_EXE2="$PERF_STAT $PERF_OPTS_STAT $BIN2 $QUER $FILE.$SUF2"
STAT_EXE3="$PERF_STAT $PERF_OPTS_STAT $BIN3 $QUER $FILE.$SUF3"
