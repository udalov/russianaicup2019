#!/usr/bin/env bash

set -e
trap "kill 0" EXIT

PLAYER1=Local
PLAYER2=Local

make -Cout -j4

from=$1
to=$2
[ $from ] || from=101
[ $to ] || to=200

WIDTH=20
score1=0
score2=0

for i in $(seq $from $to); do
    completed=$((i - from))
    outof=$((to - from + 1))
    progress=$((completed * WIDTH / outof))
    printf "$completed/$outof | $from ["
    printf "%0${progress}d" 0 | tr 0 '*'
    printf "%0$((WIDTH - progress))d" 0 | tr 0 '.'
    printf "] $to | $score1 $score2\r"

    scripts/create-config.py $PLAYER1 $PLAYER2 Simple $i 3600 --custom-properties >out/config.json
    out/aicup2019 &
    if [ "$PLAYER2" == "Local" ]; then
        out/prev 127.0.0.1 31002 &
    fi
    ./aicup2019 --batch-mode --log-level ERROR --config out/config.json --save-results out/result.txt
    read first second < <(scripts/parse-result.py)
    printf "                                                                    \r"
    echo $i $first $second
    if (( $first > $second )); then
        score1=$(python3 -c "print($score1+1)")
    elif (( $first < $second )); then
        score2=$(python3 -c "print($score2+1)")
    else
        score1=$(python3 -c "print($score1+0.5)")
        score2=$(python3 -c "print($score2+0.5)")
    fi
done

echo "result $score1 $score2"

wait
