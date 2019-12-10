#include "Game.hpp"

using namespace std;

Game::Game() { }
Game::Game(int currentTick, Properties properties, Level level, vector<Player> players, World world) : currentTick(currentTick), properties(properties), level(level), players(players), world(world) { }
Game Game::readFrom(InputStream& stream) {
    Game result;
    result.currentTick = stream.readInt();
    result.properties = Properties::readFrom(stream);
    result.level = Level::readFrom(stream);
    result.players = vector<Player>(stream.readInt());
    for (size_t i = 0; i < result.players.size(); i++) {
        result.players[i] = Player::readFrom(stream);
    }
    result.world = World::readFrom(stream);
    return result;
}
