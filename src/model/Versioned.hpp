#ifndef _MODEL_VERSIONED_HPP_
#define _MODEL_VERSIONED_HPP_

#include "../Stream.hpp"
#include <string>
#include <unordered_map>
#include "UnitAction.hpp"
#include "Vec2D.h"

class Versioned {
public:
    std::unordered_map<int, UnitAction> inner;
    Versioned();
    Versioned(std::unordered_map<int, UnitAction> inner);
    static Versioned readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;
};

#endif
