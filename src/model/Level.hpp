#ifndef _MODEL_LEVEL_HPP_
#define _MODEL_LEVEL_HPP_

#include "../Stream.hpp"
#include <string>
#include <vector>
#include "Tile.hpp"
#include "Vec2D.h"

class Level {
public:
    std::vector<std::vector<Tile>> tiles;
    Level();
    Level(std::vector<std::vector<Tile>> tiles);
    static Level readFrom(InputStream& stream);
    std::string toString() const;

    Tile operator()(double x, double y) const {
        auto i = static_cast<size_t>(x);
        auto j = static_cast<size_t>(y);
        return /* i >= tiles.size() || j >= tiles[0].size() ? Tile::WALL : */ tiles[i][j];
    }
    Tile operator()(const Vec& v) const {
        return this->operator()(v.x, v.y);
    }
    std::vector<std::string> toStrings() const;
};

#endif
