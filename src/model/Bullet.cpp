#include "Bullet.hpp"

using namespace std;

Bullet::Bullet() { }
Bullet::Bullet(WeaponType weaponType, int unitId, int playerId, Vec2Double position, Vec2Double velocity, int damage, double size, optional<ExplosionParams> explosionParams) : weaponType(weaponType), unitId(unitId), playerId(playerId), position(position), velocity(velocity), damage(damage), size(size), explosionParams(explosionParams) { }
Bullet Bullet::readFrom(InputStream& stream) {
    Bullet result;
    switch (stream.readInt()) {
    case 0:
        result.weaponType = WeaponType::PISTOL;
        break;
    case 1:
        result.weaponType = WeaponType::ASSAULT_RIFLE;
        break;
    case 2:
        result.weaponType = WeaponType::ROCKET_LAUNCHER;
        break;
    default:
        throw runtime_error("Unexpected discriminant value");
    }
    result.unitId = stream.readInt();
    result.playerId = stream.readInt();
    result.position = Vec2Double::readFrom(stream);
    result.velocity = Vec2Double::readFrom(stream);
    result.damage = stream.readInt();
    result.size = stream.readDouble();
    result.explosionParams = stream.readBool() ? optional<ExplosionParams>(ExplosionParams::readFrom(stream)) : nullopt;
    return result;
}
string Bullet::toString() const {
    return string("[") + weaponTypeToString(weaponType) + " " + position.toString() + " v=" + velocity.toString() + "]";
}
