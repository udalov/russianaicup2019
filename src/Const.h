#pragma once

#include <optional>
#include "model/Properties.hpp"
#include "model/WeaponParams.hpp"

constexpr auto jumpPadJumpSpeed = 20.0;
constexpr auto jumpPadJumpTime = 0.525;
constexpr auto lootBoxSize = Vec(0.5, 0.5);
constexpr auto minePrepareTime = 1.0;
constexpr auto mineSize = Vec(0.5, 0.5);
constexpr auto mineTriggerRadius = 1.0;
constexpr auto mineTriggerTime = 0.5;
constexpr auto ticksPerSecond = 60.0;
constexpr auto unitFallSpeed = 10.0;
constexpr auto unitJumpSpeed = 10.0;
constexpr auto unitJumpTime = 0.55;
constexpr auto unitMaxHorizontalSpeed = 10.0;
constexpr auto unitSize = Vec(0.9, 1.8);
constexpr auto updatesPerTick = 100;

constexpr auto pistolParams = WeaponParams(8, 0.4, 1.0, 0.05, 0.5, 0.5, 1.0, BulletParams(50.0, 0.2, 20), std::nullopt);
constexpr auto assaultRifleParams = WeaponParams(20, 0.1, 1.0, 0.1, 0.5, 0.2, 1.9, BulletParams(50.0, 0.2, 5), std::nullopt);
constexpr auto rocketLauncherParams = WeaponParams(1, 1.0, 1.0, 0.1, 0.5, 1.0, 1.0, BulletParams(20.0, 0.4, 30), ExplosionParams(3.0, 50));

constexpr auto mineExplosionParams = ExplosionParams(3.0, 50);

void checkConstants(const Properties& properties);

void dumpConstants(const Properties& properties);
