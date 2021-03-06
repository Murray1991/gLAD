#!/bin/bash

[ ! $# -ge 3 ] && echo "Usage: ./tests.sh K TestCase File [record|stat]" && exit 1

#perf directory
DIR="./perf"

#perf record
PERF_REC="perf record"
FREQ="-F 97"
PERF_OPTS="-g --call-graph dwarf $FREQ -e cycles,instructions:u,L1-dcache-loads,L1-dcache-loads-misses,cache-references,cache-misses -o"
PERF="$PERF_REC $PERF_OPTS"
PERF1="$PERF $DIR/perf.record.tst1.data"
PERF2="$PERF $DIR/perf.record.tst2.data"
PERF3="$PERF $DIR/perf.record.tst3.data"
PERF4="$PERF $DIR/perf.record.tst4.data"

#perf stat
PERF_STAT="perf stat"
PERF_OPTS_STAT="-e instructions:u,L1-dcache-loads,L1-dcache-loads-misses,cache-references,cache-misses "

#parameters
K=$1
QUER=$2
FILE=$3
ARG=$4

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
    T=$( $EXE1 )
    echo "[tst1] $T"
    T=$( $EXE2 )
    echo "[tst2] $T"
    T=$( $EXE3 )
    echo "[tst3] $T"
    T=$( $EXE4 )
    echo "[tst4] $T"
}

execute $QUER
