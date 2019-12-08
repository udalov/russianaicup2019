#include "Weapon.hpp"

#include "modelUtil.h"

using namespace std;

Weapon::Weapon() : type(), params(), magazine(), wasShooting(), spread(), fireTimer(), lastAngle(), lastFireTick() { }
Weapon::Weapon(WeaponType type, WeaponParams params, int magazine, bool wasShooting, double spread, optional<double> fireTimer, optional<double> lastAngle, optional<int> lastFireTick) : type(type), params(params), magazine(magazine), wasShooting(wasShooting), spread(spread), fireTimer(fireTimer), lastAngle(lastAngle), lastFireTick(lastFireTick) { }
Weapon Weapon::readFrom(InputStream& stream) {
    Weapon result;
    switch (stream.readInt()) {
        case 0: result.type = WeaponType::PISTOL; break;
        case 1: result.type = WeaponType::ASSAULT_RIFLE; break;
        case 2: result.type = WeaponType::ROCKET_LAUNCHER; break;
        default: throw runtime_error("Unexpected discriminant value");
    }
    result.params = WeaponParams::readFrom(stream);
    result.magazine = stream.readInt();
    result.wasShooting = stream.readBool();
    result.spread = stream.readDouble();
    result.fireTimer = stream.readBool() ? optional<double>(stream.readDouble()) : nullopt;
    result.lastAngle = stream.readBool() ? optional<double>(stream.readDouble()) : nullopt;
    result.lastFireTick = stream.readBool() ? optional<int>(stream.readInt()) : nullopt;
    return result;
}
string Weapon::toString() const {
    string ans = weaponTypeToString(type) + " " + to_string(magazine);
    // TODO: ans += " " + ::toString(spread);
    if (fireTimer.has_value()) ans += " " + ::toString(*fireTimer) + "s";
    // TODO: if (lastAngle.has_value()) ans += " " + ::toString(*lastAngle);
    // if (lastFireTick.has_value()) ans += " " + to_string(*lastFireTick);
    return ans;
}
