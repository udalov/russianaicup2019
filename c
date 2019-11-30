#!/usr/bin/env bash

set -e

scripts/create-config.py Local Quick >out/config.json
for i in `seq 1 10`
do
    out/aicup2019 &
    ./aicup2019 --batch-mode --log-level ERROR --config out/config.json --save-results out/result.txt
    scripts/parse-result.py
done
