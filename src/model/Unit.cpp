#include "Unit.hpp"

using namespace std;

Unit::Unit(): playerId(), id(), health(), position(), jumpState(), onLadder(), mines(), weapon() { }
Unit::Unit(int playerId, int id, int health, Vec2Double position, JumpState jumpState, bool onLadder, int mines, optional<Weapon> weapon) : playerId(playerId), id(id), health(health), position(position), jumpState(jumpState), onLadder(onLadder), mines(mines), weapon(weapon) { }
Unit Unit::readFrom(InputStream& stream) {
    Unit result;
    result.playerId = stream.readInt();
    result.id = stream.readInt();
    result.health = stream.readInt();
    result.position = Vec2Double::readFrom(stream);
    Vec2Double::readFrom(stream); // size
    result.jumpState = JumpState::readFrom(stream);
    stream.readBool(); // walkedRight
    stream.readBool(); // stand
    stream.readBool(); // onGround
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
        (weapon.has_value() ? " {" + weapon->toString() + "}" : "");
}
