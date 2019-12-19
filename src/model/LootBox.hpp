#ifndef _MODEL_LOOT_BOX_HPP_
#define _MODEL_LOOT_BOX_HPP_

#include "../Stream.hpp"
#include <string>
#include "Item.hpp"
#include "Vec2D.h"
#include "../Const.h"

class LootBox {
public:
    Vec2Double position;
    Item item;
    LootBox();
    LootBox(Vec2Double position, Item item);
    static LootBox readFrom(InputStream& stream);
    std::string toString() const;

    constexpr Vec center() const {
        return position + Vec(0, lootBoxSize.y / 2);
    }
};

#endif
