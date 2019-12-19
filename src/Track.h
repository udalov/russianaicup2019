#pragma once

#include "model/UnitAction.hpp"
#include <vector>

struct Track {
    std::vector<UnitAction> moves;
    size_t shift;

    explicit Track(size_t len) : moves(len), shift() {}
    explicit Track(const std::vector<UnitAction>& moves) : moves(moves), shift() {}

    void consume() {
        moves.push_back(moves.back());
        shift++;
    }

    size_t size() const { return moves.size(); }

    UnitAction& get(size_t index) { return moves[index + shift]; }
    void set(size_t index, const UnitAction& value) { get(index) = value; }

    UnitAction& operator[](size_t index) { return get(index); }
    const UnitAction& operator[](size_t index) const { return const_cast<Track *>(this)->get(index); }

    UnitAction& first() { return get(0); }
};
