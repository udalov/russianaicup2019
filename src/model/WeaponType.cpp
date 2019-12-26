#include "WeaponType.hpp"

using namespace std;

string weaponTypeToString(WeaponType type) {
    switch (type) {
        case WeaponType::PISTOL: return "P";
        case WeaponType::ASSAULT_RIFLE: return "AR";
        case WeaponType::ROCKET_LAUNCHER: return "RL";
        default: return "?";
    }
}
