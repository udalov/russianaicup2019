#include "MyStrategy.hpp"

#include <iostream>
#include <vector>
#include "Const.h"
#include "util.h"

using namespace std;

const double EPS = 1e-9;

MyStrategy::MyStrategy(unordered_map<string, string>&& params): params(params) {}

unique_ptr<vector<UnitAction>> getRandomMoveSequence() {
    auto ans = make_unique<vector<UnitAction>>();
    UnitAction moveRight;
    moveRight.velocity = 100.0;
    UnitAction jump;
    jump.jump = true;
    UnitAction jumpAndMoveRight;
    jumpAndMoveRight.velocity = 1000.0;
    jumpAndMoveRight.jump = true;
    UnitAction left;
    left.velocity = -100.0;

    auto t = 100 / updatesPerTick;
    size_t cp1 = 13 * t;
    size_t cp2 = 25 * t;
    size_t cp3 = 186 * t;
    size_t cp4 = 1000 * t;

    for (size_t i = 0; i < cp1; i++) ans->push_back(moveRight);
    for (size_t i = cp1; i < cp2; i++) ans->push_back(jump);
    for (size_t i = cp2; i < cp3; i++) ans->push_back(jumpAndMoveRight);
    for (size_t i = cp3; i < cp4; i++) ans->push_back(left);

    for (auto& t : *ans) {
        t.shoot = true;
        t.aim = Vec(50, 42);
    }
    return ans;
}

Tile getTile(const Level& level, double x, double y) {
    auto i = static_cast<size_t>(x);
    auto j = static_cast<size_t>(y);
    return /* i >= level.tiles.size() || j >= level.tiles[0].size() ? Tile::WALL : */ level.tiles[i][j];
}

Tile getTile(const Level& level, Vec v) {
    return getTile(level, v.x, v.y);
}

Tile getTile(const Game& game, double x, double y) {
    return getTile(game.level, x, y);
}

void drawUnit(const Unit& unit, const ColorFloat& color, Debug& debug) {
    auto corner = unit.position - Vec(unitSize.x / 2, 0);
    debug.draw(CustomData::Rect(corner, unitSize, color));
}

bool isWall(const Game& game, double x, double y) {
    return getTile(game, x, y) == Tile::WALL;
}

bool isWallOrPlatform(const Game& game, double x, double y, bool jumpDown) {
    auto tile = getTile(game, x, y);
    // TODO: move tile == LADDER under !jumpDown check?
    return tile == Tile::WALL || tile == Tile::LADDER || (!jumpDown && tile == Tile::PLATFORM);
}

bool intersects(const Unit& unit, const LootBox& box) {
    auto pos = unit.center();
    return 2 * abs(pos.x - box.position.x) <= unitSize.x + lootBoxSize.x &&
        2 * abs(pos.y + box.position.y) <= unitSize.y + lootBoxSize.y;
}

vector<LootBox> getReachableLootBoxes(const Unit& unit, const Game& game, size_t ticks) {
    auto loot = game.lootBoxes;
    auto v1 = unit.center();
    loot.erase(remove_if(loot.begin(), loot.end(), [&v1, ticks](const auto& b) {
        constexpr auto maxSpeed = 20.0 / ticksPerSecond;
        return max(abs(v1.x-b.position.x), abs(v1.y-b.position.y)) > unitSize.y + lootBoxSize.y + maxSpeed * ticks;
    }), loot.end());
    return loot;
}

Unit prediction;

void simulate(Unit me, Game game, const vector<UnitAction>& moves, Debug& debug, size_t ticks) {
    auto alpha = 1.0 / ticksPerSecond / updatesPerTick;

    debug.log(string("cur: ") + me.toString());

    auto& x = me.position.x;
    auto& y = me.position.y;
    auto ux = unitSize.x;
    auto uy = unitSize.y;
    auto half = ux / 2;

    auto loot = getReachableLootBoxes(me, game, ticks);

    for (size_t tick = 0; tick < ticks; tick++) {
        auto closestLootBox = min_element(loot.begin(), loot.end(), [&me](const auto& b1, const auto& b2) {
            return b1.position.sqrDist(me.center()) < b2.position.sqrDist(me.center());
        });
        auto& move = moves[tick];
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
            }
        }

        for (size_t mt = 0; mt < updatesPerTick; mt++) {
            if (!me.onLadder && me.jumpState.canJump && me.jumpState.maxTime >= -EPS &&
                (move.jump || !me.jumpState.canCancel)) {
                me.jumpState.maxTime -= alpha;
            }

            if (vx < 0 && (
                isWall(game, x - half + vx, y) ||
                isWall(game, x - half + vx, y + uy/2) ||
                isWall(game, x - half + vx, y + uy)
            )) {
                x = floor(x - half) + half + EPS;
            } else if (vx > 0 && (
                isWall(game, x + half + vx, y) ||
                isWall(game, x + half + vx, y + uy/2) ||
                isWall(game, x + half + vx, y + uy)
            )) {
                x = ceil(x + half) - half - EPS;
            } else {
                x += vx;
            }

            if (vy < 0 && (
                isWallOrPlatform(game, x - half, y + vy, move.jumpDown) ||
                isWallOrPlatform(game, x + half, y + vy, move.jumpDown)
            )) {
                y = floor(y) + EPS;
                me.jumpState.canJump = true;
                me.jumpState.canCancel = true;
                me.jumpState.maxTime = unitJumpTime;
                me.jumpState.speed = unitJumpSpeed;
                if (move.jump) {
                    vy = me.jumpState.speed * alpha;
                }
            } else if (vy > 0 && !me.onLadder && (
                isWall(game, x - half, y + uy + vy) || isWall(game, x + half, y + uy + vy)
            )) {
                y = ceil(y + uy) - uy - EPS;
                me.jumpState.canJump = false;
                me.jumpState.canCancel = false;
                me.jumpState.maxTime = 0.0;
                me.jumpState.speed = 0.0;
                vy = -unitFallSpeed * alpha;
            } else {
                y += vy;
            }

            if (getTile(game, x, y) == Tile::LADDER) {
                me.onLadder = true;
                me.jumpState.maxTime = unitJumpTime;
            } else {
                me.onLadder = false;
            }

            auto& weapon = me.weapon;
            if (weapon.has_value()) {
                auto& wp = weapon->params;
                weapon->fireTimer =
                    (weapon->fireTimer.has_value() ? *weapon->fireTimer : wp.reloadTime) - alpha;
                if (*weapon->fireTimer <= 0) {
                    weapon->fireTimer = wp.fireRate;
                    if (--weapon->magazine == 0) {
                        weapon->magazine = wp.magazineSize;
                        weapon->fireTimer = wp.reloadTime;
                    }
                }
            }

            if (closestLootBox != loot.end() && intersects(me, *closestLootBox)) {
                if (closestLootBox->item.isWeapon() && (!me.weapon || move.swapWeapon)) {
                    me.weapon = Weapon();
                    me.weapon->type = closestLootBox->item.weaponType();
                    WeaponParams params;
                    switch (me.weapon->type) {
                        case WeaponType::PISTOL: params = pistolParams; break;
                        case WeaponType::ASSAULT_RIFLE: params = assaultRifleParams; break;
                        case WeaponType::ROCKET_LAUNCHER: params = rocketLauncherParams; break;
                    }
                    me.weapon->params = params;
                    me.weapon->magazine = params.magazineSize;
                    me.weapon->spread = params.minSpread; // ???
                    me.weapon->fireTimer = optional<double>(params.reloadTime);
                    me.weapon->lastAngle = optional<double>(atan2(move.aim.y, move.aim.x));
                }

                size_t i = closestLootBox - loot.begin();
                if (i + 1 != loot.size()) {
                    swap(loot[i], loot.back());
                }
                loot.pop_back();
            }
        }

        if (me.jumpState.maxTime <= -EPS) {
            me.jumpState.canJump = false;
            me.jumpState.canCancel = false;
            me.jumpState.maxTime = 0.0;
            me.jumpState.speed = 0.0;
            vy = 0.0;
        }

        if (getTile(game, x - half, y) == Tile::JUMP_PAD ||
            getTile(game, x + half, y) == Tile::JUMP_PAD ||
            getTile(game, x - half, y + uy) == Tile::JUMP_PAD ||
            getTile(game, x + half, y + uy) == Tile::JUMP_PAD) {
            me.jumpState.canJump = true;
            me.jumpState.canCancel = false;
            me.jumpState.maxTime = jumpPadJumpTime;
            me.jumpState.speed = jumpPadJumpSpeed;
            vy = me.jumpState.speed * alpha;
        }

        if (tick > 10 && (tick + game.currentTick) % 10 == 0) {
            auto coeff = 1.0f - tick * 1.0f / ticks;
            drawUnit(me, ColorFloat(0.2f * coeff, 0.2f * coeff, 1.0f * coeff, 1.0), debug);
        }
        if (tick == 0) {
            debug.log(string("next: ") + me.toString());
            prediction = me;
        }
    }
}

string surroundingToString(const Vec& v, const Level& level) {
    string ans = "";
    for (int dy = 2; dy >= -2; dy--) {
        for (int dx = -2; dx <= 2; dx++) {
            auto z = v + Vec(dx, dy);
            auto tile = 0 <= z.x && z.x < level.tiles.size() && 0 <= z.y && z.y < level.tiles[0].size()
                ? getTile(level, z) : Tile::WALL;
            ans += tileToChar(tile);
        }
        ans += "\n";
    }
    return ans;
}

int errors = 0;

UnitAction MyStrategy::getAction(const Unit& me, const Game& game, Debug& debug) {
    auto tick = game.currentTick;
    if (tick == 0) {
        if (params.find("--dump-constants") != params.end()) {
            dumpConstants(game.properties);
            terminate();
        }
        checkConstants(game.properties);
    }

    static auto moves = getRandomMoveSequence();
    UnitAction ans;
    if (moves->empty()) return ans;

    cout << tick << " " << me.toString() << endl;
    if (tick != 0 && prediction.toString() != me.toString()) {
        cout << "ERROR! predicted:" << endl << tick << " " << prediction.toString() << endl;
        cout << surroundingToString(me.position, game.level);
        errors++;
    }
    if (tick == game.properties.maxTickCount - 1) {
        cout << "TOTAL ERRORS: " << errors << endl;
    }

    simulate(me, game, *moves, debug, 300);
    ans = *moves->begin();
    moves->erase(moves->begin());
    return ans;
    /*
    auto nearestEnemy = minBy(game.units, [&me](auto& other) {
        return other.playerId != me.playerId ? me.position.sqrDist(other.position) : 1e100;
    });
    auto nearestWeapon = minBy(game.lootBoxes, [&me](auto& lootBox) {
        return lootBox.item.isWeapon() ? me.position.sqrDist(lootBox.position) : 1e100;
    });
    Vec target = me.position;
    if (me.weapon == nullptr && nearestWeapon != nullptr) {
        target = nearestWeapon->position;
    } else if (nearestEnemy != nullptr) {
        target = nearestEnemy->position;
    }
    debug.log(string("Me: ") + me.position.toString());
    debug.log(string("Target: ") + target.toString());
    Vec aim;
    if (nearestEnemy != nullptr) {
        aim = Vec(nearestEnemy->position.x - me.position.x, nearestEnemy->position.y - me.position.y);
    }
    bool jump = target.y > me.position.y;
    if (target.x > me.position.x && game.level.tiles[me.position.x + 1][me.position.y] == Tile::WALL) {
        jump = true;
    }
    if (target.x < me.position.x && game.level.tiles[me.position.x - 1][me.position.y] == Tile::WALL) {
        jump = true;
    }
    UnitAction action;
    action.velocity = target.x - me.position.x;
    action.jump = jump;
    action.jumpDown = !action.jump;
    action.aim = aim;
    action.shoot = true;
    action.swapWeapon = false;
    action.plantMine = false;
    return action;
    */
}
