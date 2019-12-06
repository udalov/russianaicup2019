#include "Tile.hpp"

char tileToChar(Tile tile) {
    switch (tile) {
        case Tile::EMPTY: return '.';
        case Tile::WALL: return '#';
        case Tile::PLATFORM: return '^';
        case Tile::LADDER: return 'H';
        case Tile::JUMP_PAD: return 'T';
        default: return '?';
    }
}
