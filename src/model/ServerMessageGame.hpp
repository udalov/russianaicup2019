#ifndef _MODEL_SERVER_MESSAGE_GAME_HPP_
#define _MODEL_SERVER_MESSAGE_GAME_HPP_

#include "../Stream.hpp"
#include <optional>
#include "PlayerView.hpp"

class ServerMessageGame {
public:
    std::optional<PlayerView> playerView;
    ServerMessageGame();
    ServerMessageGame(PlayerView&& playerView);
    static ServerMessageGame readFrom(InputStream& stream);
};

#endif
