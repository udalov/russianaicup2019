#include "JumpState.hpp"

#include "modelUtil.h"

JumpState::JumpState() { }
JumpState::JumpState(bool canJump, double speed, double maxTime, bool canCancel) : canJump(canJump), speed(speed), maxTime(maxTime), canCancel(canCancel) { }
JumpState JumpState::readFrom(InputStream& stream) {
    JumpState result;
    result.canJump = stream.readBool();
    result.speed = stream.readDouble();
    result.maxTime = stream.readDouble();
    result.canCancel = stream.readBool();
    return result;
}
std::string JumpState::toString() const {
    return std::string(canJump ? "^" : ".") + 
        std::string(canCancel ? "v" : ".") + " " +
        ::toString(speed) + " " +
        ::toString(maxTime);
}
