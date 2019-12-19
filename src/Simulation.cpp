#include "Simulation.h"

#include "model/Tile.hpp"
#include "Const.h"
#include "util.h"
#include <iostream>
#include <string>

using namespace std;

constexpr auto EPS = 1e-9;

constexpr auto RL_EXPLOSION_EPS = 0.2;
constexpr auto ENEMY_BULLET_RL_EPS = 0.2;
constexpr auto ENEMY_BULLET_OTHER_EPS = 0.01;

bool isWall(const Level& level, double x, double y) {
    return level(x, y) == Tile::WALL;
}

bool isWallOrPlatform(const Level& level, double x, double y, bool jumpDown) {
    auto tile = level(x, y);
    return tile == Tile::WALL || (!jumpDown && tile == Tile::PLATFORM);
}

bool intersects(const Unit& unit, const Vec& rectCenter, double sizeX, double sizeY) {
    auto unitCenter = unit.center();
    return 2 * abs(unitCenter.x - rectCenter.x) <= unitSize.x + sizeX &&
        2 * abs(unitCenter.y - rectCenter.y) <= unitSize.y + sizeY;
}

bool intersects(const Unit& unit, const LootBox& box) {
    return intersects(unit, box.center(), lootBoxSize.x, lootBoxSize.y);
}

bool intersectsBullet(const Unit& unit, const Bullet& bullet, double eps = 0.0) {
    return intersects(unit, bullet.position, bullet.size + eps, bullet.size + eps);
}

void removeUnreachableLootBoxes(const Unit& unit, World& world, size_t ticks) {
    auto& loot = world.lootBoxes;
    auto v = unit.center();
    constexpr auto closenessX = unitSize.x + lootBoxSize.x;
    constexpr auto closenessY = unitSize.y + lootBoxSize.y;
    loot.erase(remove_if(loot.begin(), loot.end(), [&v, ticks](const auto& box) {
        constexpr auto maxSpeedX = 10.0 / ticksPerSecond;
        constexpr auto maxSpeedY = 20.0 / ticksPerSecond;
        auto b = box.center();
        return abs(v.x - b.x) > (unitSize.x + lootBoxSize.x) / 2 + maxSpeedX * ticks + EPS &&
            abs(v.y - b.y) > (unitSize.y + lootBoxSize.y) / 2 + maxSpeedY * ticks + EPS;
    }), loot.end());
}

void simulate(
    int myId, const Level& level, World& world, const Track& track, int microticks, size_t ticks,
    const function<void(size_t, const World&)>& callback
) {
    auto& me = findUnit(world, myId);
    // TODO: use more microticks when bullet is nearby
    auto alpha = 1.0 / ticksPerSecond / microticks;

    auto& x = me.position.x;
    auto& y = me.position.y;
    auto ux = unitSize.x;
    auto uy = unitSize.y;
    auto half = ux / 2;

    removeUnreachableLootBoxes(me, world, ticks);

    for (size_t tick = 0; tick < ticks; tick++) {
        auto closestLootBox = minBy<LootBox>(world.lootBoxes, [&me](const auto& box) {
            return box.center().sqrDist(me.center());
        });
        auto& move = track[tick];
        auto vx = min(max(move.velocity, -unitMaxHorizontalSpeed), unitMaxHorizontalSpeed) * alpha;
        auto vy = 0.0;

        if (me.onLadder) {
            if (move.jump) {
                vy = me.jumpState.speed * alpha;
            }
        } else {
            if (
                me.jumpState.canJump && me.jumpState.maxTime >= -EPS &&
                (move.jump || !me.jumpState.canCancel)
            ) {
                vy = me.jumpState.speed * alpha;
            } else {
                vy = -unitFallSpeed * alpha;
                me.jumpState = JumpState::NO_JUMP;
            }
        }

        for (size_t mt = 0; mt < microticks; mt++) {
            for (size_t i = 0; i < world.bullets.size();) {
                auto& bullet = world.bullets[i];

                bullet.position += bullet.velocity * alpha;
                auto bx = bullet.position.x;
                auto by = bullet.position.y;

                auto remove = false;
                auto s = bullet.size / 2;
                if (isWall(level, bx - s, by - s) || isWall(level, bx - s, by + s) ||
                    isWall(level, bx + s, by - s) || isWall(level, bx + s, by + s)) {
                    remove = true;
                }

                auto bulletEps =
                    bullet.playerId == me.playerId ? 0 :
                    bullet.weaponType == WeaponType::ROCKET_LAUNCHER ? ENEMY_BULLET_RL_EPS : ENEMY_BULLET_OTHER_EPS;
                if (!remove) {
                    // TODO: optimize
                    for (auto& unit : world.units) {
                        if (unit.id != bullet.unitId && intersectsBullet(unit, bullet, bulletEps)) {
                            unit.health -= bullet.damage;
                            remove = true;
                            break;
                        }
                    }
                }

                if (remove) {
                    auto explosion = bullet.explosionParams;
                    if (explosion.has_value()) {
                        auto size = 2 * (explosion->radius + RL_EXPLOSION_EPS);
                        for (auto& unit : world.units) {
                            if (intersects(unit, bullet.position, size, size)) {
                                unit.health -= explosion->damage;
                            }
                        }
                    }
                    fastRemove(world.bullets, bullet);
                } else i++;
            }

            if (!me.onLadder && me.jumpState.canJump && me.jumpState.maxTime >= -EPS &&
                (move.jump || !me.jumpState.canCancel)) {
                me.jumpState.maxTime -= alpha;
            }

            if (vx < 0 && (
                isWall(level, x - half + vx, y) ||
                isWall(level, x - half + vx, y + uy/2) ||
                isWall(level, x - half + vx, y + uy)
            )) {
                x = floor(x - half) + half + EPS;
            } else if (vx > 0 && (
                isWall(level, x + half + vx, y) ||
                isWall(level, x + half + vx, y + uy/2) ||
                isWall(level, x + half + vx, y + uy)
            )) {
                x = ceil(x + half) - half - EPS;
            } else {
                x += vx;
            }

            auto hasJumpTime = me.jumpState.maxTime > 0.0;
            if (vy < 0 && floor(y + vy) != floor(y) && (
                isWallOrPlatform(level, x - half, y + vy, move.jumpDown) ||
                isWallOrPlatform(level, x + half, y + vy, move.jumpDown) ||
                (level(x, y + vy) == Tile::LADDER && !move.jumpDown)
            )) {
                y = floor(y) + EPS;
                me.jumpState = JumpState::UNIT_JUMP;
                if (move.jump) {
                    vy = me.jumpState.speed * alpha;
                }
            } else if (vy > 0 && !me.onLadder && (
                !hasJumpTime ||
                isWall(level, x - half, y + uy + vy) || isWall(level, x + half, y + uy + vy)
            )) {
                if (hasJumpTime) {
                    y = ceil(y + uy) - uy - EPS;
                } else {
                    y += vy;
                }
                me.jumpState = JumpState::NO_JUMP;
                vy = -unitFallSpeed * alpha;
            } else {
                y += vy;
            }

            if (level(x, y) == Tile::LADDER) {
                me.onLadder = true;
                me.jumpState = JumpState::UNIT_JUMP;
            } else {
                me.onLadder = false;
            }

            if (level(x - half, y) == Tile::JUMP_PAD ||
                level(x + half, y) == Tile::JUMP_PAD ||
                level(x - half, y + uy) == Tile::JUMP_PAD ||
                level(x + half, y + uy) == Tile::JUMP_PAD) {
                me.jumpState = JumpState::JUMP_PAD_JUMP;
                vy = jumpPadJumpSpeed * alpha;
            }

            auto& weapon = me.weapon;
            if (weapon.has_value()) {
                auto& wp = weapon->params;
                weapon->fireTimer -= alpha;
                if (weapon->fireTimer <= 0 && move.shoot) {
                    auto& bp = wp.bullet;
                    auto aim = move.aim;
                    auto explosion = weapon->type == WeaponType::ROCKET_LAUNCHER ? optional<ExplosionParams>(rocketLauncherParams.explosion) : nullopt;
                    world.bullets.emplace_back(weapon->type, me.id, me.playerId, me.center(), aim.normalize() * bp.speed, bp.damage, bp.size, explosion);
                    if (--weapon->magazine == 0) {
                        weapon->magazine = wp.magazineSize;
                        weapon->fireTimer = wp.reloadTime;
                    } else {
                        weapon->fireTimer = wp.fireRate;
                    }
                }
            }

            if (closestLootBox && intersects(me, *closestLootBox)) {
                auto item = closestLootBox->item;
                if (item.isWeapon() && (!me.weapon || move.swapWeapon)) {
                    me.weapon = Weapon();
                    me.weapon->type = item.weaponType();
                    WeaponParams params;
                    switch (me.weapon->type) {
                        case WeaponType::PISTOL: params = pistolParams; break;
                        case WeaponType::ASSAULT_RIFLE: params = assaultRifleParams; break;
                        case WeaponType::ROCKET_LAUNCHER: params = rocketLauncherParams; break;
                    }
                    me.weapon->params = params;
                    me.weapon->magazine = params.magazineSize;
                    me.weapon->spread = params.minSpread; // ???
                    me.weapon->fireTimer = params.reloadTime;
                    me.weapon->lastAngle = optional<double>(atan2(move.aim.y, move.aim.x));
                    fastRemove(world.lootBoxes, *closestLootBox);
                } else if (item.isHealthPack() && item.health() > 0 && me.health < 100) {
                    me.health = min(me.health + item.health(), 100);
                    fastRemove(world.lootBoxes, *closestLootBox);
                }
            }
        }

        callback(tick, world);
    }
}
