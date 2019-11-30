#ifndef _MODEL_PLAYER_VIEW_HPP_
#define _MODEL_PLAYER_VIEW_HPP_

#include "../Stream.hpp"
#include <memory>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include "Bullet.hpp"
#include "BulletParams.hpp"
#include "ExplosionParams.hpp"
#include "Game.hpp"
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

class PlayerView {
public:
    int myId;
    Game game;
    PlayerView();
    PlayerView(int myId, Game game);
    static PlayerView readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;
};

#endif
