#ifndef _MODEL_GAME_HPP_
#define _MODEL_GAME_HPP_

#include "../Stream.hpp"
#include <vector>
#include "Level.hpp"
#include "Player.hpp"
#include "Properties.hpp"
#include "World.h"

class Game {
public:
    int currentTick;
    Properties properties;
    Level level;
    std::vector<Player> players;
    World world;
    Game();
    Game(int currentTick, Properties properties, Level level, std::vector<Player> players, World world);
    static Game readFrom(InputStream& stream);
};

#endif
