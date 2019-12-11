#ifndef _MY_STRATEGY_HPP_
#define _MY_STRATEGY_HPP_

#include "Debug.hpp"
#include "model/CustomData.hpp"
#include "model/Game.hpp"
#include "model/Unit.hpp"
#include "model/UnitAction.hpp"
#include <string>
#include <unordered_map>

class MyStrategy {
public:
    MyStrategy();
    MyStrategy(std::unordered_map<std::string, std::string>&& params);
    UnitAction getAction(const Unit& myUnit, const Game& game, Debug& debug);
private:
    std::unordered_map<std::string, std::string> params;
};

#endif
