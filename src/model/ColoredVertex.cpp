#include "ColoredVertex.hpp"

ColoredVertex::ColoredVertex() { }
ColoredVertex::ColoredVertex(Vec2Double position, ColorFloat color) : position(position), color(color) { }
void ColoredVertex::writeTo(OutputStream& stream) const {
    position.writeFloatTo(stream);
    color.writeTo(stream);
}
std::string ColoredVertex::toString() const {
    return std::string("ColoredVertex") + "(" +
        position.toString() +
        color.toString() +
        ")";
}
