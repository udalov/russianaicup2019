#ifndef _MODEL_BULLET_PARAMS_HPP_
#define _MODEL_BULLET_PARAMS_HPP_

#include "../Stream.hpp"
#include <string>

class BulletParams {
public:
    double speed;
    double size;
    int damage;
    BulletParams();
    BulletParams(double speed, double size, int damage);
    static BulletParams readFrom(InputStream& stream);
    std::string toString() const;
};

#endif
