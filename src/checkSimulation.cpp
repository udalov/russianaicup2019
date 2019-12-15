#include "checkSimulation.h"

#include "Const.h"
#include "Simulation.h"
#include "util.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

unique_ptr<vector<UnitAction>> getRandomMoveSequence(int microticks, const Vec& aim) {
    auto ans = make_unique<vector<UnitAction>>();
    UnitAction moveRight;
    moveRight.velocity = 100.0;
    UnitAction jump;
    jump.jump = true;
    UnitAction jumpAndMoveRight;
    jumpAndMoveRight.velocity = 1000.0;
    jumpAndMoveRight.jump = true;
    UnitAction left;
    left.velocity = -100.0;

    auto t = 100 / microticks;
    size_t cp1 = 13 * t;
    size_t cp2 = 25 * t;
    size_t cp3 = 186 * t;
    size_t cp4 = 1000 * t;

    for (size_t i = 0; i < cp1; i++) ans->push_back(moveRight);
    for (size_t i = cp1; i < cp2; i++) ans->push_back(jump);
    for (size_t i = cp2; i < cp3; i++) ans->push_back(jumpAndMoveRight);
    for (size_t i = cp3; i < cp4; i++) ans->push_back(left);

    for (auto& action : *ans) {
        action.shoot = true;
        action.aim = aim;
    }
    return ans;
}

void drawRect(Vec corner, Vec size, const ColorFloat& color, Debug& debug) {
    debug.draw(CustomData::Rect(corner, size, color));
}

void drawUnit(const Unit& unit, const ColorFloat& color, Debug& debug) {
    drawRect(unit.position - Vec(unitSize.x / 2, 0), unitSize, color, debug);
}

void drawBullet(const Bullet& bullet, const ColorFloat& color, Debug& debug) {
    auto size =
        bullet.weaponType == WeaponType::ROCKET_LAUNCHER ? rocketLauncherParams.bullet.size :
        bullet.weaponType == WeaponType::ASSAULT_RIFLE ? assaultRifleParams.bullet.size :
        bullet.weaponType == WeaponType::PISTOL ? pistolParams.bullet.size : 100.0;
    drawRect(bullet.position - Vec(size / 2, size / 2), Vec(size, size), color, debug);
}

string surroundingToString(const Vec& v, const Level& level) {
    string ans;
    for (int dy = 2; dy >= -2; dy--) {
        for (int dx = -2; dx <= 2; dx++) {
            auto z = v + Vec(dx, dy);
            auto tile = 0 <= z.x && z.x < level.tiles.size() && 0 <= z.y && z.y < level.tiles[0].size()
                ? level(z) : Tile::WALL;
            ans += tileToChar(tile);
        }
        ans += "\n";
    }
    return ans;
}

int simulationErrors = 0;
pair<int, World> simulationPrediction;

UnitAction checkSimulation(int myId, const Game& game, Debug& debug) {
    auto& me = findUnit(game.world, myId);
    auto nearestEnemy = minBy<Unit>(game.world.units, [&me](const auto& other) {
        return other.playerId != me.playerId ? me.position.sqrDist(other.position) : 1e100;
    });

    auto microticks = updatesPerTick;
    static auto moves = getRandomMoveSequence(microticks, /* nearestEnemy->position - me.position */ Vec(0, -1));
    UnitAction ans;
    if (moves->empty()) return ans;

    auto tick = game.currentTick;
    if (tick != 0) {
        auto actual = renderWorld(myId, game.world);
        auto expected = renderWorld(simulationPrediction.first, simulationPrediction.second);
        cout << tick << " " << actual << endl;
        if (expected != actual) {
            cout << "ERROR! predicted:" << endl << tick << " " << expected << endl;
            if (expected.substr(0, expected.find('\n')) != actual.substr(0, actual.find('\n'))) {
                cout << surroundingToString(findUnit(game.world, myId).position, game.level);
            } else {
                cout << endl;
            }
            simulationErrors++;
        }
    }
    if (tick == game.properties.maxTickCount - 1) {
        cout << "TOTAL ERRORS: " << simulationErrors << endl;
    }

    auto world = game.world;
    debug.log(string("cur: ") + findUnit(world, myId).toString());

    constexpr size_t ticks = 300;

    simulate(myId, game.level, world, *moves, microticks, ticks,
             [myId, ticks, currentTick=game.currentTick, &debug](size_t tick, const World& world) {
        auto& me = findUnit(world, myId);
        if (tick > 10 && (tick + currentTick) % 10 == 0) {
            auto coeff = 1.0f - tick * 1.0f / ticks;
            drawUnit(me, ColorFloat(0.2f * coeff, 0.2f * coeff, 1.0f * coeff, 1.0), debug);
        }
        if (tick > 0 && (tick + currentTick) % 5 == 0) {
            auto coeff = 1.0f - tick * 1.0f / ticks;
            for (auto& bullet : world.bullets) {
                drawBullet(bullet, ColorFloat(0.7f * coeff, 0.2f * coeff, 0.1f * coeff, 0.8), debug);
            }
        }
        if (tick == 0) {
            debug.log(string("next: ") + me.toString());
            simulationPrediction = make_pair(myId, world);
        }
    });
    ans = *moves->begin();
    moves->erase(moves->begin());
    return ans;
}
