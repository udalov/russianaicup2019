#pragma once

#include "model/UnitAction.hpp"
#include "model/Game.hpp"
#include "Debug.hpp"

UnitAction checkSimulation(int myId, const Game& game, Debug& debug, bool batch, bool visualize);
