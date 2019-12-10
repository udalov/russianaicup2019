#include "UnitAction.hpp"

#include "modelUtil.h"

using namespace std;

UnitAction::UnitAction() : velocity(), jump(), jumpDown(), aim(), shoot(), swapWeapon(), plantMine() {}
UnitAction::UnitAction(double velocity, bool jump, bool jumpDown, Vec2Double aim, bool shoot, bool reload, bool swapWeapon, bool plantMine) : velocity(velocity), jump(jump), jumpDown(jumpDown), aim(aim), shoot(shoot), reload(reload), swapWeapon(swapWeapon), plantMine(plantMine) { }
void UnitAction::writeTo(OutputStream& stream) const {
    stream.write(velocity);
    stream.write(jump);
    stream.write(jumpDown);
    aim.writeTo(stream);
    stream.write(shoot);
    stream.write(reload);
    stream.write(swapWeapon);
    stream.write(plantMine);
}
string UnitAction::toString() const {
    string ans = ::toString(velocity);
    string mod;
    if (jump) mod += "^";
    if (jumpDown) mod += "v";
    if (shoot) mod += "X";
    if (reload) mod += "R";
    if (!mod.empty()) ans += " " + mod;
    return "<" + ans + ">";
}
