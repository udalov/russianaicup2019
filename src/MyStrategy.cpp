#include "MyStrategy.hpp"

#include <iostream>
#include <numeric>
#include <vector>
#include "Const.h"
#include "util.h"

using namespace std;

const double EPS = 1e-9;

MyStrategy::MyStrategy(unordered_map<string, string>&& params): params(params) {}

unique_ptr<vector<UnitAction>> getRandomMoveSequence(int updatesPerTick) {
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

    auto t = 100 / updatesPerTick;
    size_t cp1 = 13 * t;
    size_t cp2 = 25 * t;
    size_t cp3 = 186 * t;
    size_t cp4 = 1000 * t;

    for (size_t i = 0; i < cp1; i++) ans->push_back(moveRight);
    for (size_t i = cp1; i < cp2; i++) ans->push_back(jump);
    for (size_t i = cp2; i < cp3; i++) ans->push_back(jumpAndMoveRight);
    for (size_t i = cp3; i < cp4; i++) ans->push_back(left);

    for (auto& t : *ans) {
        t.shoot = true;
        t.aim = Vec(50, 42);
    }
    return ans;
}

Tile getTile(const Level& level, double x, double y) {
    auto i = static_cast<size_t>(x);
    auto j = static_cast<size_t>(y);
    return /* i >= level.tiles.size() || j >= level.tiles[0].size() ? Tile::WALL : */ level.tiles[i][j];
}

Tile getTile(const Level& level, Vec v) {
    return getTile(level, v.x, v.y);
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

bool isWall(const Level& level, double x, double y) {
    return getTile(level, x, y) == Tile::WALL;
}

bool isWallOrPlatform(const Level& level, double x, double y, bool jumpDown) {
    auto tile = getTile(level, x, y);
    // TODO: move tile == LADDER under !jumpDown check?
    return tile == Tile::WALL || tile == Tile::LADDER || (!jumpDown && tile == Tile::PLATFORM);
}

bool intersects(const Unit& unit, const Vec& smth, double sizeX, double sizeY) {
    auto pos = unit.center();
    return 2 * abs(pos.x - smth.x) <= unitSize.x + sizeX &&
        2 * abs(pos.y - smth.y) <= unitSize.y + sizeY;
}

bool intersects(const Unit& unit, const LootBox& box) {
    return intersects(unit, box.position, lootBoxSize.x, lootBoxSize.y);
}

bool intersectsEnemyBullet(const Unit& unit, const Bullet& bullet) {
    constexpr auto eps = 1e-8;
    return intersects(unit, bullet.position, bullet.size + eps, bullet.size + eps);
}

Unit& findUnit(World& world, int id) {
    return *find_if(world.units.begin(), world.units.end(), [id](const auto& unit) { return unit.id == id; });
}

const Unit& findUnit(const World& world, int id) {
    return static_cast<const Unit&>(findUnit(const_cast<World&>(world), id));
}

vector<LootBox> getReachableLootBoxes(const Unit& unit, const World& world, size_t ticks) {
    auto loot = world.lootBoxes;
    auto v = unit.center();
    constexpr auto closenessX = unitSize.x + lootBoxSize.x;
    constexpr auto closenessY = unitSize.y + lootBoxSize.y;
    loot.erase(remove_if(loot.begin(), loot.end(), [&v, ticks](const auto& b) {
        constexpr auto maxSpeedX = 10.0 / ticksPerSecond;
        constexpr auto maxSpeedY = 20.0 / ticksPerSecond;
        return abs(v.x - b.position.x) > (unitSize.x + lootBoxSize.x) / 2 + maxSpeedX * ticks + EPS &&
            abs(v.y - b.position.y) > (unitSize.y + lootBoxSize.y) / 2 + maxSpeedY * ticks + EPS;
    }), loot.end());
    return loot;
}

pair<int, World> simulationPrediction;

void simulate(int myId, const Level& level, World& world, int currentTick, const vector<UnitAction>& moves, Debug *debug, int updatesPerTick, size_t ticks) {
    auto& me = findUnit(world, myId);
    auto alpha = 1.0 / ticksPerSecond / updatesPerTick;

    if (debug) {
        debug->log(string("cur: ") + me.toString());
    }

    auto& x = me.position.x;
    auto& y = me.position.y;
    auto ux = unitSize.x;
    auto uy = unitSize.y;
    auto half = ux / 2;

    auto loot = getReachableLootBoxes(me, world, ticks);

    for (size_t tick = 0; tick < ticks; tick++) {
        auto closestLootBox = min_element(loot.begin(), loot.end(), [&me](const auto& b1, const auto& b2) {
            return b1.position.sqrDist(me.center()) < b2.position.sqrDist(me.center());
        });
        auto& move = moves[tick];
        auto vx = min(max(move.velocity, -unitMaxHorizontalSpeed), unitMaxHorizontalSpeed) * alpha;
        auto vy = 0.0;

        if (me.onLadder) {
            if (move.jump) {
                vy = me.jumpState.speed * alpha;
            }
        } else {
            if (
                me.jumpState.canJump && me.jumpState.maxTime >= -EPS &&
                (move.jump || !me.jumpState.canCancel)
            ) {
                vy = me.jumpState.speed * alpha;
            } else {
                vy = -unitFallSpeed * alpha;
            }
        }

        for (size_t mt = 0; mt < updatesPerTick; mt++) {
            for (size_t i = 0; i < world.bullets.size();) {
                auto& bullet = world.bullets[i];
                // Only simulate enemy bullets for now.
                if (bullet.playerId == me.playerId) { i++; continue; }

                bullet.position += bullet.velocity * alpha;
                auto bx = bullet.position.x;
                auto by = bullet.position.y;

                auto s = bullet.size / 2;
                if (isWall(level, bx - s, by - s) || isWall(level, bx - s, by + s) ||
                    isWall(level, bx + s, by - s) || isWall(level, bx + s, by + s)) {
                    swap(bullet, world.bullets.back());
                    world.bullets.pop_back();
                } else if (intersectsEnemyBullet(me, bullet)) {
                    me.health -= bullet.damage;
                    swap(bullet, world.bullets.back());
                    world.bullets.pop_back();
                } else {
                    i++;
                }
            }

            if (!me.onLadder && me.jumpState.canJump && me.jumpState.maxTime >= -EPS &&
                (move.jump || !me.jumpState.canCancel)) {
                me.jumpState.maxTime -= alpha;
            }

            if (vx < 0 && (
                isWall(level, x - half + vx, y) ||
                isWall(level, x - half + vx, y + uy/2) ||
                isWall(level, x - half + vx, y + uy)
            )) {
                x = floor(x - half) + half + EPS;
            } else if (vx > 0 && (
                isWall(level, x + half + vx, y) ||
                isWall(level, x + half + vx, y + uy/2) ||
                isWall(level, x + half + vx, y + uy)
            )) {
                x = ceil(x + half) - half - EPS;
            } else {
                x += vx;
            }

            if (vy < 0 && (
                isWallOrPlatform(level, x - half, y + vy, move.jumpDown) ||
                isWallOrPlatform(level, x + half, y + vy, move.jumpDown)
            )) {
                y = floor(y) + EPS;
                me.jumpState.canJump = true;
                me.jumpState.canCancel = true;
                me.jumpState.maxTime = unitJumpTime;
                me.jumpState.speed = unitJumpSpeed;
                if (move.jump) {
                    vy = me.jumpState.speed * alpha;
                }
            } else if (vy > 0 && !me.onLadder && (
                isWall(level, x - half, y + uy + vy) || isWall(level, x + half, y + uy + vy)
            )) {
                y = ceil(y + uy) - uy - EPS;
                me.jumpState.canJump = false;
                me.jumpState.canCancel = false;
                me.jumpState.maxTime = 0.0;
                me.jumpState.speed = 0.0;
                vy = -unitFallSpeed * alpha;
            } else {
                y += vy;
            }

            if (getTile(level, x, y) == Tile::LADDER) {
                me.onLadder = true;
                me.jumpState.maxTime = unitJumpTime;
            } else {
                me.onLadder = false;
            }

            auto& weapon = me.weapon;
            if (weapon.has_value()) {
                auto& wp = weapon->params;
                weapon->fireTimer =
                    (weapon->fireTimer.has_value() ? *weapon->fireTimer : wp.reloadTime) - alpha;
                if (*weapon->fireTimer <= 0) {
                    weapon->fireTimer = wp.fireRate;
                    if (--weapon->magazine == 0) {
                        weapon->magazine = wp.magazineSize;
                        weapon->fireTimer = wp.reloadTime;
                    }
                }
            }

            if (closestLootBox != loot.end() && intersects(me, *closestLootBox)) {
                if (closestLootBox->item.isWeapon() && (!me.weapon || move.swapWeapon)) {
                    me.weapon = Weapon();
                    me.weapon->type = closestLootBox->item.weaponType();
                    WeaponParams params;
                    switch (me.weapon->type) {
                        case WeaponType::PISTOL: params = pistolParams; break;
                        case WeaponType::ASSAULT_RIFLE: params = assaultRifleParams; break;
                        case WeaponType::ROCKET_LAUNCHER: params = rocketLauncherParams; break;
                    }
                    me.weapon->params = params;
                    me.weapon->magazine = params.magazineSize;
                    me.weapon->spread = params.minSpread; // ???
                    me.weapon->fireTimer = optional<double>(params.reloadTime);
                    me.weapon->lastAngle = optional<double>(atan2(move.aim.y, move.aim.x));
                }

                size_t i = closestLootBox - loot.begin();
                if (i + 1 != loot.size()) {
                    swap(loot[i], loot.back());
                }
                loot.pop_back();
            }
        }

        if (me.jumpState.maxTime <= -EPS) {
            me.jumpState.canJump = false;
            me.jumpState.canCancel = false;
            me.jumpState.maxTime = 0.0;
            me.jumpState.speed = 0.0;
            vy = 0.0;
        }

        if (getTile(level, x - half, y) == Tile::JUMP_PAD ||
            getTile(level, x + half, y) == Tile::JUMP_PAD ||
            getTile(level, x - half, y + uy) == Tile::JUMP_PAD ||
            getTile(level, x + half, y + uy) == Tile::JUMP_PAD) {
            me.jumpState.canJump = true;
            me.jumpState.canCancel = false;
            me.jumpState.maxTime = jumpPadJumpTime;
            me.jumpState.speed = jumpPadJumpSpeed;
            vy = me.jumpState.speed * alpha;
        }

        if (debug) {
            if (tick > 10 && (tick + currentTick) % 10 == 0) {
                auto coeff = 1.0f - tick * 1.0f / ticks;
                drawUnit(me, ColorFloat(0.2f * coeff, 0.2f * coeff, 1.0f * coeff, 1.0), *debug);
            }
            if (tick > 0 && (tick + currentTick) % 5 == 0) {
                auto coeff = 1.0f - tick * 1.0f / ticks;
                for (auto& bullet : world.bullets) {
                    if (bullet.playerId != me.playerId) {
                        drawBullet(bullet, ColorFloat(0.7f * coeff, 0.2f * coeff, 0.1f * coeff, 0.8), *debug);
                    }
                }
            }
            if (tick == 0) {
                debug->log(string("next: ") + me.toString());
                simulationPrediction = make_pair(me.id, world);
            }
        }
    }
}

string surroundingToString(const Vec& v, const Level& level) {
    string ans = "";
    for (int dy = 2; dy >= -2; dy--) {
        for (int dx = -2; dx <= 2; dx++) {
            auto z = v + Vec(dx, dy);
            auto tile = 0 <= z.x && z.x < level.tiles.size() && 0 <= z.y && z.y < level.tiles[0].size()
                ? getTile(level, z) : Tile::WALL;
            ans += tileToChar(tile);
        }
        ans += "\n";
    }
    return ans;
}

string renderWorld(const pair<int, World>& pair) {
    auto& world = pair.second;
    auto me = findUnit(world, pair.first);
    string ans = me.toString();
    for (auto& bullet : world.bullets) {
        // Only render enemy bullets for now.
        if (bullet.playerId != me.playerId) {
            ans += "\n  " + bullet.toString();
        }
    }
    return ans;
}

bool simulation = false;
int simulationErrors = 0;

UnitAction checkSimulation(int myId, const Game& game, Debug& debug) {
    auto microticks = updatesPerTick;
    static auto moves = getRandomMoveSequence(microticks);
    UnitAction ans;
    if (moves->empty()) return ans;

    auto tick = game.currentTick;
    if (tick != 0) {
        auto actual = renderWorld(make_pair(myId, game.world));
        auto expected = renderWorld(simulationPrediction);
        cout << tick << " " << actual << endl;
        if (expected != actual) {
            cout << "ERROR! predicted:" << endl << tick << " " << expected << endl;
            if (expected.substr(0, expected.find("\n")) != actual.substr(0, actual.find("\n"))) {
                auto& me = findUnit(game.world, myId);
                cout << surroundingToString(me.position, game.level);
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
    simulate(myId, game.level, world, game.currentTick, *moves, &debug, microticks, 300);
    ans = *moves->begin();
    moves->erase(moves->begin());
    return ans;
}

typedef vector<UnitAction> Track;

vector<Track> generateTracks(size_t len) {
    vector<Track> ans;
    Track t(len);
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
    return ans;
}

double estimate(const World& world, int myId, const Unit *nearestEnemy, const LootBox *nearestWeapon) {
    auto& me = findUnit(world, myId);
    auto score = me.health * 100;
    if (!me.weapon) {
        score += -100.0 - me.position.distance(nearestWeapon->position);
    } else {
        score += -10.0 - abs(me.position.distance(nearestEnemy->position) - 10.0);
    }
    return score;
}

vector<Track> savedTracks;

UnitAction MyStrategy::getAction(int myId, const Game& game, Debug& debug) {
    auto tick = game.currentTick;
    if (tick == 0) {
        srand(42);
        if (params.find("--dump-constants") != params.end()) {
            dumpConstants(game.properties);
            terminate();
        }
        checkConstants(game.properties);
        simulation = params.find("--simulate") != params.end();
    }

    auto& me = findUnit(game.world, myId);
    if (simulation) return checkSimulation(myId, game, debug);

    auto nearestEnemy = minBy(game.world.units, [&me](const auto& other) {
        return other.playerId != me.playerId ? me.position.sqrDist(other.position) : 1e100;
    });
    auto nearestWeapon = minBy(game.world.lootBoxes, [&me](const auto& lootBox) {
        return lootBox.item.isWeapon() ? me.position.sqrDist(lootBox.position) : 1e100;
    });

    constexpr size_t trackLen = 80;
    constexpr size_t tracksToSave = 10;
    constexpr size_t microticks = 4;

    auto tracks = generateTracks(trackLen);
    for (auto& track : savedTracks) track.push_back(track.back()), tracks.push_back(track);
    savedTracks.clear();

    auto scores = vector<double>(tracks.size());
    for (size_t i = 0; i < tracks.size(); i++) {
        auto& track = tracks[i];
        auto world = game.world;
        simulate(myId, game.level, world, game.currentTick, track, nullptr, microticks, min(track.size(), trackLen));
        scores[i] = estimate(world, myId, nearestEnemy, nearestWeapon);
    }

    auto indices = vector<size_t>(tracks.size());
    iota(indices.begin(), indices.end(), 0);

    sort(indices.begin(), indices.end(), [&scores](auto& i1, auto& i2) { return scores[i1] > scores[i2]; });

    for (size_t i = 0; i < tracksToSave && i < tracks.size(); i++) savedTracks.push_back(tracks[indices[i]]);

    auto ans = tracks.empty() ? UnitAction() : tracks[indices.front()].front();
    // cout << tick << " " << ans.toString() << endl;
    // cout << tick << " " << me.toString() << endl;
    ans.aim = nearestEnemy->position - me.position;
    ans.shoot = true;
    return ans;
    /*
    auto nearestEnemy = minBy(game.units, [&me](auto& other) {
        return other.playerId != me.playerId ? me.position.sqrDist(other.position) : 1e100;
    });
    auto nearestWeapon = minBy(game.lootBoxes, [&me](auto& lootBox) {
        return lootBox.item.isWeapon() ? me.position.sqrDist(lootBox.position) : 1e100;
    });
    Vec target = me.position;
    if (me.weapon == nullptr && nearestWeapon != nullptr) {
        target = nearestWeapon->position;
    } else if (nearestEnemy != nullptr) {
        target = nearestEnemy->position;
    }
    debug.log(string("Me: ") + me.position.toString());
    debug.log(string("Target: ") + target.toString());
    Vec aim;
    if (nearestEnemy != nullptr) {
        aim = Vec(nearestEnemy->position.x - me.position.x, nearestEnemy->position.y - me.position.y);
    }
    bool jump = target.y > me.position.y;
    if (target.x > me.position.x && game.level.tiles[me.position.x + 1][me.position.y] == Tile::WALL) {
        jump = true;
    }
    if (target.x < me.position.x && game.level.tiles[me.position.x - 1][me.position.y] == Tile::WALL) {
        jump = true;
    }
    UnitAction action;
    action.velocity = target.x - me.position.x;
    action.jump = jump;
    action.jumpDown = !action.jump;
    action.aim = aim;
    action.shoot = true;
    action.swapWeapon = false;
    action.plantMine = false;
    return action;
    */
}
