#include "UnitAction.hpp"

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
std::string UnitAction::toString() const {
    return std::string("UnitAction") + "(" +
        std::to_string(velocity) +
        (jump ? "true" : "false") + 
        (jumpDown ? "true" : "false") + 
        aim.toString() +
        (shoot ? "true" : "false") + 
        (reload ? "true" : "false") + 
        (swapWeapon ? "true" : "false") + 
        (plantMine ? "true" : "false") + 
        ")";
}
