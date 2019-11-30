#include "CustomData.hpp"


CustomData::Log::Log() { }
CustomData::Log::Log(std::string text) : text(text) { }
void CustomData::Log::writeTo(OutputStream& stream) const {
    stream.write(TAG);
    stream.write(text);
}
std::string CustomData::Log::toString() const {
    return std::string("CustomData::Log") + "(" +
        text + 
        ")";
}

CustomData::Rect::Rect() { }
CustomData::Rect::Rect(Vec2Double pos, Vec2Double size, ColorFloat color) : pos(pos), size(size), color(color) { }
void CustomData::Rect::writeTo(OutputStream& stream) const {
    stream.write(TAG);
    pos.writeFloatTo(stream);
    size.writeFloatTo(stream);
    color.writeTo(stream);
}
std::string CustomData::Rect::toString() const {
    return std::string("CustomData::Rect") + "(" +
        pos.toString() +
        size.toString() +
        color.toString() +
        ")";
}

CustomData::Line::Line() { }
CustomData::Line::Line(Vec2Double p1, Vec2Double p2, float width, ColorFloat color) : p1(p1), p2(p2), width(width), color(color) { }
void CustomData::Line::writeTo(OutputStream& stream) const {
    stream.write(TAG);
    p1.writeFloatTo(stream);
    p2.writeFloatTo(stream);
    stream.write(width);
    color.writeTo(stream);
}
std::string CustomData::Line::toString() const {
    return std::string("CustomData::Line") + "(" +
        p1.toString() +
        p2.toString() +
        std::to_string(width) +
        color.toString() +
        ")";
}

CustomData::Polygon::Polygon() { }
CustomData::Polygon::Polygon(std::vector<ColoredVertex> vertices) : vertices(vertices) { }
void CustomData::Polygon::writeTo(OutputStream& stream) const {
    stream.write(TAG);
    stream.write((int)(vertices.size()));
    for (const ColoredVertex& verticesElement : vertices) {
        verticesElement.writeTo(stream);
    }
}
std::string CustomData::Polygon::toString() const {
    return std::string("CustomData::Polygon") + "(" +
        "TODO" + 
        ")";
}

CustomData::PlacedText::PlacedText() { }
CustomData::PlacedText::PlacedText(std::string text, Vec2Double pos, TextAlignment alignment, float size, ColorFloat color) : text(text), pos(pos), alignment(alignment), size(size), color(color) { }
void CustomData::PlacedText::writeTo(OutputStream& stream) const {
    stream.write(TAG);
    stream.write(text);
    pos.writeFloatTo(stream);
    stream.write((int)(alignment));
    stream.write(size);
    color.writeTo(stream);
}
std::string CustomData::PlacedText::toString() const {
    return std::string("CustomData::PlacedText") + "(" +
        text + 
        pos.toString() +
        "TODO" + 
        std::to_string(size) +
        color.toString() +
        ")";
}
