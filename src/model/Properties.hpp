#ifndef _MODEL_PROPERTIES_HPP_
#define _MODEL_PROPERTIES_HPP_

#include "../Stream.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include "WeaponType.hpp"
#include "WeaponParams.hpp"
#include "ExplosionParams.hpp"
#include "Vec2D.h"

class Properties {
public:
    int maxTickCount;
    int teamSize;
    double ticksPerSecond;
    int updatesPerTick;
    Vec2Double lootBoxSize;
    Vec2Double unitSize;
    double unitMaxHorizontalSpeed;
    double unitFallSpeed;
    double unitJumpTime;
    double unitJumpSpeed;
    double jumpPadJumpTime;
    double jumpPadJumpSpeed;
    int unitMaxHealth;
    int healthPackHealth;
    std::unordered_map<WeaponType, WeaponParams> weaponParams;
    Vec2Double mineSize;
    ExplosionParams mineExplosionParams;
    double minePrepareTime;
    double mineTriggerTime;
    double mineTriggerRadius;
    int killScore;
    Properties();
    Properties(int maxTickCount, int teamSize, double ticksPerSecond, int updatesPerTick, Vec2Double lootBoxSize, Vec2Double unitSize, double unitMaxHorizontalSpeed, double unitFallSpeed, double unitJumpTime, double unitJumpSpeed, double jumpPadJumpTime, double jumpPadJumpSpeed, int unitMaxHealth, int healthPackHealth, std::unordered_map<WeaponType, WeaponParams> weaponParams, Vec2Double mineSize, ExplosionParams mineExplosionParams, double minePrepareTime, double mineTriggerTime, double mineTriggerRadius, int killScore);
    static Properties readFrom(InputStream& stream);
};

#endif
