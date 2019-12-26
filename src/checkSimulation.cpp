#include "checkSimulation.h"

#include "Const.h"
#include "Simulation.h"
#include "util.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

struct Moves {
    UnitAction defaultMove;
    UnitAction right;
    UnitAction left;
    UnitAction jump;
    UnitAction plantMine;
};

Moves createMoves() {
    Moves ans;

    ans.defaultMove = UnitAction();
    ans.defaultMove.shoot = true;
    ans.defaultMove.aim = Vec(0, -1);

    ans.right = ans.defaultMove;
    ans.right.velocity = 10.0;
    ans.left = ans.defaultMove;
    ans.left.velocity = -10.0;

    ans.jump = ans.defaultMove;
    ans.jump.jump = true;
    ans.plantMine = ans.defaultMove;
    ans.plantMine.plantMine = true;

    return ans;
}

UnitAction operator+(const UnitAction& a1, const UnitAction& a2) {
    assert(a1.velocity == 0.0 || a2.velocity == 0.0);
    assert(a1.aim == Vec() || a2.aim == Vec());
    return UnitAction(
        a1.velocity + a2.velocity,
        a1.jump || a2.jump,
        a1.jumpDown || a2.jumpDown,
        a1.aim + a2.aim,
        a1.shoot || a2.shoot,
        a1.reload || a2.reload,
        a1.swapWeapon || a2.swapWeapon,
        a1.plantMine || a2.plantMine
    );
}

#define repeat(n) for (size_t i = 0; i < (n); i++)

Track getPredefinedMoveSequence() {
    static Moves moves = createMoves();

    vector<UnitAction> ans;

    repeat(13) ans.push_back(moves.right);
    repeat(12) ans.push_back(moves.jump);
    repeat(161) ans.push_back(moves.jump + moves.right);
    ans.push_back(moves.left);

    /*
    // Check mine trigger radius and explosion
    repeat(100) ans.push_back(moves.right);
    ans.push_back(moves.plantMine);
    repeat(10) ans.push_back(moves.right);
    auto rh = moves.right;
    rh.velocity = 0.2000001 * rh.velocity;
    ans.push_back(rh);
    repeat(60) ans.push_back(moves.defaultMove);
    repeat(17) ans.push_back(moves.right);
    ans.push_back(moves.defaultMove);
    */

    return Track(ans);
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

    if (e.mines.size() != a.mines.size()) return false;
    for (size_t i = 0; i < e.mines.size(); i++) {
        if (e.mines[i].toString() != a.mines[i].toString()) return false;
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

    static auto moves = getPredefinedMoveSequence();
    UnitAction ans;

    auto tick = game.currentTick;
    bool ok = tick == 0 || isPredictionCorrect(findUnit(game.world, myId).playerId, lastWorld, expectedWorld, game.world);
    if (!ok) simulationErrors++;
    if (!batch) reportPredictionDifference(myId, expectedWorld, game, lastAction, ok, true, tick);

    auto world = game.world;
    simulate(myId, game.level, world, moves, updatesPerTick, 0, 1, 1, [&](size_t tick, const World& world) {
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

    ans = moves.first();

    if (visualize) {
        auto w = game.world;
        debug.log(string("cur: ") + findUnit(w, myId).toString());
        debug.log(ans.toString());

        constexpr size_t ticks = 300;

        simulate(myId, game.level, w, moves, updatesPerTick, 0, ticks, ticks, [&](size_t tick, const World& world) {
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

    moves.consume();
    lastAction = ans;
    lastWorld = game.world;
    return ans;
}
