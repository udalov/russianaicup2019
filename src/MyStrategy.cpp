#include "MyStrategy.hpp"

#include <iostream>
#include <vector>
#include "util.h"

using namespace std;

const size_t MICROTICKS = 100;
const double EPS = 1e-9;

MyStrategy::MyStrategy() {}

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

Tile getTile(const Game& game, Vec v) {
    auto i = static_cast<size_t>(v.x);
    auto j = static_cast<size_t>(v.y);
    return /* i >= game.level.tiles.size() || j >= game.level.tiles[0].size() ? Tile::WALL : */ game.level.tiles[i][j];
}

void drawUnit(const Unit& unit, const Game& game, const ColorFloat& color, Debug& debug) {
    auto corner = unit.position - Vec(game.properties.unitSize.x / 2, 0);
    debug.draw(CustomData::Rect(corner, game.properties.unitSize, color));
}

bool isWall(Vec v, const Game& game) {
    return getTile(game, v) == Tile::WALL;
}

bool isWallOrPlatform(Vec v, bool jumpDown, const Game& game) {
    auto tile = getTile(game, v);
    return tile == Tile::WALL || tile == Tile::LADDER || (!jumpDown && tile == Tile::PLATFORM);
}

void simulate(Unit me, Game game, const vector<UnitAction>& moves, Debug& debug, size_t ticks) {
    static auto unitMaxHorizontalSpeed = game.properties.unitMaxHorizontalSpeed;
    static auto ticksPerSecond = game.properties.ticksPerSecond;
    static auto unitSize = game.properties.unitSize;

    auto alpha = 1.0 / ticksPerSecond / MICROTICKS;

    debug.log(string("cur: ") + me.toString());

    for (size_t tick = 0; tick < ticks; tick++) {
        auto& move = moves[tick];
        auto horizontalVelocity = Vec(min(max(move.velocity, -unitMaxHorizontalSpeed), unitMaxHorizontalSpeed) * alpha, 0.0);
        auto verticalVelocity = Vec();
        if (me.onLadder) {
            if (move.jump) {
                verticalVelocity.y = me.jumpState.speed * alpha;
            }
        } else {
            if (move.jump && me.jumpState.canJump && me.jumpState.maxTime >= -EPS) {
                verticalVelocity.y = me.jumpState.speed * alpha;
            } else {
                verticalVelocity.y = -game.properties.unitFallSpeed * alpha;
            }
        }

        for (size_t mt = 0; mt < MICROTICKS; mt++) {
            auto leftBottom = me.position + Vec(-unitSize.x / 2, 0);
            auto rightBottom = me.position + Vec(unitSize.x / 2, 0);
            auto leftMid = me.position + Vec(-unitSize.x / 2, unitSize.y / 2);
            auto rightMid = me.position + Vec(unitSize.x / 2, unitSize.y / 2);
            auto leftTop = me.position + Vec(-unitSize.x / 2, unitSize.y);
            auto rightTop = me.position + Vec(unitSize.x / 2, unitSize.y);

            if (horizontalVelocity.x < 0 && (
                isWall(leftBottom + horizontalVelocity, game) ||
                isWall(leftMid + horizontalVelocity, game) ||
                isWall(leftTop + horizontalVelocity, game)
            )) {
                horizontalVelocity.x = floor(leftBottom.x) - leftBottom.x + EPS;
            } else if (horizontalVelocity.x > 0 && (
                isWall(rightBottom + horizontalVelocity, game) ||
                isWall(rightMid + horizontalVelocity, game) ||
                isWall(rightTop + horizontalVelocity, game)
            )) {
                horizontalVelocity.x = ceil(rightBottom.x) - rightBottom.x - EPS;
            }

            me.position += horizontalVelocity;
            if (verticalVelocity.y < 0 && (
                isWallOrPlatform(leftBottom + verticalVelocity, move.jumpDown, game) ||
                isWallOrPlatform(rightBottom + verticalVelocity, move.jumpDown, game)
            )) {
                verticalVelocity.y = floor(me.position.y) - me.position.y + EPS;
                me.jumpState.canJump = true;
                me.jumpState.canCancel = true;
                me.jumpState.maxTime = game.properties.unitJumpTime;
                me.jumpState.speed = game.properties.unitJumpSpeed;
            } else if (verticalVelocity.y > 0) {
                if (!me.onLadder) {
                    me.jumpState.maxTime -= alpha;
                    if (isWall(leftTop + verticalVelocity, game) || isWall(rightTop + verticalVelocity, game)) {
                        verticalVelocity.y = ceil(leftTop.y) - leftTop.y - EPS;
                        me.jumpState.canJump = false;
                        me.jumpState.canCancel = false;
                        me.jumpState.maxTime = 0.0;
                        me.jumpState.speed = 0.0;
                    }
                }
            }

            me.position += verticalVelocity;

            if (getTile(game, me.position) == Tile::LADDER) {
                me.onLadder = true;
                me.jumpState.maxTime = game.properties.unitJumpTime;
            } else {
                me.onLadder = false;
            }
        }

        if (me.jumpState.maxTime <= -EPS) {
            verticalVelocity.y = 0.0;
            me.jumpState.canJump = false;
            me.jumpState.canCancel = false;
            me.jumpState.maxTime = 0.0;
            me.jumpState.speed = 0.0;
        }

        if (tick > 10 && (tick + game.currentTick) % 10 == 0) {
            auto coeff = 1.0f - tick * 1.0f / ticks;
            drawUnit(me, game, ColorFloat(0.2f * coeff, 0.2f * coeff, 1.0f * coeff, 1.0), debug);
        }
        if (tick == 0) {
            debug.log(string("next: ") + me.toString());
        }
    }
}

UnitAction MyStrategy::getAction(const Unit& me, const Game& game, Debug& debug) {
    auto tick = game.currentTick;
    static auto moves = getRandomMoveSequence();
    UnitAction ans;
    if (moves->empty()) return ans;
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
