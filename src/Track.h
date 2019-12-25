#pragma once

#include "model/UnitAction.hpp"
#include <utility>
#include <vector>

struct Track {
    std::vector<UnitAction> moves;
    size_t shift;

    explicit Track(size_t len) : moves(len), shift() {}
    explicit Track(std::vector<UnitAction> moves) : moves(std::move(moves)), shift() {}
    explicit Track(const std::initializer_list<UnitAction>& moves) : moves(moves.begin(), moves.end()), shift() {}

    void consume() {
        moves.push_back(moves.back());
        shift++;
    }

    size_t size() const { return moves.size() - shift; }

    UnitAction& get(size_t index) { return moves[index + shift]; }

    UnitAction& operator[](size_t index) { return get(index); }
    const UnitAction& operator[](size_t index) const { return const_cast<Track *>(this)->get(index); }

    UnitAction& first() { return get(0); }
};
