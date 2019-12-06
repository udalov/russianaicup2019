#include "modelUtil.h"

#include <sstream>

using namespace std;

string toString(double x) {
    ostringstream out;
    out.precision(3);
    out << fixed << x;
    return out.str();
}
