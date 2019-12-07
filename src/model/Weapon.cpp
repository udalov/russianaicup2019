#include "Weapon.hpp"

Weapon::Weapon() { }
Weapon::Weapon(WeaponType type, WeaponParams params, int magazine, bool wasShooting, double spread, std::shared_ptr<double> fireTimer, std::shared_ptr<double> lastAngle, std::shared_ptr<int> lastFireTick) : type(type), params(params), magazine(magazine), wasShooting(wasShooting), spread(spread), fireTimer(fireTimer), lastAngle(lastAngle), lastFireTick(lastFireTick) { }
Weapon Weapon::readFrom(InputStream& stream) {
    Weapon result;
    switch (stream.readInt()) {
    case 0:
        result.type = WeaponType::PISTOL;
        break;
    case 1:
        result.type = WeaponType::ASSAULT_RIFLE;
        break;
    case 2:
        result.type = WeaponType::ROCKET_LAUNCHER;
        break;
    default:
        throw std::runtime_error("Unexpected discriminant value");
    }
    result.params = WeaponParams::readFrom(stream);
    result.magazine = stream.readInt();
    result.wasShooting = stream.readBool();
    result.spread = stream.readDouble();
    if (stream.readBool()) {
        result.fireTimer = std::shared_ptr<double>(new double());
        *result.fireTimer = stream.readDouble();
    } else {
        result.fireTimer = std::shared_ptr<double>();
    }
    if (stream.readBool()) {
        result.lastAngle = std::shared_ptr<double>(new double());
        *result.lastAngle = stream.readDouble();
    } else {
        result.lastAngle = std::shared_ptr<double>();
    }
    if (stream.readBool()) {
        result.lastFireTick = std::shared_ptr<int>(new int());
        *result.lastFireTick = stream.readInt();
    } else {
        result.lastFireTick = std::shared_ptr<int>();
    }
    return result;
}
std::string Weapon::toString() const {
    // TODO
    return weaponTypeToString(type);
}
