#!/usr/bin/env bash

if [ $(basename $0) == "run-game.sh" ]; then
    set -e
    trap "kill 0" EXIT
fi

fail() { echo "$1" >&2; exit 1; }
[ "$P1" ] || fail "No P1"
[ "$P2" ] || fail "No P2"
[ "$TEAM_SIZE" ] || fail "No TEAM_SIZE"
[ "$SEED" ] || fail "No SEED"
[ "$LEVEL" ] || fail "No LEVEL"

[ "$LEN" ] || LEN=3600
[ "$BASE_PORT" ] || BASE_PORT=31001

port1=$BASE_PORT
port2=$((port1 + 1))

if [ $P1 == Local ]; then P1="$P1$port1"; fi
if [ $P2 == Local ]; then P2="$P2$port2"; fi

config=out/config-$BASE_PORT.json
result=out/result-$BASE_PORT.txt
scripts/create-config.py $P1 $P2 $LEVEL $SEED $LEN $TEAM_SIZE >$config

if [[ $P1 == Local* ]]; then
    out/aicup2019 127.0.0.1 $port1 $ARGS &
fi

if [[ $P2 == Local* ]]; then
    out/prev 127.0.0.1 $port2 $PREV_ARGS &
fi

./aicup2019 $LR_ARGS --log-level ERROR --config $config --save-results $result --player-names $P1 $P2

wait
