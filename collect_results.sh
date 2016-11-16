#!/bin/bash
INDEX="tst"
WHERE="i5"

[ ! $# -ge 1 ] && echo "Example: ./collect_results.sh data/italian-cities.txt" && exit 1

mkdir -p results
FILE=$1
F="${FILE##*/}"
TESTDIR=./test/test_cases/

for index in 1 2 3 4
do
    echo "Collecting results for $INDEX$index..."
    for test in `ls $TESTDIR`
    do
        echo "... for $TESTDIR$test ..."
        ./testK.sh $index $TESTDIR$test $FILE >> results/$INDEX$index.$WHERE.$F.dat
    done
done

