#include "Unit.hpp"

Unit::Unit() { }
Unit::Unit(int playerId, int id, int health, Vec2Double position, Vec2Double size, JumpState jumpState, bool walkedRight, bool stand, bool onGround, bool onLadder, int mines, std::shared_ptr<Weapon> weapon) : playerId(playerId), id(id), health(health), position(position), size(size), jumpState(jumpState), walkedRight(walkedRight), stand(stand), onGround(onGround), onLadder(onLadder), mines(mines), weapon(weapon) { }
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
    if (stream.readBool()) {
        result.weapon = std::shared_ptr<Weapon>(new Weapon());
        *result.weapon = Weapon::readFrom(stream);
    } else {
        result.weapon = std::shared_ptr<Weapon>();
    }
    return result;
}
void Unit::writeTo(OutputStream& stream) const {
    stream.write(playerId);
    stream.write(id);
    stream.write(health);
    position.writeTo(stream);
    size.writeTo(stream);
    jumpState.writeTo(stream);
    stream.write(walkedRight);
    stream.write(stand);
    stream.write(onGround);
    stream.write(onLadder);
    stream.write(mines);
    if (weapon) {
        stream.write(false);
    } else {
        stream.write(true);
        (*weapon).writeTo(stream);
    }
}
std::string Unit::toString() const {
    return position.toString() + " " +
        std::to_string(health) + " " +
        jumpState.toString() + " " +
        // (walkedRight ? "R" : ".") +
        // (stand ? "S" : ".") +
        // (onGround ? "G" : ".") +
        (onLadder ? "L" : ".") +
        (weapon ? " " + weapon->toString() : "");
}
