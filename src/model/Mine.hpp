#ifndef _MODEL_MINE_HPP_
#define _MODEL_MINE_HPP_

#include "../Stream.hpp"
#include <optional>
#include <string>
#include "MineState.hpp"
#include "Vec2D.h"

class Mine {
public:
    int playerId;
    Vec2Double position;
    MineState state;
    std::optional<double> timer;
    double triggerRadius;
    Mine();
    Mine(int playerId, Vec2Double position, MineState state, std::optional<double> timer, double triggerRadius);
    static Mine readFrom(InputStream& stream);
    std::string toString() const;
};

#endif
