#include "BulletParams.hpp"

BulletParams::BulletParams() { }
BulletParams BulletParams::readFrom(InputStream& stream) {
    BulletParams result;
    result.speed = stream.readDouble();
    result.size = stream.readDouble();
    result.damage = stream.readInt();
    return result;
}
std::string BulletParams::toString() const {
    return std::string("BulletParams") + "(" +
        std::to_string(speed) +
        std::to_string(size) +
        std::to_string(damage) +
        ")";
}
