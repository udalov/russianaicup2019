#ifndef _MODEL_EXPLOSION_PARAMS_HPP_
#define _MODEL_EXPLOSION_PARAMS_HPP_

#include "../Stream.hpp"
#include <string>

class ExplosionParams {
public:
    double radius;
    int damage;
    constexpr ExplosionParams() : radius(), damage() {}
    constexpr ExplosionParams(double radius, int damage) : radius(radius), damage(damage) { }
    static ExplosionParams readFrom(InputStream& stream);
    bool operator==(const ExplosionParams& other) const;
    std::string toString() const;
};

#endif
