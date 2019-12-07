#ifndef _MODEL_PLAYER_MESSAGE_GAME_HPP_
#define _MODEL_PLAYER_MESSAGE_GAME_HPP_

#include "../Stream.hpp"
#include <memory>
#include "CustomData.hpp"
#include "Versioned.hpp"

class PlayerMessageGame {
public:
    class CustomDataMessage;
    class ActionMessage;

    static std::shared_ptr<PlayerMessageGame> readFrom(InputStream& stream);
    virtual void writeTo(OutputStream& stream) const = 0;
};

class PlayerMessageGame::CustomDataMessage : public PlayerMessageGame {
public:
    static const int TAG = 0;
public:
    std::shared_ptr<CustomData> data;
    CustomDataMessage(std::shared_ptr<CustomData> data);
    void writeTo(OutputStream& stream) const;
};

class PlayerMessageGame::ActionMessage : public PlayerMessageGame {
public:
    static const int TAG = 1;
public:
    Versioned action;
    ActionMessage(Versioned action);
    void writeTo(OutputStream& stream) const;
};

#endif
