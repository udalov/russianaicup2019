#include "MyStrategy.hpp"

#include <iostream>
#include <vector>
#include "Const.h"
#include "util.h"

using namespace std;

const size_t MICROTICKS = 100;
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

    auto t = 100 / MICROTICKS;
    size_t cp1 = 13 * t;
    size_t cp2 = 25 * t;
    size_t cp3 = 186 * t;
    size_t cp4 = 1000 * t;

    for (size_t i = 0; i < cp1; i++) ans->push_back(moveRight);
    for (size_t i = cp1; i < cp2; i++) ans->push_back(jump);
    for (size_t i = cp2; i < cp3; i++) ans->push_back(jumpAndMoveRight);
    for (size_t i = cp3; i < cp4; i++) ans->push_back(left);
    return ans;
}

Tile getTile(const Game& game, double x, double y) {
    auto i = static_cast<size_t>(x);
    auto j = static_cast<size_t>(y);
    return /* i >= game.level.tiles.size() || j >= game.level.tiles[0].size() ? Tile::WALL : */ game.level.tiles[i][j];
}

Tile getTile(const Game& game, Vec v) {
    return getTile(game, v.x, v.y);
}

void drawUnit(const Unit& unit, const Game& game, const ColorFloat& color, Debug& debug) {
    auto corner = unit.position - Vec(game.properties.unitSize.x / 2, 0);
    debug.draw(CustomData::Rect(corner, game.properties.unitSize, color));
}

bool isWall(const Game& game, double x, double y) {
    return getTile(game, x, y) == Tile::WALL;
}

bool isWallOrPlatform(const Game& game, double x, double y, bool jumpDown) {
    auto tile = getTile(game, x, y);
    // TODO: move tile == LADDER under !jumpDown check?
    return tile == Tile::WALL || tile == Tile::LADDER || (!jumpDown && tile == Tile::PLATFORM);
}

Unit prediction;

void simulate(Unit me, Game game, const vector<UnitAction>& moves, Debug& debug, size_t ticks) {
    static auto unitMaxHorizontalSpeed = game.properties.unitMaxHorizontalSpeed;
    static auto ticksPerSecond = game.properties.ticksPerSecond;
    static auto unitSize = game.properties.unitSize;

    auto alpha = 1.0 / ticksPerSecond / MICROTICKS;

    debug.log(string("cur: ") + me.toString());

    auto& x = me.position.x;
    auto& y = me.position.y;
    auto ux = unitSize.x;
    auto uy = unitSize.y;
    auto half = ux / 2;

    for (size_t tick = 0; tick < ticks; tick++) {
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
                vy = -game.properties.unitFallSpeed * alpha;
            }
        }

        for (size_t mt = 0; mt < MICROTICKS; mt++) {
            if (vx < 0 && (
                isWall(game, x - half + vx, y) ||
                isWall(game, x - half + vx, y + uy/2) ||
                isWall(game, x - half + vx, y + uy)
            )) {
                vx = floor(x - half) - (x - half) + EPS;
            } else if (vx > 0 && (
                isWall(game, x + half + vx, y) ||
                isWall(game, x + half + vx, y + uy/2) ||
                isWall(game, x + half + vx, y + uy)
            )) {
                vx = ceil(x + half) - (x + half) - EPS;
            }

            x += vx;

            if (vy < 0 && (
                isWallOrPlatform(game, x - half, y + vy, move.jumpDown) ||
                isWallOrPlatform(game, x + half, y + vy, move.jumpDown)
            )) {
                vy = floor(y) - y + EPS;
                me.jumpState.canJump = true;
                me.jumpState.canCancel = true;
                me.jumpState.maxTime = game.properties.unitJumpTime;
                me.jumpState.speed = game.properties.unitJumpSpeed;
            } else if (vy > 0) {
                if (!me.onLadder) {
                    me.jumpState.maxTime -= alpha;
                    if (isWall(game, x - half, y + uy + vy) || isWall(game, x + half, y + uy + vy)) {
                        vy = ceil(y + uy) - (y + uy) - EPS;
                        me.jumpState.canJump = false;
                        me.jumpState.canCancel = false;
                        me.jumpState.maxTime = 0.0;
                        me.jumpState.speed = 0.0;
                    }
                }
            }

            y += vy;

            if (getTile(game, x, y) == Tile::LADDER) {
                me.onLadder = true;
                me.jumpState.maxTime = game.properties.unitJumpTime;
            } else {
                me.onLadder = false;
            }
        }

        if (me.jumpState.maxTime <= -EPS) {
            vy = 0.0;
            me.jumpState.canJump = false;
            me.jumpState.canCancel = false;
            me.jumpState.maxTime = 0.0;
            me.jumpState.speed = 0.0;
        }

        if (getTile(game, x - half, y) == Tile::JUMP_PAD ||
            getTile(game, x + half, y) == Tile::JUMP_PAD ||
            getTile(game, x - half, y + uy) == Tile::JUMP_PAD ||
            getTile(game, x + half, y + uy) == Tile::JUMP_PAD) {
            me.jumpState.canJump = true;
            me.jumpState.canCancel = false;
            me.jumpState.maxTime = game.properties.jumpPadJumpTime;
            me.jumpState.speed = game.properties.jumpPadJumpSpeed;
        }

        if (tick > 10 && (tick + game.currentTick) % 10 == 0) {
            auto coeff = 1.0f - tick * 1.0f / ticks;
            drawUnit(me, game, ColorFloat(0.2f * coeff, 0.2f * coeff, 1.0f * coeff, 1.0), debug);
        }
        if (tick == 0) {
            debug.log(string("next: ") + me.toString());
            prediction = me;
        }
    }
}

UnitAction MyStrategy::getAction(const Unit& me, const Game& game, Debug& debug) {
    auto tick = game.currentTick;
    if (tick == 0) {
        if (params.find("--dump-constants") != params.end()) {
            dumpConstants(game.properties);
            terminate();
        }
    }

    static auto moves = getRandomMoveSequence();
    UnitAction ans;
    if (moves->empty()) return ans;

    cout << tick << " " << me.toString() << endl;
    if (tick != 0 && prediction.toString() != me.toString()) {
        cout << "ERROR! predicted:" << endl << tick << " " << prediction.toString() << endl << endl;
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
        return dynamic_pointer_cast<Item::Weapon>(lootBox.item) ? me.position.sqrDist(lootBox.position) : 1e100;
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
