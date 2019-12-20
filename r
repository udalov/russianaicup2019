#!/usr/bin/env bash

set -e
trap "kill 0" EXIT

PLAYER1=Local
PLAYER2=Local

make -Cout -j4

FROM=$1
TO=$2
[ $FROM ] || FROM=101
[ $TO ] || TO=200

score1=0
score2=0
for i in `seq $FROM $TO`; do
    scripts/create-config.py $PLAYER1 $PLAYER2 Simple $i 3600 --custom-properties >out/config.json
    out/aicup2019 &
    if [ "$PLAYER2" == "Local" ]; then
        out/prev 127.0.0.1 31002 &
    fi
    ./aicup2019 --batch-mode --log-level ERROR --config out/config.json --save-results out/result.txt
    read first second < <(scripts/parse-result.py)
    echo $i $first $second
    if (( $first > $second )); then
        score1=$(($score1 + 2))
    elif (( $first < $second )); then
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
