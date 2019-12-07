#ifndef _MODEL_LOOT_BOX_HPP_
#define _MODEL_LOOT_BOX_HPP_

#include "../Stream.hpp"
#include <string>
#include "Item.hpp"
#include "Vec2D.h"

class LootBox {
public:
    Vec2Double position;
    Vec2Double size;
    std::shared_ptr<Item> item;
    LootBox();
    LootBox(Vec2Double position, Vec2Double size, std::shared_ptr<Item> item);
    static LootBox readFrom(InputStream& stream);
    std::string toString() const;
};

#endif
