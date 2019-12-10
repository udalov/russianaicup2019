#ifndef _MODEL_MINE_HPP_
#define _MODEL_MINE_HPP_

#include "../Stream.hpp"
#include <optional>
#include <string>
#include "MineState.hpp"
#include "ExplosionParams.hpp"
#include "Vec2D.h"

class Mine {
public:
    int playerId;
    Vec2Double position;
    Vec2Double size;
    MineState state;
    std::optional<double> timer;
    double triggerRadius;
    ExplosionParams explosionParams;
    Mine();
    Mine(int playerId, Vec2Double position, Vec2Double size, MineState state, std::optional<double> timer, double triggerRadius, ExplosionParams explosionParams);
    static Mine readFrom(InputStream& stream);
    std::string toString() const;
};

#endif
