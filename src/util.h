#pragma once

#include "model/LootBox.hpp"
#include "model/Unit.hpp"
#include <functional>
#include <vector>

template<typename T> const T *minBy(const std::vector<T>& v, const std::function<double(const T&)>& f) {
    auto best = 1e100;
    const T *ans = nullptr;
    for (auto& x : v) {
        auto cur = f(x);
        if (cur < best) {
            best = cur;
            ans = &x;
        }
    }
    return ans;
}

template<typename T> void fastRemove(std::vector<T>& v, T& element) {
    std::swap(element, v.back());
    v.pop_back();
}

template<typename T> void fastRemove(std::vector<T>& v, size_t index) {
    fastRemove(v, v[index]);
}

const Unit *minBy(const std::vector<Unit>& units, const std::function<double(const Unit&)>& f) {
    return minBy<Unit>(units, f);
}

const LootBox *minBy(const std::vector<LootBox>& lootBoxes, const std::function<double(const LootBox&)>& f) {
    return minBy<LootBox>(lootBoxes, f);
}
