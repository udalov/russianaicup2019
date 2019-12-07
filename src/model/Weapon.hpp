#ifndef _MODEL_WEAPON_HPP_
#define _MODEL_WEAPON_HPP_

#include "../Stream.hpp"
#include <memory>
#include <string>
#include "WeaponType.hpp"
#include "WeaponParams.hpp"

class Weapon {
public:
    WeaponType typ;
    WeaponParams params;
    int magazine;
    bool wasShooting;
    double spread;
    std::shared_ptr<double> fireTimer;
    std::shared_ptr<double> lastAngle;
    std::shared_ptr<int> lastFireTick;
    Weapon();
    Weapon(WeaponType typ, WeaponParams params, int magazine, bool wasShooting, double spread, std::shared_ptr<double> fireTimer, std::shared_ptr<double> lastAngle, std::shared_ptr<int> lastFireTick);
    static Weapon readFrom(InputStream& stream);
    std::string toString() const;
};

#endif
