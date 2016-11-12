#!/bin/bash

[ ! $# -gt 0 ] && echo "Usage: ./testK.sh indexNumber queryFile file" && exit 1

function execute {
    K=$1; N=$2; Q=$3; I=$4.$2.sdsl;
    EXE="./build/top_k_queries.tst$N.bin $K $Q $I"
    $EXE
}

echo "### indexNumber=$1, file= queryFile=$2"
echo "### Output format: K  Time(us)    ResultsFound    SizeOfQueryFile     HashValue"
for K in 1 2 5 10 15 20 25 30 35 40 45 50
do
	execute $K $1 $2 $3
done
