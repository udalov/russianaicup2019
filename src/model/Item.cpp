#include "Item.hpp"

using namespace std;

Item::Item() {}
string Item::toString() const {
    switch (data.index()) {
        case 0: return string("HP(") + to_string(health()) + ")";
        case 1: return string("W(") + weaponTypeToString(weaponType()) + ")";
        case 2: return "M";
        default: return "?";
    }
}
WeaponType readWeaponType(InputStream& stream) {
    switch (stream.readInt()) {
        case 0: return WeaponType::PISTOL;
        case 1: return WeaponType::ASSAULT_RIFLE;
        case 2: return WeaponType::ROCKET_LAUNCHER;
        default: throw std::runtime_error("Unexpected discriminant value");
    }
}
Item Item::readFrom(InputStream& stream) {
    switch (stream.readInt()) {
        case 0: return Item(decltype(data)(stream.readInt()));
        case 1: return Item(decltype(data)(readWeaponType(stream)));
        case 2: return Item(decltype(data)(tuple<>()));
        default: throw std::runtime_error("Unexpected discriminant value");
    }
};
