#!/usr/bin/env bash

if [ $(basename $0) == "run-game.sh" ]; then
    set -e
    trap "kill 0" EXIT
fi

fail() { echo "$1" >&2; exit 1; }
[ "$P1" ] || fail "No P1"
[ "$P2" ] || fail "No P2"
[ "$SEED" ] || fail "No SEED"

[ "$LEN" ] || LEN=3600
[ "$BASE_PORT" ] || BASE_PORT=31001

port=$BASE_PORT

scripts/create-config.py $P1 $P2 Simple $SEED $LEN --custom-properties >out/config.json

if [ "$P1" == "Local" ]; then
    out/aicup2019 127.0.0.1 $port $ARGS &
    port=$((port + 1))
fi

if [ "$P2" == "Local" ]; then
    out/prev 127.0.0.1 $port $PREV_ARGS &
fi

./aicup2019 $LR_ARGS --log-level ERROR --config out/config.json --save-results out/result.txt --player-names $P1 $P2

wait
