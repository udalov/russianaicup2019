#ifndef _MODEL_WEAPON_PARAMS_HPP_
#define _MODEL_WEAPON_PARAMS_HPP_

#include "../Stream.hpp"
#include <optional>
#include <string>
#include "BulletParams.hpp"
#include "ExplosionParams.hpp"

class WeaponParams {
public:
    int magazineSize;
    double fireRate;
    double reloadTime;
    double minSpread;
    double maxSpread;
    double recoil;
    double aimSpeed;
    BulletParams bullet;
    std::optional<ExplosionParams> explosion;
    WeaponParams();
    constexpr WeaponParams(int magazineSize, double fireRate, double reloadTime, double minSpread, double maxSpread, double recoil, double aimSpeed, BulletParams bullet, std::optional<ExplosionParams> explosion) : magazineSize(magazineSize), fireRate(fireRate), reloadTime(reloadTime), minSpread(minSpread), maxSpread(maxSpread), recoil(recoil), aimSpeed(aimSpeed), bullet(bullet), explosion(explosion) { }
    static WeaponParams readFrom(InputStream& stream);
    bool operator==(const WeaponParams& other) const;
    std::string toString() const;
};

#endif
