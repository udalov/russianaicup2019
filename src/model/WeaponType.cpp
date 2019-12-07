#include "WeaponType.hpp"

using namespace std;

string weaponTypeToString(WeaponType type) {
    return type == WeaponType::PISTOL ? "P"
        : type == WeaponType::ASSAULT_RIFLE ? "AR"
        : type == WeaponType::ROCKET_LAUNCHER ? "RL"
        : "?";
}
