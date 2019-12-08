#include "WeaponParams.hpp"

using namespace std;

WeaponParams::WeaponParams() : magazineSize(), fireRate(), reloadTime(), minSpread(), maxSpread(), recoil(), aimSpeed(), bullet(), explosion() { }
WeaponParams WeaponParams::readFrom(InputStream& stream) {
    WeaponParams result;
    result.magazineSize = stream.readInt();
    result.fireRate = stream.readDouble();
    result.reloadTime = stream.readDouble();
    result.minSpread = stream.readDouble();
    result.maxSpread = stream.readDouble();
    result.recoil = stream.readDouble();
    result.aimSpeed = stream.readDouble();
    result.bullet = BulletParams::readFrom(stream);
    result.explosion = stream.readBool() ? optional<ExplosionParams>(ExplosionParams::readFrom(stream)) : nullopt;
    return result;
}
bool WeaponParams::operator==(const WeaponParams& other) const {
    return magazineSize == other.magazineSize &&
        fireRate == other.fireRate &&
        reloadTime == other.reloadTime &&
        minSpread == other.minSpread &&
        maxSpread == other.maxSpread &&
        recoil == other.recoil &&
        aimSpeed == other.aimSpeed &&
        bullet.speed == other.bullet.speed &&
        bullet.size == other.bullet.size &&
        bullet.damage == other.bullet.damage &&
        explosion == other.explosion;
}
std::string WeaponParams::toString() const {
    return std::string("WeaponParams") + "(" +
        std::to_string(magazineSize) +
        std::to_string(fireRate) +
        std::to_string(reloadTime) +
        std::to_string(minSpread) +
        std::to_string(maxSpread) +
        std::to_string(recoil) +
        std::to_string(aimSpeed) +
        bullet.toString() +
        "TODO" + 
        ")";
}
