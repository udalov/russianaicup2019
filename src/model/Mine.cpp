#include "Mine.hpp"

using namespace std;

Mine::Mine() { }
Mine::Mine(int playerId, Vec2Double position, Vec2Double size, MineState state, optional<double> timer, double triggerRadius, ExplosionParams explosionParams) : playerId(playerId), position(position), size(size), state(state), timer(timer), triggerRadius(triggerRadius), explosionParams(explosionParams) { }
Mine Mine::readFrom(InputStream& stream) {
    Mine result;
    result.playerId = stream.readInt();
    result.position = Vec2Double::readFrom(stream);
    result.size = Vec2Double::readFrom(stream);
    switch (stream.readInt()) {
        case 0: result.state = MineState::PREPARING; break;
        case 1: result.state = MineState::IDLE; break;
        case 2: result.state = MineState::TRIGGERED; break;
        case 3: result.state = MineState::EXPLODED; break;
        default: throw runtime_error("Unexpected discriminant value");
    }
    result.timer = stream.readBool() ? optional<double>(stream.readDouble()) : nullopt;
    result.triggerRadius = stream.readDouble();
    result.explosionParams = ExplosionParams::readFrom(stream);
    return result;
}
string Mine::toString() const {
    return string("Mine") + "(" +
        to_string(playerId) +
        position.toString() +
        size.toString() +
        "TODO" + 
        "TODO" + 
        to_string(triggerRadius) +
        explosionParams.toString() +
        ")";
}
