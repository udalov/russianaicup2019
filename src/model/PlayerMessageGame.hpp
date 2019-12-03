#ifndef _MODEL_PLAYER_MESSAGE_GAME_HPP_
#define _MODEL_PLAYER_MESSAGE_GAME_HPP_

#include "../Stream.hpp"
#include <memory>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include "CustomData.hpp"
#include "ColorFloat.hpp"
#include "ColoredVertex.hpp"
#include "TextAlignment.hpp"
#include "UnitAction.hpp"
#include "Versioned.hpp"

class PlayerMessageGame {
public:
    class CustomDataMessage;
    class ActionMessage;

    static std::shared_ptr<PlayerMessageGame> readFrom(InputStream& stream);
    virtual void writeTo(OutputStream& stream) const = 0;
    virtual std::string toString() const = 0;
};

class PlayerMessageGame::CustomDataMessage : public PlayerMessageGame {
public:
    static const int TAG = 0;
public:
    std::shared_ptr<CustomData> data;
    CustomDataMessage();
    CustomDataMessage(std::shared_ptr<CustomData> data);
    static CustomDataMessage readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const override;
};

class PlayerMessageGame::ActionMessage : public PlayerMessageGame {
public:
    static const int TAG = 1;
public:
    Versioned action;
    ActionMessage();
    ActionMessage(Versioned action);
    static ActionMessage readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const override;
};

#endif
