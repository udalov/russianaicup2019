#ifndef _MODEL_UNIT_HPP_
#define _MODEL_UNIT_HPP_

#include "../Stream.hpp"
#include <optional>
#include <string>
#include "JumpState.hpp"
#include "Weapon.hpp"
#include "Vec2D.h"
#include "../Const.h"

class Unit {
public:
    int playerId;
    int id;
    int health;
    Vec2Double position;
    JumpState jumpState;
    bool onGround;
    bool onLadder;
    int mines;
    std::optional<Weapon> weapon;
    Unit();
    Unit(int playerId, int id, int health, Vec2Double position, JumpState jumpState, bool onGround, bool onLadder, int mines, std::optional<Weapon> weapon);
    static Unit readFrom(InputStream& stream);
    std::string toString() const;

    constexpr Vec center() const {
        return position + Vec(0, unitSize.y / 2);
    }
};

#endif
