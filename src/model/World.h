#pragma once

#include "../Stream.hpp"
#include <vector>
#include "Bullet.hpp"
#include "LootBox.hpp"
#include "Mine.hpp"
#include "Unit.hpp"

class World {
public:
    std::vector<Unit> units;
    std::vector<Bullet> bullets;
    std::vector<Mine> mines;
    std::vector<LootBox> lootBoxes;
    World();
    World(const std::vector<Unit>& units, const std::vector<Bullet>& bullets, const std::vector<Mine>& mines, const std::vector<LootBox>& lootBoxes);
    static World readFrom(InputStream& stream);
};
