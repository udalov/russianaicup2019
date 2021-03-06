#include "Weapon.hpp"

#include "modelUtil.h"

using namespace std;

Weapon::Weapon() : type(), params(), magazine(), spread(), fireTimer(), lastAngle() { }
Weapon::Weapon(WeaponType type, WeaponParams params, int magazine, double spread, double fireTimer, optional<double> lastAngle) : type(type), params(params), magazine(magazine), spread(spread), fireTimer(fireTimer), lastAngle(lastAngle) { }
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
    stream.readBool(); // wasShooting
    result.spread = stream.readDouble();
    result.fireTimer = stream.readBool() ? stream.readDouble() : 0.0;
    result.lastAngle = stream.readBool() ? optional<double>(stream.readDouble()) : nullopt;
    if (stream.readBool()) stream.readInt(); // lastFireTick
    return result;
}
string Weapon::toString() const {
    string ans = weaponTypeToString(type) + " " + to_string(magazine);
    // TODO: ans += " " + ::toString(spread);
    if (fireTimer > 0.0) ans += " " + ::toString(fireTimer, 3) + "s";
    // TODO: if (lastAngle.has_value()) ans += " " + ::toString(*lastAngle);
    return ans;
}
