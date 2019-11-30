#ifndef _MODEL_COLORED_VERTEX_HPP_
#define _MODEL_COLORED_VERTEX_HPP_

#include "../Stream.hpp"
#include <string>
#include <stdexcept>
#include "ColorFloat.hpp"
#include "Vec2D.h"

class ColoredVertex {
public:
    Vec2Double position;
    ColorFloat color;
    ColoredVertex();
    ColoredVertex(Vec2Double position, ColorFloat color);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;
};

#endif
