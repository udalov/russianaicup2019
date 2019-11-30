#pragma once

#include <cmath>
#include <sstream>
#include <string>
#include "../Stream.hpp"

constexpr inline double sqr(double x) { return x * x; }

template<typename T> struct Vec2D {
    T x, y;

    constexpr Vec2D() : x(), y() {}
    constexpr Vec2D(T x, T y) : x(x), y(y) {}

    constexpr Vec2D operator+(const Vec2D& other) const { return Vec2D(x + other.x, y + other.y); }
    constexpr Vec2D operator-(const Vec2D& other) const { return Vec2D(x - other.x, y - other.y); }
    constexpr Vec2D operator*(T coeff) const { return Vec2D(x * coeff, y * coeff); }

    constexpr void operator+=(const Vec2D& other) { x += other.x; y += other.y; }
    constexpr void operator-=(const Vec2D& other) { x -= other.x; y -= other.y; }
    constexpr void operator*=(T coeff) { x *= coeff; y *= coeff; }

    Vec2D normalize() const { return *this * (1 / len()); }

    constexpr T dot(const Vec2D& v) const { return x*v.x + y*v.y; }

    constexpr T sqrLen() const { return x*x + y*y; }
    T len() const { return sqrt(sqrLen()); }

    constexpr T sqrDist(const Vec2D& other) const { return sqr(x-other.x) + sqr(y-other.y); }
    T distance(const Vec2D& other) const { return sqrt(sqrDist(other)); }

    static Vec2D<T> readFrom(InputStream& stream) {
        Vec2D<T> result;
        result.x = stream.readDouble();
        result.y = stream.readDouble();
        return result;
    }
    void writeTo(OutputStream& stream) const {
        stream.write(x);
        stream.write(y);
    }
    std::string toString() const {
        std::ostringstream out;
        out.precision(3);
        out << std::fixed << "(" << x << ", " << y << ")";
        return out.str();
    }
};

typedef Vec2D<double> Vec2Double;
typedef Vec2D<float> Vec2Float;

typedef Vec2Double Vec;
