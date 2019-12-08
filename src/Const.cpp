#include "Const.h"

#include <iostream>
#include <iomanip>
#include <stdexcept>

using namespace std;

template<typename T> void check(T expected, T actual) {
    if (!(expected == actual)) {
        throw std::runtime_error("Constant value is incorrect");
    }
}

void checkConstants(const Properties& properties) {
    check(jumpPadJumpSpeed, properties.jumpPadJumpSpeed);
    check(jumpPadJumpTime, properties.jumpPadJumpTime);
    check(lootBoxSize, properties.lootBoxSize);
    check(ticksPerSecond, properties.ticksPerSecond);
    check(unitFallSpeed, properties.unitFallSpeed);
    check(unitJumpSpeed, properties.unitJumpSpeed);
    check(unitJumpTime, properties.unitJumpTime);
    check(unitMaxHorizontalSpeed, properties.unitMaxHorizontalSpeed);
    check(unitSize, properties.unitSize);
    check(updatesPerTick, properties.updatesPerTick);

    check(pistolParams, properties.weaponParams.find(WeaponType::PISTOL)->second);
    check(assaultRifleParams, properties.weaponParams.find(WeaponType::ASSAULT_RIFLE)->second);
    check(rocketLauncherParams, properties.weaponParams.find(WeaponType::ROCKET_LAUNCHER)->second);
}


#ifndef LOCAL
void dumpConstants(const Properties& p) {
    throw std::runtime_error("Unsupported");
}
#else

#include "nlohmann/json.hpp.inc"

using json = nlohmann::json;

void dumpConstants(const Properties& p) {
    json a = {
        { "health_pack_health", p.healthPackHealth },
        { "jump_pad_jump_speed", p.jumpPadJumpSpeed },
        { "jump_pad_jump_time", p.jumpPadJumpTime },
        { "kill_score", p.killScore },
        { "loot_box_size",
            { { "x", p.lootBoxSize.x }, { "y", p.lootBoxSize.y } }
        },
        { "max_tick_count", p.maxTickCount },
        { "mine_explosion_params",
            { { "radius", p.mineExplosionParams.radius }, { "damage", p.mineExplosionParams.damage } }
        },
        { "mine_prepare_time", p.minePrepareTime },
        { "mine_size",
            { { "x", p.mineSize.x }, { "y", p.mineSize.y } }
        },
        { "mine_trigger_radius", p.mineTriggerRadius },
        { "mine_trigger_time", p.mineTriggerTime },
        { "team_size", p.teamSize },
        { "ticks_per_second", p.ticksPerSecond },
        { "unit_fall_speed", p.unitFallSpeed },
        { "unit_jump_speed", p.unitJumpSpeed },
        { "unit_jump_time", p.unitJumpTime },
        { "unit_max_health", p.unitMaxHealth },
        { "unit_max_horizontal_speed", p.unitMaxHorizontalSpeed },
        { "unit_size",
            { { "x", p.unitSize.x }, { "y", p.unitSize.y } }
        },
        { "updates_per_tick", p.updatesPerTick },
        { "weapon_params", {} },
    };
    for (auto& [name, weapon] : {
        pair("Pistol", WeaponType::PISTOL),
        pair("AssaultRifle", WeaponType::ASSAULT_RIFLE),
        pair("RocketLauncher", WeaponType::ROCKET_LAUNCHER),
    }) {
        auto wp = p.weaponParams.at(weapon);
        a["weapon_params"][name] = {
            { "magazine_size", wp.magazineSize },
            { "fire_rate", wp.fireRate },
            { "reload_time", wp.reloadTime },
            { "min_spread", wp.minSpread },
            { "max_spread", wp.maxSpread },
            { "recoil", wp.recoil },
            { "aim_speed", wp.aimSpeed },
            { "bullet",
                {
                    { "speed", wp.bullet.speed },
                    { "size", wp.bullet.size },
                    { "damage", wp.bullet.damage },
                }
            }
        };
    }
    cout << std::setw(4) << a << endl;
}
#endif
