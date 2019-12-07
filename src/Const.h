#pragma once

#include "model/Properties.hpp"

constexpr auto jumpPadJumpSpeed = 20.0;
constexpr auto jumpPadJumpTime = 0.525;
constexpr auto lootBoxSize = Vec(0.5, 0.5);
constexpr auto ticksPerSecond = 60.0;
constexpr auto unitFallSpeed = 10.0;
constexpr auto unitJumpSpeed = 10.0;
constexpr auto unitJumpTime = 0.55;
constexpr auto unitMaxHorizontalSpeed = 10.0;
constexpr auto unitSize = Vec(0.9, 1.8);
constexpr auto updatesPerTick = 100;

void checkConstants(const Properties& properties);

void dumpConstants(const Properties& properties);
