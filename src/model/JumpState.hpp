#ifndef _MODEL_JUMP_STATE_HPP_
#define _MODEL_JUMP_STATE_HPP_

#include "../Stream.hpp"
#include <string>

class JumpState {
public:
    bool canJump;
    double speed;
    double maxTime;
    bool canCancel;
    JumpState();
    JumpState(bool canJump, double speed, double maxTime, bool canCancel);
    static JumpState readFrom(InputStream& stream);
    std::string toString() const;

    static JumpState NO_JUMP;
    static JumpState UNIT_JUMP;
    static JumpState JUMP_PAD_JUMP;
};

#endif
