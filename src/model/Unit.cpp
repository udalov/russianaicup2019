#include "Unit.hpp"

#include "../Const.h"

using namespace std;

Unit::Unit(): playerId(), id(), health(), position(), size(), jumpState(), walkedRight(), stand(), onGround(), onLadder(), mines(), weapon() { }
Unit::Unit(int playerId, int id, int health, Vec2Double position, Vec2Double size, JumpState jumpState, bool walkedRight, bool stand, bool onGround, bool onLadder, int mines, optional<Weapon> weapon) : playerId(playerId), id(id), health(health), position(position), size(size), jumpState(jumpState), walkedRight(walkedRight), stand(stand), onGround(onGround), onLadder(onLadder), mines(mines), weapon(weapon) { }
Unit Unit::readFrom(InputStream& stream) {
    Unit result;
    result.playerId = stream.readInt();
    result.id = stream.readInt();
    result.health = stream.readInt();
    result.position = Vec2Double::readFrom(stream);
    result.size = Vec2Double::readFrom(stream);
    result.jumpState = JumpState::readFrom(stream);
    result.walkedRight = stream.readBool();
    result.stand = stream.readBool();
    result.onGround = stream.readBool();
    result.onLadder = stream.readBool();
    result.mines = stream.readInt();
    result.weapon = stream.readBool() ? optional<Weapon>(Weapon::readFrom(stream)) : nullopt;
    return result;
}
string Unit::toString() const {
    return position.toString() + " " +
        to_string(health) + " " +
        jumpState.toString() + " " +
        // (walkedRight ? "R" : ".") +
        // (stand ? "S" : ".") +
        // (onGround ? "G" : ".") +
        (onLadder ? "L" : ".") +
        (weapon.has_value() ? " " + weapon->toString() : "");
}
Vec Unit::center() const {
    return position + Vec(0, unitSize.y / 2);
}
