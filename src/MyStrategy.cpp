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

void drawRect(Vec corner, Vec size, const ColorFloat& color, Debug& debug) {
    debug.draw(CustomData::Rect(corner, size, color));
}

void drawUnit(const Unit& unit, const ColorFloat& color, Debug& debug) {
    drawRect(unit.position - Vec(unitSize.x / 2, 0), unitSize, color, debug);
}

void drawBullet(const Bullet& bullet, const ColorFloat& color, Debug& debug) {
    auto size =
        bullet.weaponType == WeaponType::ROCKET_LAUNCHER ? rocketLauncherParams.bullet.size :
        bullet.weaponType == WeaponType::ASSAULT_RIFLE ? assaultRifleParams.bullet.size :
        bullet.weaponType == WeaponType::PISTOL ? pistolParams.bullet.size : 100.0;
    drawRect(bullet.position - Vec(size / 2, size / 2), Vec(size, size), color, debug);
}

bool isWall(const Game& game, double x, double y) {
    return getTile(game, x, y) == Tile::WALL;
}

bool isWallOrPlatform(const Game& game, double x, double y, bool jumpDown) {
    auto tile = getTile(game, x, y);
    // TODO: move tile == LADDER under !jumpDown check?
    return tile == Tile::WALL || tile == Tile::LADDER || (!jumpDown && tile == Tile::PLATFORM);
}

bool intersects(const Unit& unit, const Vec& smth, double sizeX, double sizeY) {
    auto pos = unit.center();
    return 2 * abs(pos.x - smth.x) <= unitSize.x + sizeX &&
        2 * abs(pos.y - smth.y) <= unitSize.y + sizeY;
}

bool intersects(const Unit& unit, const LootBox& box) {
    return intersects(unit, box.position, lootBoxSize.x, lootBoxSize.y);
}

bool intersectsEnemyBullet(const Unit& unit, const Bullet& bullet) {
    constexpr auto eps = 1e-8;
    return intersects(unit, bullet.position, bullet.size + eps, bullet.size + eps);
}

vector<LootBox> getReachableLootBoxes(const Unit& unit, const Game& game, size_t ticks) {
    auto loot = game.lootBoxes;
    auto v = unit.center();
    constexpr auto closenessX = unitSize.x + lootBoxSize.x;
    constexpr auto closenessY = unitSize.y + lootBoxSize.y;
    loot.erase(remove_if(loot.begin(), loot.end(), [&v, ticks](const auto& b) {
        constexpr auto maxSpeedX = 10.0 / ticksPerSecond;
        constexpr auto maxSpeedY = 20.0 / ticksPerSecond;
        return abs(v.x - b.position.x) > (unitSize.x + lootBoxSize.x) / 2 + maxSpeedX * ticks + EPS &&
            abs(v.y - b.position.y) > (unitSize.y + lootBoxSize.y) / 2 + maxSpeedY * ticks + EPS;
    }), loot.end());
    return loot;
}

pair<int, Game> prediction;

void simulate(int myId, Game game, const vector<UnitAction>& moves, Debug& debug, size_t ticks) {
    auto& me = *find_if(game.units.begin(), game.units.end(), [myId](const auto& unit) { return unit.id == myId; });
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
            for (size_t i = 0; i < game.bullets.size();) {
                auto& bullet = game.bullets[i];
                // Only simulate enemy bullets for now.
                if (bullet.playerId == me.playerId) { i++; continue; }

                bullet.position += bullet.velocity * alpha;
                auto bx = bullet.position.x;
                auto by = bullet.position.y;

                auto s = bullet.size / 2;
                if (isWall(game, bx - s, by - s) || isWall(game, bx - s, by + s) ||
                    isWall(game, bx + s, by - s) || isWall(game, bx + s, by + s)) {
                    swap(bullet, game.bullets.back());
                    game.bullets.pop_back();
                } else if (intersectsEnemyBullet(me, bullet)) {
                    me.health -= bullet.damage;
                    swap(bullet, game.bullets.back());
                    game.bullets.pop_back();
                } else {
                    i++;
                }
            }

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
        if (tick > 0 && (tick + game.currentTick) % 5 == 0) {
            auto coeff = 1.0f - tick * 1.0f / ticks;
            for (auto& bullet : game.bullets) {
                if (bullet.playerId != me.playerId) {
                    drawBullet(bullet, ColorFloat(0.7f * coeff, 0.2f * coeff, 0.1f * coeff, 0.8), debug);
                }
            }
        }
        if (tick == 0) {
            debug.log(string("next: ") + me.toString());
            prediction = make_pair(me.id, game);
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

string renderWorld(const pair<int, Game>& world) {
    auto myId = world.first;
    auto& game = world.second;
    auto me = *find_if(game.units.begin(), game.units.end(), [myId](const auto& unit) { return unit.id == myId; });
    string ans = me.toString();
    for (auto& bullet : game.bullets) {
        // Only render enemy bullets for now.
        if (bullet.playerId != me.playerId) {
            ans += "\n  " + bullet.toString();
        }
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

    if (tick != 0) {
        auto actual = renderWorld(make_pair(me.id, game));
        auto expected = renderWorld(prediction);
        cout << tick << " " << actual << endl;
        if (expected != actual) {
            cout << "ERROR! predicted:" << endl << tick << " " << expected << endl;
            if (expected.substr(0, expected.find("\n")) != actual.substr(0, actual.find("\n"))) {
                cout << surroundingToString(me.position, game.level);
            } else {
                cout << endl;
            }
            errors++;
        }
    }
    if (tick == game.properties.maxTickCount - 1) {
        cout << "TOTAL ERRORS: " << errors << endl;
    }

    simulate(me.id, game, *moves, debug, 300);
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
