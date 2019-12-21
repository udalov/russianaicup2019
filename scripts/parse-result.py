#!/usr/bin/env python3

import json, sys

result = "out/result.txt"
if len(sys.argv) >= 2:
    result = "out/result-" + sys.argv[1] + ".txt"
data = json.load(open(result, "r"))

first = data["results"][0]
second = data["results"][1]
if data["players"][0]["crashed"]: first = -1
if data["players"][1]["crashed"]: second = -1
print(first, second)
