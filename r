#!/usr/bin/env bash

set -e
trap "kill 0" EXIT

make -Cout -j4

[ "$P1" ] || P1=Local
[ "$P2" ] || P2=Local
[ "$TEAM_SIZE" ] || TEAM_SIZE=2
[ "$LEVEL" ] || LEVEL=Complex

LR_ARGS=--batch-mode
BASE_PORT=$((10000 + RANDOM % 20000))

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

    SEED=$i
    source scripts/run-game.sh

    read first second < <(scripts/parse-result.py $BASE_PORT)
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
