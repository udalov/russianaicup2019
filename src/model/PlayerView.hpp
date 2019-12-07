#ifndef _MODEL_PLAYER_VIEW_HPP_
#define _MODEL_PLAYER_VIEW_HPP_

#include "../Stream.hpp"
#include <memory>
#include <string>
#include "Game.hpp"

class PlayerView {
public:
    int myId;
    Game game;
    PlayerView();
    PlayerView(int myId, Game game);
    static PlayerView readFrom(InputStream& stream);
};

#endif
