#ifndef _MODEL_GAME_HPP_
#define _MODEL_GAME_HPP_

#include "../Stream.hpp"
#include <memory>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include "Bullet.hpp"
#include "BulletParams.hpp"
#include "ExplosionParams.hpp"
#include "Item.hpp"
#include "JumpState.hpp"
#include "Level.hpp"
#include "LootBox.hpp"
#include "Mine.hpp"
#include "MineState.hpp"
#include "Player.hpp"
#include "Properties.hpp"
#include "Tile.hpp"
#include "Unit.hpp"
#include "Weapon.hpp"
#include "WeaponParams.hpp"
#include "WeaponType.hpp"

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
    void writeTo(OutputStream& stream) const;
    std::string toString() const;
};

#endif
