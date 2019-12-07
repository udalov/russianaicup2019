#include "Versioned.hpp"

Versioned::Versioned() { }
Versioned::Versioned(std::unordered_map<int, UnitAction> inner) : inner(inner) { }
void Versioned::writeTo(OutputStream& stream) const {
    stream.write(43981);
    stream.write((int)(inner.size()));
    for (const auto& innerEntry : inner) {
        stream.write(innerEntry.first);
        innerEntry.second.writeTo(stream);
    }
}
