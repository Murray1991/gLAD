#!/bin/bash

[ ! $# -gt 0 ] && echo "Usage: ./tests.sh K TestCase [record|stat]" && exit 1

#perf directory
DIR="./perf"

#perf record
PERF_REC="perf record"
PERF_OPTS="-g --call-graph dwarf -F 90 -e cycles,instructions:u,L1-dcache-loads,L1-dcache-loads-misses,cache-references,cache-misses -o"
PERF="$PERF_REC $PERF_OPTS"
PERF1="$PERF $DIR/perf.record.tst1.data"
PERF2="$PERF $DIR/perf.record.tst2.data"
PERF3="$PERF $DIR/perf.record.tst3.data"
PERF4="$PERF $DIR/perf.record.tst4.data"

#perf stat
PERF_STAT="perf stat"
PERF_OPTS_STAT="-e instructions:u,L1-dcache-loads,L1-dcache-loads-misses,cache-references,cache-misses "

#executables and parameters
FILE="./data/enwiki-20160601-all-titles"
K=$1
FILE=$2
ARG=$3

function execute {
    QUER=$1
    EXE1="./build/top_k_queries.tst1.bin $K $QUER $FILE.1.sdsl"
    EXE2="./build/top_k_queries.tst2.bin $K $QUER $FILE.2.sdsl"
    EXE3="./build/top_k_queries.tst3.bin $K $QUER $FILE.3.sdsl"
    EXE4="./build/top_k_queries.tst4.bin $K $QUER $FILE.4.sdsl"

    if [ "$ARG" == 'record' ]
    then
        EXE1="$PERF1 $EXE1"
        EXE2="$PERF2 $EXE2"
        EXE3="$PERF3 $EXE3"
        EXE4="$PERF4 $EXE4"
    elif [ "$ARG" == 'stat' ]
    then
        EXE1="$PERF_STAT $PERF_OPTS_STAT $EXE1"
        EXE2="$PERF_STAT $PERF_OPTS_STAT $EXE2"
        EXE3="$PERF_STAT $PERF_OPTS_STAT $EXE3"
        EXE4="$PERF_STAT $PERF_OPTS_STAT $EXE4"
    fi

    #tests...
    echo "$EXE1 ..."; $EXE1
    echo "$EXE2 ..."; $EXE2
    echo "$EXE3 ..."; $EXE3
    echo "$EXE4 ..."; $EXE4
}

execute $FILE
