#ifndef _MODEL_ITEM_HPP_
#define _MODEL_ITEM_HPP_

#include "../Stream.hpp"
#include <string>
#include <tuple>
#include <variant>
#include "WeaponType.hpp"

class Item {
public:
    // Health amount, weapon type, or mine
    std::variant<int, WeaponType, std::tuple<>> data;

    bool isWeapon() const;
    WeaponType weaponType() const;

    bool isHealthPack() const;
    int health() const;

    Item();
    Item(std::variant<int, WeaponType, std::tuple<>> data) : data(data) {}
    static Item readFrom(InputStream& stream);
    std::string toString() const;
};

#endif
