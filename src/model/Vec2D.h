#pragma once

#include <cmath>
#include <string>
#include "../Stream.hpp"
#include "modelUtil.h"

constexpr inline double sqr(double x) { return x * x; }

template<typename T> struct Vec2D {
    T x, y;

    constexpr Vec2D() : x(), y() {}
    constexpr Vec2D(T x, T y) : x(x), y(y) {}

    constexpr Vec2D operator+(const Vec2D& other) const { return Vec2D(x + other.x, y + other.y); }
    constexpr Vec2D operator-(const Vec2D& other) const { return Vec2D(x - other.x, y - other.y); }
    constexpr Vec2D operator*(T coeff) const { return Vec2D(x * coeff, y * coeff); }
    constexpr Vec2D operator/(T coeff) const { return Vec2D(x / coeff, y / coeff); }

    constexpr void operator+=(const Vec2D& other) { x += other.x; y += other.y; }
    constexpr void operator-=(const Vec2D& other) { x -= other.x; y -= other.y; }
    constexpr void operator*=(T coeff) { x *= coeff; y *= coeff; }
    constexpr void operator/=(T coeff) { x /= coeff; y /= coeff; }

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
    void writeFloatTo(OutputStream& stream) const {
        stream.write((float) x);
        stream.write((float) y);
    }
    std::string toString() const {
        return std::string("(") + ::toString(x) + ", " + ::toString(y) + ")";
    }
};

typedef Vec2D<double> Vec2Double;
typedef Vec2Double Vec;
