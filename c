#!/usr/bin/env bash

set -e
trap "kill 0" EXIT

NOVIS=--batch-mode

make -Cout -j4
scripts/create-config.py Local Empty levels/level.txt --custom-properties >out/config.json
out/aicup2019 "$@" &
./aicup2019 $NOVIS --log-level ERROR --config out/config.json --save-results out/result.txt
test -s out/result.txt && scripts/parse-result.py

wait
