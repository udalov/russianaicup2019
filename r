#!/usr/bin/env bash

set -e
trap "kill 0" EXIT

make -Cout -j4

score1=0
score2=0
for i in `seq 1 40`
do
    scripts/create-config.py Local Quick Simple $i 3600 --custom-properties >out/config.json
    out/aicup2019 &
    ./aicup2019 --batch-mode --log-level ERROR --config out/config.json --save-results out/result.txt
    read first second < <(scripts/parse-result.py)
    echo $i $first $second
    if (( $first > $second ));
    then
        score1=$(($score1 + 2))
    elif (( $first < $second ));
    then
        score2=$(($score2 + 2))
    else
        score1=$(($score1 + 1))
        score2=$(($score2 + 1))
    fi
done

score1=$(echo "scale=1;$score1/2.0" | bc -l)
score2=$(echo "scale=1;$score2/2.0" | bc -l)
echo "result $score1 $score2"

wait
