#!/usr/bin/env bash

set -e
trap "kill 0" EXIT

scripts/create-config.py Local Quick >out/config.json
out/aicup2019 &
./aicup2019 --log-level ERROR --config out/config.json --save-results out/result.txt
test -s out/result.txt && scripts/parse-result.py

wait
