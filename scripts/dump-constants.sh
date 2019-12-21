#!/usr/bin/env bash

set -e
trap "kill 0" EXIT
make -Cout -j4
scripts/create-config.py Local31001 Empty Simple 42 >out/config.json
out/aicup2019 --dump-constants >scripts/properties.json &
./aicup2019 --batch-mode --log-level ERROR --config out/config.json
wait
