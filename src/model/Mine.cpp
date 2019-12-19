#include "Mine.hpp"

#include "ExplosionParams.hpp"

using namespace std;

Mine::Mine() { }
Mine::Mine(int playerId, Vec2Double position, MineState state, optional<double> timer, double triggerRadius) : playerId(playerId), position(position), state(state), timer(timer), triggerRadius(triggerRadius) { }
Mine Mine::readFrom(InputStream& stream) {
    Mine result;
    result.playerId = stream.readInt();
    result.position = Vec2Double::readFrom(stream);
    Vec2Double::readFrom(stream); // size
    switch (stream.readInt()) {
        case 0: result.state = MineState::PREPARING; break;
        case 1: result.state = MineState::IDLE; break;
        case 2: result.state = MineState::TRIGGERED; break;
        case 3: result.state = MineState::EXPLODED; break;
        default: throw runtime_error("Unexpected discriminant value");
    }
    result.timer = stream.readBool() ? optional<double>(stream.readDouble()) : nullopt;
    result.triggerRadius = stream.readDouble();
    ExplosionParams::readFrom(stream);
    return result;
}
string Mine::toString() const {
    return string("[M ") + position.toString() + " " +
        (timer.has_value() ? to_string(*timer) + "s" : "-") + "]";
}
