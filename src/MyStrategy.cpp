#include "MyStrategy.hpp"

#include <iostream>
#include <numeric>
#include <vector>
#include "Const.h"
#include "util.h"
#include "Simulation.h"
#include "checkSimulation.h"

using namespace std;

MyStrategy::MyStrategy() : params() {}
MyStrategy::MyStrategy(unordered_map<string, string> params) : params(move(params)) {}

vector<Track> generateTracks(size_t len, const Unit& me, const Unit *nearestEnemy) {
    vector<Track> ans;
    Track t(len);
    /*
    if (nearestEnemy) {
        for (size_t i = 0; i < len; i++) {
            t[i].aim = nearestEnemy->position - me.position;
        }
    }
    */
    ans.push_back(t);
    for (size_t i = 0; i < len; i++) t[i].velocity = 10.0; ans.push_back(t);
    for (size_t i = 0; i < len; i++) t[i].velocity = -10.0; ans.push_back(t);
    for (size_t i = 0; i < len; i++) t[i].velocity = 0.0, t[i].jump = true; ans.push_back(t);
    for (size_t i = 0; i < len; i++) t[i].velocity = 10.0, t[i].jump = true; ans.push_back(t);
    for (size_t i = 0; i < len; i++) t[i].velocity = -10.0, t[i].jump = true; ans.push_back(t);
    for (size_t i = 0; i < len; i++) t[i].velocity = 0.0, t[i].jump = false, t[i].jumpDown = true; ans.push_back(t);
    for (size_t i = 0; i < len; i++) t[i].velocity = 10.0, t[i].jump = false, t[i].jumpDown = true; ans.push_back(t);
    for (size_t i = 0; i < len; i++) t[i].velocity = -10.0, t[i].jump = false, t[i].jumpDown = true; ans.push_back(t);

    for (size_t times = 0; times < 100; times++) {
        size_t t1 = rand() % ans.size();
        size_t t2;
        do {
            t2 = rand() % ans.size();
        } while (t1 == t2);
        size_t mid = rand() % len;
        copy(ans[t1].begin(), ans[t1].begin() + mid, t.begin());
        copy(ans[t2].begin() + mid, ans[t2].end(), t.begin() + mid);
        ans.push_back(t);
    }

    /*
    size_t shootTick = -1;
    auto& weapon = me.weapon;
    if (weapon.has_value()) {
        auto timer = weapon->fireTimer;
        for (size_t i = 0; i < len; i++) {
            if (timer <= 0.0) {
                shootTick = i;
                break;
            }
            timer -= 1.0 / ticksPerSecond;
        }
        if (shootTick >= 0) {
            size_t sz = ans.size();
            for (size_t j = 0; j < sz; j++) {
                t = ans[j];
                t[shootTick].shoot = true;
                ans.push_back(t);
            }
        }
    }
    */
    return ans;
}

double estimate(const World& world, int myId, int magazineAtStart, const Unit *nearestEnemy, const LootBox *nearestWeapon) {
    auto& me = findUnit(world, myId);
    auto score = 0.0;
    for (auto& unit : world.units) {
        // TODO: intelligent suicide
        if (unit.playerId == me.playerId) score += 2 * unit.health;
        else score -= unit.health;
    }
    score *= 10000;

    // Hack
    // if (!me.weapon || me.weapon->magazine == magazineAtStart) score -= 1000.0;

    if (!me.weapon && nearestWeapon) {
        score += -100.0 - me.position.distance(nearestWeapon->position);
    } else if (nearestEnemy) {
        score += -10.0 - abs(me.position.distance(nearestEnemy->position) - 10.0);
    }
    return score;
}

int getMyHealth(const World& world, int playerId) {
    int ans = 0;
    for (auto& unit : world.units) if (unit.playerId == playerId) ans += unit.health;
    return ans;
}

bool needToShoot(const Unit& me, const Game& game, Track track, const Vec& aim) {
    if (!me.weapon) return false;
    if (me.weapon->fireTimer > 0.0) return false;
    if (me.weapon->type != WeaponType::ROCKET_LAUNCHER) return true;

    auto size = min(track.size(), 30ul);

    auto world1 = game.world;
    simulate(
        me.id, game.level, world1, track, 4, size,
        [&](size_t tick, const World& world) { }
    );
    int expectedHealth = getMyHealth(world1, me.playerId);

    track[0].shoot = true;
    track[0].aim = aim;

    auto world2 = game.world;
    simulate(
        me.id, game.level, world2, track, 4, size,
        [&](size_t tick, const World& world) { }
    );
    int actualHealth = getMyHealth(world2, me.playerId);
    return actualHealth >= expectedHealth;
}

struct AllyData {
    vector<Track> savedTracks;

    AllyData() : savedTracks() {}
};

AllyData datas[2];
int minAllyId;
bool visualize = false;
bool simulation = false;
bool isInitialized = false;

UnitAction MyStrategy::getAction(const Unit& myUnit, const Game& game, Debug& debug) {
    constexpr size_t trackLen = 80;

    auto tick = game.currentTick;
    auto myId = myUnit.id;
    auto& me = findUnit(game.world, myId);
    if (!isInitialized) {
        isInitialized = true;
        srand(42);
        if (params.find("--dump-constants") != params.end()) {
            dumpConstants(game.properties);
            terminate();
        }
        checkConstants(game.properties);
        simulation = params.find("--simulate") != params.end();
        visualize = params.find("--vis") != params.end();

        minAllyId = numeric_limits<int>::max();
        for (auto& unit : game.world.units) {
            if (unit.playerId == me.playerId) {
                minAllyId = min(minAllyId, unit.id);
            }
        }
    }

    if (simulation) return checkSimulation(myId, game, debug);

    auto& data = datas[me.id != minAllyId];

    auto nearestEnemy = minBy<Unit>(game.world.units, [&me](const auto& other) {
        return other.playerId != me.playerId ? me.position.sqrDist(other.position) : 1e100;
    });
    auto nearestWeapon = minBy<LootBox>(game.world.lootBoxes, [&me](const auto& box) {
        return box.item.isWeapon() ? me.position.sqrDist(box.position) : 1e100;
    });
    auto magazineAtStart = me.weapon ? me.weapon->magazine : -1;

    constexpr size_t tracksToSave = 10;
    constexpr size_t microticks = 4;

    constexpr size_t estimateCutoff = 20;
    constexpr size_t estimateEveryNth = 10;

    auto tracks = generateTracks(trackLen, me, nearestEnemy);
    // cout << "### " << tick << " | " << tracks.size() << " tracks | " << me.toString() << endl;
    auto& savedTracks = data.savedTracks;
    tracks.insert(tracks.end(), savedTracks.begin(), savedTracks.end());
    savedTracks.clear();

    auto scores = vector<double>(tracks.size());
    for (size_t i = 0; i < tracks.size(); i++) {
        auto& track = tracks[i];
        auto world = game.world;
        auto score = 0.0;
        simulate(
            myId, game.level, world, track, microticks, min(track.size(), trackLen),
            [&](size_t tick, const World& world) {
                if (tick >= estimateCutoff && tick % estimateEveryNth == 0) {
                    auto coeff = (trackLen - tick) / (double)(trackLen - estimateCutoff) * 0.5 + 0.5;
                    score += coeff * estimate(world, myId, magazineAtStart, nearestEnemy, nearestWeapon);
                }
            }
        );
        scores[i] = score;
    }

    auto indices = vector<size_t>(tracks.size());
    iota(indices.begin(), indices.end(), 0);

    sort(indices.begin(), indices.end(), [&scores](auto& i1, auto& i2) { return scores[i1] > scores[i2]; });

    for (size_t i = 0; i < tracksToSave && i < tracks.size(); i++) {
        // TODO: optimize
        auto track = tracks[indices[i]];
        track.erase(track.begin());
        track.push_back(track.back());
        savedTracks.emplace_back(track);
    }

    if (visualize) {
        debug.log(renderWorld(myId, game.world));
        auto bestTrack = tracks[indices.front()];
        auto w = game.world;
        simulate(
            myId, game.level, w, bestTrack, microticks, 4,
            [&](size_t tick, const World& world) {
                debug.log(string("+") + to_string(tick) + " " + bestTrack[tick].toString() + " -> " + renderWorld(myId, world));
            }
        );
    }

    auto aim = nearestEnemy->position - me.position;
    auto shoot = needToShoot(me, game, tracks[indices.front()], aim);
    auto ans = tracks.empty() ? UnitAction() : tracks[indices.front()].front();
    ans.aim = aim;
    ans.shoot = shoot;
    // cout << tick << " " << ans.toString() << endl;
    // cout << tick << " " << me.toString() << endl;
    return ans;
}
