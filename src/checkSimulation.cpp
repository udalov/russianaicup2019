#include "checkSimulation.h"

#include "Const.h"
#include "Simulation.h"
#include "util.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

unique_ptr<Track> getRandomMoveSequence(const Vec &aim) {
    size_t cp1 = 13;
    size_t cp2 = 25;
    size_t cp3 = 186;
    size_t cp4 = 1000;

    auto ans = make_unique<Track>(cp4);
    UnitAction defaultMove;
    defaultMove.shoot = true;
    defaultMove.aim = aim;

    UnitAction moveRight = defaultMove;
    moveRight.velocity = 10.0;
    UnitAction jump = defaultMove;
    jump.jump = true;
    UnitAction jumpAndMoveRight = moveRight;
    jumpAndMoveRight.jump = true;
    UnitAction left = defaultMove;
    left.velocity = -10.0;

    for (size_t i = 0; i < cp1; i++) (*ans)[i] = moveRight;
    for (size_t i = cp1; i < cp2; i++) (*ans)[i] = jump;
    for (size_t i = cp2; i < cp3; i++) (*ans)[i] = jumpAndMoveRight;
    for (size_t i = cp3; i < cp4; i++) (*ans)[i] = left;

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
        if (dy == 0) {
            ans += " center at (" + to_string((size_t) v.x) + ", " + to_string((size_t) v.y) + ")";
        }
        ans += "\n";
    }
    return ans;
}

bool isPredictionCorrect(int playerId, const World& last, const World& expected, const World& actual) {
    auto& e = expected;
    auto& a = actual;
    if (e.units.size() != a.units.size()) return false;
    for (size_t i = 0; i < e.units.size(); i++) {
        if (e.units[i].playerId != playerId) continue;

        auto eu = e.units[i];
        auto au = a.units[i];
        if (au.weapon.has_value() && !findUnit(last, au.id).weapon.has_value()) {
            // Since we don't pick up loot boxes in microticks, ignore errors when picking up weapons
            eu.weapon = au.weapon;
        }

        if (eu.toString() != au.toString()) return false;
    }

    auto probablySame = [](const Bullet& b1, const Bullet& b2) {
        return b1.velocity.sqrDist(b2.velocity) < 1e-3;
    };

    for (auto& eb : e.bullets) {
        // Do not check bullets that we created during simulation, since they'll surely be incorrect
        auto previous = find_if(last.bullets.begin(), last.bullets.end(), bind(probablySame, placeholders::_1, eb));
        if (previous == last.bullets.end()) continue;

        // Every other expected bullet should be equal to some bullet in actual
        auto ab = find_if(a.bullets.begin(), a.bullets.end(), bind(probablySame, placeholders::_1, eb));
        if (ab == a.bullets.end() || ab->toString() != eb.toString()) return false;
    }

    for (auto& ab : a.bullets) {
        // Do not check bullets that were actually fired (e.g. by opponent), since we can't predict them
        auto previous = find_if(last.bullets.begin(), last.bullets.end(), bind(probablySame, placeholders::_1, ab));
        if (previous == last.bullets.end()) continue;

        // Every actual bullet should be equal to some bullet in expected
        auto eb = find_if(e.bullets.begin(), e.bullets.end(), bind(probablySame, placeholders::_1, ab));
        if (eb == e.bullets.end() || eb->toString() != ab.toString()) return false;
    }

    return true;
}

void reportPredictionDifference(
    int myId, const World& expectedWorld, const Game& actualGame, const UnitAction& lastAction,
    bool isOk, bool logWorldAnyway, int tick
) {
    auto actual = renderWorld(myId, actualGame.world);
    if (!isOk) {
        cout << "-> " << lastAction.toString() << endl;
    }
    if (!isOk || logWorldAnyway) {
        cout << tick << " " << actual << endl;
    }
    if (!isOk) {
        auto expected = renderWorld(myId, expectedWorld);
        cout << "ERROR! predicted:" << endl << tick << " " << expected << endl;
        if (expected.substr(0, expected.find('\n')) != actual.substr(0, actual.find('\n'))) {
            cout << surroundingToString(findUnit(actualGame.world, myId).position, actualGame.level);
        } else {
            cout << endl;
        }
    }
}

bool finished = false;
int simulationErrors = 0;
static World lastWorld;
static World expectedWorld;
static UnitAction lastAction;

UnitAction checkSimulation(int myId, const Game& game, Debug& debug, bool batch, bool visualize) {
    if (finished) return UnitAction();

    auto& me = findUnit(game.world, myId);
    auto nearestEnemy = minBy<Unit>(game.world.units, [&me](const auto& other) {
        return other.playerId != me.playerId ? me.position.sqrDist(other.position) : 1e100;
    });

    static auto moves = getRandomMoveSequence( /* nearestEnemy->position - me.position */ Vec(0, -1));
    UnitAction ans;

    auto tick = game.currentTick;
    bool ok = tick == 0 || isPredictionCorrect(me.playerId, lastWorld, expectedWorld, game.world);
    if (!ok) simulationErrors++;
    if (!batch) reportPredictionDifference(myId, expectedWorld, game, lastAction, ok, true, tick);

    auto world = game.world;
    simulate(myId, game.level, world, *moves, updatesPerTick, 0, 1, 1, [&](size_t tick, const World& world) {
        expectedWorld = world;
    });

    if (tick == game.properties.maxTickCount - 1 || findUnit(world, myId).health <= 0) {
        if (!batch) {
            cout << "TOTAL ERRORS: ";
        }
        if (!finished || !batch) {
            // In batch mode, simulation finishes if we predict that we're dead on the next turn.
            // If the prediction turns out to be incorrect, we don't want to output total errors more than once.
            cout << simulationErrors << endl;
        }
        finished = true;
    }

    ans = moves->first();

    if (visualize) {
        auto w = game.world;
        debug.log(string("cur: ") + findUnit(w, myId).toString());
        debug.log(ans.toString());

        constexpr size_t ticks = 300;

        simulate(myId, game.level, w, *moves, updatesPerTick, 0, ticks, ticks, [&](size_t tick, const World& world) {
            auto& me = findUnit(world, myId);
            if (tick > 10 && (tick + game.currentTick) % 10 == 0) {
                auto coeff = 1.0f - tick * 1.0f / ticks;
                drawUnit(me, ColorFloat(0.2f * coeff, 0.2f * coeff, 1.0f * coeff, 1.0), debug);
            }
            if (tick > 0 && (tick + game.currentTick) % 5 == 0) {
                auto coeff = 1.0f - tick * 1.0f / ticks;
                for (auto& bullet : world.bullets) {
                    drawBullet(bullet, ColorFloat(0.7f * coeff, 0.2f * coeff, 0.1f * coeff, 0.8), debug);
                }
            }
            if (tick == 0) {
                debug.log(string("next: ") + me.toString());
            }
        });
    }

    moves->consume();
    lastAction = ans;
    lastWorld = game.world;
    return ans;
}
