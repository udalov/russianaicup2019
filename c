#!/usr/bin/env bash

set -e
trap "kill 0" EXIT

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

make -Cout -j4
scripts/create-config.py Local Quick levels/level.txt $SEED $LEN --custom-properties >out/config.json
out/aicup2019 $ARGS &
./aicup2019 $NOVIS --log-level ERROR --config out/config.json --save-results out/result.txt
test -s out/result.txt && scripts/parse-result.py

wait
