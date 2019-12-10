#ifndef _MODEL_PLAYER_MESSAGE_GAME_HPP_
#define _MODEL_PLAYER_MESSAGE_GAME_HPP_

#include "../Stream.hpp"
#include "CustomData.hpp"
#include "Versioned.hpp"

class PlayerMessageGame {
public:
    class ActionMessage;

    virtual void writeTo(OutputStream& stream) const = 0;
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
