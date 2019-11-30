#ifndef _MODEL_SERVER_MESSAGE_GAME_HPP_
#define _MODEL_SERVER_MESSAGE_GAME_HPP_

#include "../Stream.hpp"
#include <string>
#include <memory>
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
#include "PlayerView.hpp"
#include "Properties.hpp"
#include "Tile.hpp"
#include "Unit.hpp"
#include "Weapon.hpp"
#include "WeaponParams.hpp"
#include "WeaponType.hpp"

class ServerMessageGame {
public:
    std::shared_ptr<PlayerView> playerView;
    ServerMessageGame();
    ServerMessageGame(std::shared_ptr<PlayerView> playerView);
    static ServerMessageGame readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;
};

#endif
