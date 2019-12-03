#!/usr/bin/env python3

import json, sys

port = 31001

def createLocal():
    global port
    newPort = port
    port += 1
    return {
      "Tcp": {
        "host": None,
        "port": newPort,
        "accept_timeout": None,
        "timeout": None,
        "token": None
      }
    }

level = "Simple"
if sys.argv[3] != "Simple":
    level = {
      "LoadFrom": {
        "path": sys.argv[3]
      }
    }

args = sys.argv[4:]
if "--custom-properties" in args:
    properties = json.load(open("scripts/properties.json", "r"))
    properties["max_tick_count"] = 1000
    # properties["ticks_per_second"] = 6000
    # properties["updates_per_tick"] = 1
else:
    properties = None

data = {
  "options_preset": {
    "Custom": {
      "level": level,
      "properties": properties
    }
  },
  "players": []
}

players = data["players"]

for i in range(1, 3):
    arg = sys.argv[i]
    if arg == "Quick":
        players.append("Quickstart")
    elif arg == "Empty":
        players.append({"Empty": None})
    elif arg == "Local":
        players.append(createLocal())
    else:
        raise Exception("Unknown player: " + arg)

print(json.dumps(data))
