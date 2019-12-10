#include "PlayerMessageGame.hpp"

PlayerMessageGame::ActionMessage::ActionMessage(Versioned action) : action(action) { }
void PlayerMessageGame::ActionMessage::writeTo(OutputStream& stream) const {
    stream.write(TAG);
    action.writeTo(stream);
}
