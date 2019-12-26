#include "Simulation.h"

#include "model/Tile.hpp"
#include "Const.h"
#include "util.h"
#include <iostream>
#include <string>

using namespace std;

constexpr auto EPS = 1e-9;

constexpr auto ENEMY_BULLET_RL_EPS = 0.05;
constexpr auto ENEMY_BULLET_OTHER_EPS = 0.01;

constexpr auto RL_EXPLOSION_EPS = 0.1;
constexpr auto RL_EXPLOSION = ExplosionParams(rocketLauncherParams.explosion->radius + RL_EXPLOSION_EPS, rocketLauncherParams.explosion->damage);

bool isWall(const Level& level, double x, double y) {
    return level(x, y) == Tile::WALL;
}

bool isWallOrPlatform(const Level& level, double x, double y, bool jumpDown) {
    auto tile = level(x, y);
    return tile == Tile::WALL || (!jumpDown && tile == Tile::PLATFORM);
}

bool intersects(const Vec& p1, double w1, double h1, const Vec& p2, double w2, double h2) {
    return 2 * abs(p1.x - p2.x) <= w1 + w2 && 2 * abs(p1.y - p2.y) <= h1 + h2;
}

bool intersects(const Unit& unit, const Vec& rectCenter, double sizeX, double sizeY) {
    return intersects(unit.center(), unitSize.x, unitSize.y, rectCenter, sizeX, sizeY);
}

bool intersectsLootBox(const Unit& unit, const LootBox& box) {
    return intersects(unit, box.center(), lootBoxSize.x, lootBoxSize.y);
}

bool intersectsBullet(const Unit& unit, const Bullet& bullet, double eps = 0.0) {
    return intersects(unit, bullet.position, bullet.size + eps, bullet.size + eps);
}

bool intersectsUnit(const Unit& unit, const Unit& other) {
    return abs(unit.position.x - other.position.x) <= unitSize.x &&
        abs(unit.position.y - other.position.y) <= unitSize.y;
}

bool intersectsExplosion(const Vec& target, const Vec& size, const Vec& center, const ExplosionParams& explosion) {
    return intersects(target, size.x, size.y, center, explosion.radius * 2, explosion.radius * 2);
}

bool intersectsMine(const Bullet& bullet, const Mine& mine) {
    return intersects(bullet.position, bullet.size, bullet.size, mine.center(), mineSize.x, mineSize.y);
}

void explodeToUnits(World& world, const Vec& position, const ExplosionParams& explosion) {
    for (auto& unit : world.units) {
        if (intersectsExplosion(unit.center(), unitSize, position, explosion)) {
            unit.health -= explosion.damage;
        }
    }
}

void explodeToMines(World& world, const Vec& position, const ExplosionParams& explosion) {
    for (auto& mine : world.mines) {
        if (mine.state != MineState::EXPLODED && intersectsExplosion(mine.center(), mineSize, position, explosion)) {
            mine.state = MineState::EXPLODED;
            explodeToUnits(world, mine.center(), mineExplosionParams);
        }
    }
}

void simulate(
    int myId, const Level& level, World& world, const Track& track,
    int defaultMicroticks, size_t highResCutoff, size_t lowResCutoff, size_t ticks,
    const function<void(size_t, const World&)>& callback
) {
    auto& me = findUnit(world, myId);
    auto delta = 1.0 / ticksPerSecond;

    auto& x = me.position.x;
    auto& y = me.position.y;
    auto ux = unitSize.x;
    auto uy = unitSize.y;
    auto half = ux / 2;

    for (size_t tick = 0; tick < ticks; tick++) {
        auto microticks = tick < highResCutoff ? updatesPerTick : tick < lowResCutoff ? defaultMicroticks : 1;
        auto alpha = delta / microticks;

        auto& move = track[tick];
        auto vx = min(max(move.velocity, -unitMaxHorizontalSpeed), unitMaxHorizontalSpeed) * alpha;

        if (move.plantMine && me.mines > 0 && me.onGround && !me.onLadder) {
            me.mines--;
            world.mines.emplace_back(me.position, MineState::PREPARING, optional<double>(minePrepareTime));
        }

        for (size_t mt = 0; mt < microticks; mt++) {
            for (size_t i = 0; i < world.mines.size();) {
                auto& mine = world.mines[i];

                if (mine.state == MineState::IDLE) {
                    if (any_of(world.units.begin(), world.units.end(), [&mine](const auto& unit) {
                        constexpr auto sizeX = 2 * mineTriggerRadius + mineSize.x;
                        constexpr auto sizeY = 2 * mineTriggerRadius + mineSize.y;
                        return intersects(unit, mine.center(), sizeX, sizeY);
                    })) {
                        mine.state = MineState::TRIGGERED;
                        mine.timer = optional<double>(mineTriggerTime);
                    }
                } else if (mine.state == MineState::EXPLODED) {
                    explodeToMines(world, mine.position, mineExplosionParams);
                    fastRemove(world.mines, mine);
                    continue;
                } else if (mine.timer.has_value()) {
                    *mine.timer -= alpha;
                    if (*mine.timer <= 0.0) {
                        mine.timer = nullopt;
                        if (mine.state == MineState::PREPARING) {
                            mine.state = MineState::IDLE;
                        } else if (mine.state == MineState::TRIGGERED) {
                            mine.state = MineState::EXPLODED;
                            explodeToUnits(world, mine.position, mineExplosionParams);
                        }
                    }
                }
                i++;
            }

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

                for (auto& mine : world.mines) {
                    if (mine.state != MineState::EXPLODED && intersectsMine(bullet, mine)) {
                        mine.state = MineState::EXPLODED;
                        explodeToUnits(world, mine.position, mineExplosionParams);
                        remove = true;
                        break;
                    }
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
                    if (bullet.weaponType == WeaponType::ROCKET_LAUNCHER) {
                        explodeToUnits(world, bullet.position, RL_EXPLOSION);
                        explodeToMines(world, bullet.position, RL_EXPLOSION);
                    }
                    fastRemove(world.bullets, bullet);
                } else i++;
            }

            auto& weapon = me.weapon;
            if (weapon.has_value()) {
                if (weapon->fireTimer <= 0 && move.shoot) {
                    auto& wp = weapon->params;
                    auto& bp = wp.bullet;
                    auto aim = move.aim;
                    world.bullets.emplace_back(weapon->type, me.id, me.playerId, me.center(), aim.normalize() * bp.speed, bp.damage, bp.size);
                    if (--weapon->magazine == 0) {
                        weapon->magazine = wp.magazineSize;
                        weapon->fireTimer = wp.reloadTime;
                    } else {
                        weapon->fireTimer = wp.fireRate;
                    }
                }
                weapon->fireTimer -= alpha;
            }

            double vy;
            if (me.onLadder) {
                if (move.jump) {
                    vy = me.jumpState.speed * alpha;
                } else if (move.jumpDown) {
                    vy = -unitFallSpeed * alpha;
                } else {
                    vy = 0.0;
                }
            } else {
                if (me.jumpState.canJump && me.jumpState.maxTime >= -EPS && (move.jump || !me.jumpState.canCancel)) {
                    vy = me.jumpState.speed * alpha;
                } else {
                    vy = -unitFallSpeed * alpha;
                }
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

                for (auto& unit : world.units) {
                    if (unit.id != me.id && intersectsUnit(me, unit)) { x -= vx; break; }
                }
            }

            auto hasJumpTime = me.jumpState.maxTime > alpha;
            if (vy < 0 && floor(y + vy) != floor(y) && (
                isWallOrPlatform(level, x - half, y + vy, move.jumpDown) ||
                isWallOrPlatform(level, x + half, y + vy, move.jumpDown) ||
                (level(x, y + vy) == Tile::LADDER && !move.jumpDown)
            )) {
                y = floor(y) + EPS;
                me.onGround = true;
            } else if (vy > 0 && !me.onLadder && (
                !hasJumpTime ||
                isWall(level, x - half, y + uy + vy) || isWall(level, x + half, y + uy + vy)
            )) {
                if (hasJumpTime) {
                    y = ceil(y + uy) - uy - EPS;
                } else {
                    y += vy;
                }
                me.onGround = false;
                me.jumpState.maxTime = -EPS;
            } else {
                y += vy;
                me.onGround = false;

                for (auto& unit : world.units) {
                    if (unit.id != me.id && intersectsUnit(me, unit)) { y -= vy; break; }
                }
            }

            if (level(x, y) == Tile::LADDER || level(x, y + uy/2) == Tile::LADDER) {
                me.onLadder = true;
                me.onGround = true;
                me.jumpState = JumpState::UNIT_JUMP;
            } else if (level(x - half, y) == Tile::JUMP_PAD ||
                level(x + half, y) == Tile::JUMP_PAD ||
                level(x - half, y + uy/2) == Tile::JUMP_PAD ||
                level(x + half, y + uy/2) == Tile::JUMP_PAD ||
                level(x - half, y + uy) == Tile::JUMP_PAD ||
                level(x + half, y + uy) == Tile::JUMP_PAD) {
                me.jumpState = JumpState::JUMP_PAD_JUMP;
            } else {
                me.onLadder = false;
                if (me.jumpState.canJump && me.jumpState.maxTime > -EPS && (move.jump || !me.jumpState.canCancel)) {
                    me.jumpState.maxTime -= alpha;
                } else if (me.onGround) {
                    me.jumpState = JumpState::UNIT_JUMP;
                } else {
                    me.jumpState = JumpState::NO_JUMP;
                }
            }
        }

        // Ideally, loot boxes should be collected at the beginning of each microtick.
        for (size_t i = 0; i < world.lootBoxes.size();) {
            auto& box = world.lootBoxes[i];
            if (!intersectsLootBox(me, box)) { i++; continue; }
            auto item = box.item;
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
                me.weapon->lastAngle = nullopt;
            } else if (item.isHealthPack() && item.health() > 0 && me.health < 100) {
                me.health = min(me.health + item.health(), 100);
            } else if (item.isMine()) {
                me.mines++;
            } else {
                i++;
                continue;
            }
            fastRemove(world.lootBoxes, box);
        }

        callback(tick, world);
    }
}
