#ifndef _MODEL_SERVER_MESSAGE_GAME_HPP_
#define _MODEL_SERVER_MESSAGE_GAME_HPP_

#include "../Stream.hpp"
#include <string>
#include <memory>
#include "PlayerView.hpp"

class ServerMessageGame {
public:
    std::shared_ptr<PlayerView> playerView;
    ServerMessageGame();
    ServerMessageGame(std::shared_ptr<PlayerView> playerView);
    static ServerMessageGame readFrom(InputStream& stream);
};

#endif
