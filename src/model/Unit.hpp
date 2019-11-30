#ifndef _MODEL_UNIT_HPP_
#define _MODEL_UNIT_HPP_

#include "../Stream.hpp"
#include <memory>
#include <string>
#include <stdexcept>
#include "JumpState.hpp"
#include "Weapon.hpp"
#include "WeaponType.hpp"
#include "WeaponParams.hpp"
#include "BulletParams.hpp"
#include "ExplosionParams.hpp"
#include "Vec2D.h"

class Unit {
public:
    int playerId;
    int id;
    int health;
    Vec2Double position;
    Vec2Double size;
    JumpState jumpState;
    bool walkedRight;
    bool stand;
    bool onGround;
    bool onLadder;
    int mines;
    std::shared_ptr<Weapon> weapon;
    Unit();
    Unit(int playerId, int id, int health, Vec2Double position, Vec2Double size, JumpState jumpState, bool walkedRight, bool stand, bool onGround, bool onLadder, int mines, std::shared_ptr<Weapon> weapon);
    static Unit readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;
};

#endif
