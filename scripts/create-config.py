#!/usr/bin/env python3

import json, sys

# Args:
# 1) First player
# 2) Second player
# 3) Path to the level, or "Simple"
# 4) Seed
# 5) Optional: max tick count
# 6) Optional: other arguments, e.g. "--custom-properties"

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

seed = int(sys.argv[4])

args = sys.argv[5:]
if "--custom-properties" in args:
    properties = json.load(open("scripts/properties.json", "r"))
    properties["max_tick_count"] = int(args[0])
    properties["team_size"] = 2
    # properties["ticks_per_second"] = 6000
    # properties["updates_per_tick"] = 1
else:
    properties = None

data = {
  "seed": seed,
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
    elif arg == "Keyboard":
        players.append("Keyboard")
    elif arg == "Empty":
        players.append({"Empty": None})
    elif arg == "Local":
        players.append(createLocal())
    else:
        raise Exception("Unknown player: " + arg)

print(json.dumps(data))
