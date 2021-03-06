#include "Level.hpp"

#include <algorithm>
#include <stdexcept>

using namespace std;

Level::Level() { }
Level::Level(vector<vector<Tile>> tiles) : tiles(tiles) { }
Level Level::readFrom(InputStream& stream) {
    Level result;
    result.tiles = vector<vector<Tile>>(stream.readInt());
    for (size_t i = 0; i < result.tiles.size(); i++) {
        result.tiles[i] = vector<Tile>(stream.readInt());
        for (size_t j = 0; j < result.tiles[i].size(); j++) {
            switch (stream.readInt()) {
                case 0: result.tiles[i][j] = Tile::EMPTY; break;
                case 1: result.tiles[i][j] = Tile::WALL; break;
                case 2: result.tiles[i][j] = Tile::PLATFORM; break;
                case 3: result.tiles[i][j] = Tile::LADDER; break;
                case 4: result.tiles[i][j] = Tile::JUMP_PAD; break;
                default: throw runtime_error("Unexpected discriminant value");
            }
        }
    }
    return result;
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
