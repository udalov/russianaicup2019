#ifndef _MODEL_GAME_HPP_
#define _MODEL_GAME_HPP_

#include "../Stream.hpp"
#include <memory>
#include <string>
#include <vector>
#include "Bullet.hpp"
#include "Level.hpp"
#include "LootBox.hpp"
#include "Mine.hpp"
#include "Player.hpp"
#include "Properties.hpp"
#include "Unit.hpp"

class Game {
public:
    int currentTick;
    Properties properties;
    Level level;
    std::vector<Player> players;
    std::vector<Unit> units;
    std::vector<Bullet> bullets;
    std::vector<Mine> mines;
    std::vector<LootBox> lootBoxes;
    Game();
    Game(int currentTick, Properties properties, Level level, std::vector<Player> players, std::vector<Unit> units, std::vector<Bullet> bullets, std::vector<Mine> mines, std::vector<LootBox> lootBoxes);
    static Game readFrom(InputStream& stream);
};

#endif
