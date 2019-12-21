#pragma once

#include "model/LootBox.hpp"
#include "model/Unit.hpp"
#include "model/World.h"
#include <functional>
#include <string>
#include <vector>

inline int randomInt() { 
    static unsigned int randomSeed = 42;
    randomSeed = 214013 * randomSeed + 2531011; 
    return (randomSeed >> 16) & 0x7FFF; 
} 

inline Unit& findUnit(World& world, int id) {
    return *find_if(world.units.begin(), world.units.end(), [id](const auto& unit) { return unit.id == id; });
}

inline const Unit& findUnit(const World& world, int id) {
    return static_cast<const Unit&>(findUnit(const_cast<World&>(world), id));
}

inline std::string renderWorld(int myId, const World& world) {
    std::string ans = findUnit(world, myId).toString();
    for (auto& bullet : world.bullets) {
        ans += "\n  " + bullet.toString();
    }
    return ans;
}

template<typename T> T *minBy(std::vector<T>& v, const std::function<double(const T&)>& f) {
    auto best = 1e100;
    T *ans = nullptr;
    for (auto& x : v) {
        auto cur = f(x);
        if (cur < best) {
            best = cur;
            ans = &x;
        }
    }
    return ans;
}

template<typename T> const T *minBy(const std::vector<T>& v, const std::function<double(const T&)>& f) {
    return static_cast<const T *>(minBy(const_cast<std::vector<T>&>(v), f));
}

template<typename T> void fastRemove(std::vector<T>& v, T& element) {
    std::swap(element, v.back());
    v.pop_back();
}

template<typename T> void fastRemove(std::vector<T>& v, size_t index) {
    fastRemove(v, v[index]);
}
