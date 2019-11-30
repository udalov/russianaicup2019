#ifndef _MODEL_BULLET_HPP_
#define _MODEL_BULLET_HPP_

#include "../Stream.hpp"
#include <memory>
#include <string>
#include <stdexcept>
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
    std::shared_ptr<ExplosionParams> explosionParams;
    Bullet();
    Bullet(WeaponType weaponType, int unitId, int playerId, Vec2Double position, Vec2Double velocity, int damage, double size, std::shared_ptr<ExplosionParams> explosionParams);
    static Bullet readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;
};

#endif
