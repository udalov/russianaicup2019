#include "MyStrategy.hpp"

#include "util.h"

using namespace std;

MyStrategy::MyStrategy() {}

double distanceSqr(Vec a, Vec b) {
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

UnitAction MyStrategy::getAction(const Unit &unit, const Game &game, Debug &debug) {
    auto nearestEnemy = minBy(game.units, [&unit](auto& other) {
        return other.playerId != unit.playerId ? distanceSqr(unit.position, other.position) : 1e100;
    });
    auto nearestWeapon = minBy(game.lootBoxes, [&unit](auto& lootBox) {
        return dynamic_pointer_cast<Item::Weapon>(lootBox.item) ? distanceSqr(unit.position, lootBox.position) : 1e100;
    });
    Vec targetPos = unit.position;
    if (unit.weapon == nullptr && nearestWeapon != nullptr) {
        targetPos = nearestWeapon->position;
    } else if (nearestEnemy != nullptr) {
        targetPos = nearestEnemy->position;
    }
    debug.draw(CustomData::Log(string("Target pos: ") + targetPos.toString()));
    Vec aim;
    if (nearestEnemy != nullptr) {
        aim = Vec(nearestEnemy->position.x - unit.position.x, nearestEnemy->position.y - unit.position.y);
    }
    bool jump = targetPos.y > unit.position.y;
    if (targetPos.x > unit.position.x && game.level.tiles[unit.position.x + 1][unit.position.y] == Tile::WALL) {
        jump = true;
    }
    if (targetPos.x < unit.position.x && game.level.tiles[unit.position.x - 1][unit.position.y] == Tile::WALL) {
        jump = true;
    }
    UnitAction action;
    action.velocity = targetPos.x - unit.position.x;
    action.jump = jump;
    action.jumpDown = !action.jump;
    action.aim = aim;
    action.shoot = true;
    action.swapWeapon = false;
    action.plantMine = false;
    return action;
}
