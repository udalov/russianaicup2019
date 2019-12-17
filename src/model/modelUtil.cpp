#include "modelUtil.h"

#include <sstream>

using namespace std;

string toString(double x, int precision) {
    ostringstream out;
    out.precision(precision);
    out << fixed << x;
    return out.str();
}

string toString(double x) {
    return toString(x, 8);
}
