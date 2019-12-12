#ifndef _MODEL_WEAPON_HPP_
#define _MODEL_WEAPON_HPP_

#include "../Stream.hpp"
#include <optional>
#include <string>
#include "WeaponType.hpp"
#include "WeaponParams.hpp"

class Weapon {
public:
    WeaponType type;
    WeaponParams params;
    int magazine;
    bool wasShooting;
    double spread;
    double fireTimer;
    std::optional<double> lastAngle;
    std::optional<int> lastFireTick;
    Weapon();
    Weapon(WeaponType type, WeaponParams params, int magazine, bool wasShooting, double spread, double fireTimer, std::optional<double> lastAngle, std::optional<int> lastFireTick);
    static Weapon readFrom(InputStream& stream);
    std::string toString() const;
};

#endif
