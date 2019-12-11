#!/usr/bin/env bash

set -e
trap "kill 0" EXIT

PLAYER1=Local
PLAYER2=Quick

NOVIS=--batch-mode
SEED=
LEN=
ARGS=

for arg
do
    if [ "$arg" == "-v" ]
    then
        NOVIS=
    elif [ -z "$SEED" ]
    then
        SEED=$arg
    elif [ -z "$LEN" ]
    then
        LEN=$arg
    else
        ARGS="$ARGS $arg"
    fi
done

if [ -z "$SEED" ]
then
    SEED=42
fi
if [ -z "$LEN" ]
then
    LEN=3600
fi

host=127.0.0.1
port=31001

make -Cout -j4
scripts/create-config.py $PLAYER1 $PLAYER2 Simple $SEED $LEN --custom-properties >out/config.json

if [ "$PLAYER1" == "Local" ]
then
    out/aicup2019 $host $port $ARGS &
    port=$((port + 1))
fi

if [ "$PLAYER2" == "Local" ]
then
    out/aicup2019 $host $port $ARGS &
fi

./aicup2019 $NOVIS --log-level ERROR --config out/config.json --save-results out/result.txt --player-names $PLAYER1 $PLAYER2
test -s out/result.txt && scripts/parse-result.py

wait
