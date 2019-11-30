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

data = {
  "options_preset": {
    "Custom": {
      "level": "Simple",
      "properties": None
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
        players.append("Empty")
    elif arg == "Local":
        players.append(createLocal())
    else:
        raise Exception("Unknown player: " + arg)

print(json.dumps(data))
