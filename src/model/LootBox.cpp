#include "LootBox.hpp"

using namespace std;

LootBox::LootBox() { }
LootBox::LootBox(Vec2Double position, Vec2Double size, std::shared_ptr<Item> item) : position(position), size(size), item(item) { }
LootBox LootBox::readFrom(InputStream& stream) {
    LootBox result;
    result.position = Vec2Double::readFrom(stream);
    result.size = Vec2Double::readFrom(stream);
    result.item = Item::readFrom(stream);
    return result;
}
std::string LootBox::toString() const {
    return position.toString() + " " + item->toString();
}
