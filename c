#!/usr/bin/env bash

set -e
trap "kill 0" EXIT

make -Cout -j4

P1=Local
P2=Quick

LR_ARGS=--batch-mode
SEED=
LEN=
ARGS=

for arg; do
    if [[ "$arg" =~ ^-.* ]]; then
        [ "$arg" == "--vis" ] && LR_ARGS=
        ARGS="$ARGS $arg"
    elif [ -z "$SEED" ]; then
        SEED=$arg
    elif [ -z "$LEN" ]; then
        LEN=$arg
    else
        ARGS="$ARGS $arg"
    fi
done

[ "$SEED" ] || SEED=42

source scripts/run-game.sh

test -s out/result.txt && scripts/parse-result.py

wait
