#include "ExplosionParams.hpp"

ExplosionParams ExplosionParams::readFrom(InputStream& stream) {
    ExplosionParams result;
    result.radius = stream.readDouble();
    result.damage = stream.readInt();
    return result;
}
bool ExplosionParams::operator==(const ExplosionParams& other) const {
    return radius == other.radius && damage == other.damage;
}
std::string ExplosionParams::toString() const {
    return std::string("ExplosionParams") + "(" +
        std::to_string(radius) +
        std::to_string(damage) +
        ")";
}
