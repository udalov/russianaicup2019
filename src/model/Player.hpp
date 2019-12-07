#ifndef _MODEL_PLAYER_HPP_
#define _MODEL_PLAYER_HPP_

#include "../Stream.hpp"
#include <string>

class Player {
public:
    int id;
    int score;
    Player();
    Player(int id, int score);
    static Player readFrom(InputStream& stream);
    bool operator ==(const Player& other) const;
    std::string toString() const;
};
namespace std {
    template<>
    struct hash<Player> {
        size_t operator ()(const Player& value) const;
    };
}

#endif
