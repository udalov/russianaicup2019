#ifndef _MODEL_MINE_HPP_
#define _MODEL_MINE_HPP_

#include "../Stream.hpp"
#include <optional>
#include <string>
#include "MineState.hpp"
#include "Vec2D.h"
#include "../Const.h"

class Mine {
public:
    Vec2Double position;
    MineState state;
    std::optional<double> timer;
    Mine();
    Mine(Vec2Double position, MineState state, std::optional<double> timer);
    static Mine readFrom(InputStream& stream);
    std::string toString() const;

    constexpr Vec center() const {
        return position + Vec(0, mineSize.y / 2);
    }
};

#endif
