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

    bool isHealthPack() const { return data.index() == 0; }
    int health() const { return *std::get_if<int>(&data); }

    bool isWeapon() const { return data.index() == 1; }
    WeaponType weaponType() const { return *std::get_if<WeaponType>(&data); }

    bool isMine() const { return data.index() == 2; }

    Item();
    Item(std::variant<int, WeaponType, std::tuple<>> data) : data(data) {}
    static Item readFrom(InputStream& stream);
    std::string toString() const;
};

#endif
