#ifndef _MODEL_BULLET_HPP_
#define _MODEL_BULLET_HPP_

#include "../Stream.hpp"
#include <optional>
#include <string>
#include "WeaponType.hpp"
#include "ExplosionParams.hpp"
#include "Vec2D.h"

class Bullet {
public:
    WeaponType weaponType;
    int unitId;
    int playerId;
    Vec2Double position;
    Vec2Double velocity;
    int damage;
    double size;
    std::optional<ExplosionParams> explosionParams;
    Bullet();
    Bullet(WeaponType weaponType, int unitId, int playerId, Vec2Double position, Vec2Double velocity, int damage, double size, std::optional<ExplosionParams> explosionParams);
    static Bullet readFrom(InputStream& stream);
    std::string toString() const;
};

#endif
