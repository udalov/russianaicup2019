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
    char flags;
    Bullet();
    Bullet(WeaponType weaponType, int unitId, int playerId, Vec2Double position, Vec2Double velocity, int damage, double size);
    static Bullet readFrom(InputStream& stream);
    std::string toString() const;

    bool needsMove() const { return (flags & 1) != 0; }
    bool isNearMines() const { return (flags & 2) != 0; }
    bool isNearUnits() const { return (flags & 4) != 0; }
    void setNeedsMove() { flags |= 1; }
    void setNearMines() { flags |= 2; }
    void setNearUnits() { flags |= 4; }
};

#endif
