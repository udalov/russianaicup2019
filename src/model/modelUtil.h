#pragma once

#include <sstream>
#include <string>

inline std::string toString(double x) {
    std::ostringstream out;
    out.precision(9);
    out << std::fixed << x;
    return out.str();
}
