#include "Game.hpp"

Game::Game() { }
Game::Game(int currentTick, Properties properties, Level level, std::vector<Player> players, std::vector<Unit> units, std::vector<Bullet> bullets, std::vector<Mine> mines, std::vector<LootBox> lootBoxes) : currentTick(currentTick), properties(properties), level(level), players(players), units(units), bullets(bullets), mines(mines), lootBoxes(lootBoxes) { }
Game Game::readFrom(InputStream& stream) {
    Game result;
    result.currentTick = stream.readInt();
    result.properties = Properties::readFrom(stream);
    result.level = Level::readFrom(stream);
    result.players = std::vector<Player>(stream.readInt());
    for (size_t i = 0; i < result.players.size(); i++) {
        result.players[i] = Player::readFrom(stream);
    }
    result.units = std::vector<Unit>(stream.readInt());
    for (size_t i = 0; i < result.units.size(); i++) {
        result.units[i] = Unit::readFrom(stream);
    }
    result.bullets = std::vector<Bullet>(stream.readInt());
    for (size_t i = 0; i < result.bullets.size(); i++) {
        result.bullets[i] = Bullet::readFrom(stream);
    }
    result.mines = std::vector<Mine>(stream.readInt());
    for (size_t i = 0; i < result.mines.size(); i++) {
        result.mines[i] = Mine::readFrom(stream);
    }
    result.lootBoxes = std::vector<LootBox>(stream.readInt());
    for (size_t i = 0; i < result.lootBoxes.size(); i++) {
        result.lootBoxes[i] = LootBox::readFrom(stream);
    }
    return result;
}
