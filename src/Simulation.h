#pragma once

#include "model/Level.hpp"
#include "model/World.h"
#include "Track.h"
#include <functional>

void simulate(
    int myId, const Level& level, World& world, const Track& moves, int microticks, size_t ticks,
    const std::function<void(size_t, const World&)>& callback
);
