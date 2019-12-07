#ifndef _MODEL_CUSTOM_DATA_HPP_
#define _MODEL_CUSTOM_DATA_HPP_

#include "../Stream.hpp"
#include <memory>
#include <string>
#include <vector>
#include "ColorFloat.hpp"
#include "ColoredVertex.hpp"
#include "TextAlignment.hpp"

class CustomData {
public:
    class Log;
    class Rect;
    class Line;
    class Polygon;
    class PlacedText;

    virtual void writeTo(OutputStream& stream) const = 0;
    virtual std::string toString() const = 0;
};

class CustomData::Log : public CustomData {
public:
    static const int TAG = 0;
public:
    std::string text;
    Log();
    Log(std::string text);
    void writeTo(OutputStream& stream) const;
    std::string toString() const override;
};

class CustomData::Rect : public CustomData {
public:
    static const int TAG = 1;
public:
    Vec2Double pos;
    Vec2Double size;
    ColorFloat color;
    Rect();
    Rect(Vec2Double pos, Vec2Double size, ColorFloat color);
    void writeTo(OutputStream& stream) const;
    std::string toString() const override;
};

class CustomData::Line : public CustomData {
public:
    static const int TAG = 2;
public:
    Vec2Double p1;
    Vec2Double p2;
    float width;
    ColorFloat color;
    Line();
    Line(Vec2Double p1, Vec2Double p2, float width, ColorFloat color);
    void writeTo(OutputStream& stream) const;
    std::string toString() const override;
};

class CustomData::Polygon : public CustomData {
public:
    static const int TAG = 3;
public:
    std::vector<ColoredVertex> vertices;
    Polygon();
    Polygon(std::vector<ColoredVertex> vertices);
    void writeTo(OutputStream& stream) const;
    std::string toString() const override;
};

class CustomData::PlacedText : public CustomData {
public:
    static const int TAG = 4;
public:
    std::string text;
    Vec2Double pos;
    TextAlignment alignment;
    float size;
    ColorFloat color;
    PlacedText();
    PlacedText(std::string text, Vec2Double pos, TextAlignment alignment, float size, ColorFloat color);
    void writeTo(OutputStream& stream) const;
    std::string toString() const override;
};

#endif
