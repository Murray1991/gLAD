#!/bin/bash

[ ! $# -gt 0 ] && echo "Usage: ./testK.sh indexNumber queryFile file" && exit 1

function avg {
    res=$(printf $1 | awk '{ sum += $1; n++ } END { if (n > 0) print sum / n; }')
    echo $res
}

function execute {
    K=$1; N=$2; Q=$3; I=$4.$2.sdsl;col="";
    EXE="./build/top_k_queries.tst$N.bin $K $Q $I"
    for i in `seq 1 5`;
    do
        res=`$EXE`
        arr=($res);
        col="${arr[1]}\n$col"
        echo "# $res"
    done
    val=$(avg $col)
    echo "$K $val"
}

echo "### indexNumber=$1, file= queryFile=$2"
echo "### Output format: K  Time(us)    ResultsFound    SizeOfQueryFile     HashValue"
for K in 1 2 5 10 15 20 25 30 35 40 45 50
do
	execute $K $1 $2 $3
done
