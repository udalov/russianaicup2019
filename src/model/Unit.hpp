#ifndef _MODEL_UNIT_HPP_
#define _MODEL_UNIT_HPP_

#include "../Stream.hpp"
#include <optional>
#include <string>
#include "JumpState.hpp"
#include "Weapon.hpp"
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
    std::optional<Weapon> weapon;
    Unit();
    Unit(int playerId, int id, int health, Vec2Double position, Vec2Double size, JumpState jumpState, bool walkedRight, bool stand, bool onGround, bool onLadder, int mines, std::optional<Weapon> weapon);
    static Unit readFrom(InputStream& stream);
    std::string toString() const;

    Vec center() const;
};

#endif
