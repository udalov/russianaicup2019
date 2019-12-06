#include "Level.hpp"

using namespace std;

Level::Level() { }
Level::Level(std::vector<std::vector<Tile>> tiles) : tiles(tiles) { }
Level Level::readFrom(InputStream& stream) {
    Level result;
    result.tiles = std::vector<std::vector<Tile>>(stream.readInt());
    for (size_t i = 0; i < result.tiles.size(); i++) {
        result.tiles[i] = std::vector<Tile>(stream.readInt());
        for (size_t j = 0; j < result.tiles[i].size(); j++) {
            switch (stream.readInt()) {
            case 0:
                result.tiles[i][j] = Tile::EMPTY;
                break;
            case 1:
                result.tiles[i][j] = Tile::WALL;
                break;
            case 2:
                result.tiles[i][j] = Tile::PLATFORM;
                break;
            case 3:
                result.tiles[i][j] = Tile::LADDER;
                break;
            case 4:
                result.tiles[i][j] = Tile::JUMP_PAD;
                break;
            default:
                throw std::runtime_error("Unexpected discriminant value");
            }
        }
    }
    return result;
}
void Level::writeTo(OutputStream& stream) const {
    stream.write((int)(tiles.size()));
    for (const std::vector<Tile>& tilesElement : tiles) {
        stream.write((int)(tilesElement.size()));
        for (const Tile& tilesElementElement : tilesElement) {
            stream.write((int)(tilesElementElement));
        }
    }
}

string Level::toString() const {
    auto lines = toStrings();
    reverse(lines.begin(), lines.end());
    string ans = "";
    for (auto& line : lines) ans += line, ans += "\n";
    return ans;
}

vector<string> Level::toStrings() const {
    vector<string> ans;
    for (size_t j = 0; j < tiles[0].size(); j++) {
        string s = "";
        for (size_t i = 0; i < tiles.size(); i++) {
            s += tileToChar(tiles[i][j]);
        }
        ans.push_back(s);
    }
    return ans;
}
