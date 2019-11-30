#ifndef _MODEL_MINE_HPP_
#define _MODEL_MINE_HPP_

#include "../Stream.hpp"
#include <memory>
#include <string>
#include <stdexcept>
#include "MineState.hpp"
#include "ExplosionParams.hpp"
#include "Vec2D.h"

class Mine {
public:
    int playerId;
    Vec2Double position;
    Vec2Double size;
    MineState state;
    std::shared_ptr<double> timer;
    double triggerRadius;
    ExplosionParams explosionParams;
    Mine();
    Mine(int playerId, Vec2Double position, Vec2Double size, MineState state, std::shared_ptr<double> timer, double triggerRadius, ExplosionParams explosionParams);
    static Mine readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;
};

#endif
