#ifndef _MODEL_BULLET_HPP_
#define _MODEL_BULLET_HPP_

#include "../Stream.hpp"
#include <string>
#include "WeaponType.hpp"
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
    Bullet();
    Bullet(WeaponType weaponType, int unitId, int playerId, Vec2Double position, Vec2Double velocity, int damage, double size);
    static Bullet readFrom(InputStream& stream);
    std::string toString() const;
};

#endif
