#include "MyStrategy.hpp"

#include "util.h"

using namespace std;

MyStrategy::MyStrategy() {}

UnitAction MyStrategy::getAction(const Unit &me, const Game &game, Debug &debug) {
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
    debug.draw(CustomData::Log(string("Me: ") + me.position.toString()));
    debug.draw(CustomData::Log(string("Target: ") + target.toString()));
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
}
