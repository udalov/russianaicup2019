#include "LootBox.hpp"

using namespace std;

LootBox::LootBox() { }
LootBox::LootBox(Vec2Double position, Item item) : position(position), item(item) { }
LootBox LootBox::readFrom(InputStream& stream) {
    LootBox result;
    result.position = Vec2Double::readFrom(stream);
    Vec2Double::readFrom(stream); // size
    result.item = Item::readFrom(stream);
    return result;
}
string LootBox::toString() const {
    return position.toString() + " " + item.toString();
}
