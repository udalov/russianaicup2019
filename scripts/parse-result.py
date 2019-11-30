#!/usr/bin/env python3

import json

data = json.load(open("out/result.txt", "r"))

first = data["results"][0]
second = data["results"][1]
if data["players"][0]["crashed"]: first = -1
if data["players"][1]["crashed"]: second = -1
print(first, second)
