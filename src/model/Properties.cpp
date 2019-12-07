#include "Properties.hpp"

Properties::Properties() { }
Properties::Properties(int maxTickCount, int teamSize, double ticksPerSecond, int updatesPerTick, Vec2Double lootBoxSize, Vec2Double unitSize, double unitMaxHorizontalSpeed, double unitFallSpeed, double unitJumpTime, double unitJumpSpeed, double jumpPadJumpTime, double jumpPadJumpSpeed, int unitMaxHealth, int healthPackHealth, std::unordered_map<WeaponType, WeaponParams> weaponParams, Vec2Double mineSize, ExplosionParams mineExplosionParams, double minePrepareTime, double mineTriggerTime, double mineTriggerRadius, int killScore) : maxTickCount(maxTickCount), teamSize(teamSize), ticksPerSecond(ticksPerSecond), updatesPerTick(updatesPerTick), lootBoxSize(lootBoxSize), unitSize(unitSize), unitMaxHorizontalSpeed(unitMaxHorizontalSpeed), unitFallSpeed(unitFallSpeed), unitJumpTime(unitJumpTime), unitJumpSpeed(unitJumpSpeed), jumpPadJumpTime(jumpPadJumpTime), jumpPadJumpSpeed(jumpPadJumpSpeed), unitMaxHealth(unitMaxHealth), healthPackHealth(healthPackHealth), weaponParams(weaponParams), mineSize(mineSize), mineExplosionParams(mineExplosionParams), minePrepareTime(minePrepareTime), mineTriggerTime(mineTriggerTime), mineTriggerRadius(mineTriggerRadius), killScore(killScore) { }
Properties Properties::readFrom(InputStream& stream) {
    Properties result;
    result.maxTickCount = stream.readInt();
    result.teamSize = stream.readInt();
    result.ticksPerSecond = stream.readDouble();
    result.updatesPerTick = stream.readInt();
    result.lootBoxSize = Vec2Double::readFrom(stream);
    result.unitSize = Vec2Double::readFrom(stream);
    result.unitMaxHorizontalSpeed = stream.readDouble();
    result.unitFallSpeed = stream.readDouble();
    result.unitJumpTime = stream.readDouble();
    result.unitJumpSpeed = stream.readDouble();
    result.jumpPadJumpTime = stream.readDouble();
    result.jumpPadJumpSpeed = stream.readDouble();
    result.unitMaxHealth = stream.readInt();
    result.healthPackHealth = stream.readInt();
    size_t weaponParamsSize = stream.readInt();
    result.weaponParams = std::unordered_map<WeaponType, WeaponParams>();
    result.weaponParams.reserve(weaponParamsSize);
    for (size_t i = 0; i < weaponParamsSize; i++) {
        WeaponType weaponParamsKey;
        switch (stream.readInt()) {
        case 0:
            weaponParamsKey = WeaponType::PISTOL;
            break;
        case 1:
            weaponParamsKey = WeaponType::ASSAULT_RIFLE;
            break;
        case 2:
            weaponParamsKey = WeaponType::ROCKET_LAUNCHER;
            break;
        default:
            throw std::runtime_error("Unexpected discriminant value");
        }
        WeaponParams weaponParamsValue;
        weaponParamsValue = WeaponParams::readFrom(stream);
        result.weaponParams.emplace(std::make_pair(weaponParamsKey, weaponParamsValue));
    }
    result.mineSize = Vec2Double::readFrom(stream);
    result.mineExplosionParams = ExplosionParams::readFrom(stream);
    result.minePrepareTime = stream.readDouble();
    result.mineTriggerTime = stream.readDouble();
    result.mineTriggerRadius = stream.readDouble();
    result.killScore = stream.readInt();
    return result;
}
