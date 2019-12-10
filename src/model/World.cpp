#include "World.h"

using namespace std;

World::World() { }
World::World(const vector<Unit>& units, const vector<Bullet>& bullets, const vector<Mine>& mines, const vector<LootBox>& lootBoxes) : units(units), bullets(bullets), mines(mines), lootBoxes(lootBoxes) { }
World World::readFrom(InputStream& stream) {
    World result;
    result.units = vector<Unit>(stream.readInt());
    for (size_t i = 0; i < result.units.size(); i++) {
        result.units[i] = Unit::readFrom(stream);
    }
    result.bullets = vector<Bullet>(stream.readInt());
    for (size_t i = 0; i < result.bullets.size(); i++) {
        result.bullets[i] = Bullet::readFrom(stream);
    }
    result.mines = vector<Mine>(stream.readInt());
    for (size_t i = 0; i < result.mines.size(); i++) {
        result.mines[i] = Mine::readFrom(stream);
    }
    result.lootBoxes = vector<LootBox>(stream.readInt());
    for (size_t i = 0; i < result.lootBoxes.size(); i++) {
        result.lootBoxes[i] = LootBox::readFrom(stream);
    }
    return result;
}
