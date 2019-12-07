#include "PlayerMessageGame.hpp"

PlayerMessageGame::CustomDataMessage::CustomDataMessage(std::shared_ptr<CustomData> data) : data(data) { }
void PlayerMessageGame::CustomDataMessage::writeTo(OutputStream& stream) const {
    stream.write(TAG);
    data->writeTo(stream);
}

PlayerMessageGame::ActionMessage::ActionMessage(Versioned action) : action(action) { }
void PlayerMessageGame::ActionMessage::writeTo(OutputStream& stream) const {
    stream.write(TAG);
    action.writeTo(stream);
}
