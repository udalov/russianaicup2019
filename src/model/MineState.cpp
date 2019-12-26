#include "MineState.hpp"

using namespace std;

string mineStateToString(MineState state) {
    switch (state) {
        case PREPARING: return "PREPARING";
        case IDLE: return "IDLE";
        case TRIGGERED: return "TRIGGERED";
        case EXPLODED: return "EXPLODED";
        default: return "?";
    }
}
