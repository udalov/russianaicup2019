#pragma once

#include "model/Game.hpp"
#include "model/UnitAction.hpp"
#include "model/World.h"
#include "Debug.hpp"

bool isPredictionCorrect(int playerId, const World& last, const World& expected, const World& actual);

void reportPredictionDifference(
    int myId, const World& expectedWorld, const Game& actualGame, const UnitAction& lastAction,
    bool isOk, bool logWorldAnyway, int tick
);

UnitAction checkSimulation(int myId, const Game& game, Debug& debug, bool batch, bool visualize);
