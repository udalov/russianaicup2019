#!/usr/bin/env python3

import json, sys

# Args:
# 1) First player
# 2) Second player
# 3) Path to the level, or "Simple" or "Complex"
# 4) Seed
# 5) Optional: max tick count
# 6) Optional: team size

def createLocal(port):
    return {
      "Tcp": {
        "host": None,
        "port": port,
        "accept_timeout": None,
        "timeout": None,
        "token": None
      }
    }

level = sys.argv[3]
if level != "Simple" and level != "Complex":
    level = {
      "LoadFrom": {
        "path": level
      }
    }

seed = int(sys.argv[4])

args = sys.argv[5:]
if len(args) > 0:
    properties = json.load(open("scripts/properties.json", "r"))
    properties["max_tick_count"] = int(args[0])
    if len(args) > 1:
        properties["team_size"] = int(args[1])
    else:
        properties["team_size"] = 2
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
    elif arg[:5] == "Local":
        players.append(createLocal(int(arg[5:])))
    else:
        raise Exception("Unknown player: " + arg)

print(json.dumps(data))
