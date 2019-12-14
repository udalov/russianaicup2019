#include "MyStrategy.hpp"

#include <iostream>
#include <numeric>
#include <vector>
#include "Const.h"
#include "util.h"

using namespace std;

const double EPS = 1e-9;

MyStrategy::MyStrategy() : params() {}
MyStrategy::MyStrategy(const unordered_map<string, string>& params) : params(params) {}

unique_ptr<vector<UnitAction>> getRandomMoveSequence(int updatesPerTick, const Vec& aim) {
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
        t.aim = aim;
    }
    return ans;
}

void log(Debug& debug, const string& message) {
    debug.draw(CustomData::Log(message));
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

bool intersects(const Unit& unit, const Vec& rectCenter, double sizeX, double sizeY) {
    auto unitCenter = unit.center();
    return 2 * abs(unitCenter.x - rectCenter.x) <= unitSize.x + sizeX &&
        2 * abs(unitCenter.y - rectCenter.y) <= unitSize.y + sizeY;
}

bool intersects(const Unit& unit, const LootBox& box) {
    return intersects(unit, box.center(), lootBoxSize.x, lootBoxSize.y);
}

bool intersectsBullet(const Unit& unit, const Bullet& bullet, double eps = 0.0) {
    return intersects(unit, bullet.position, bullet.size + eps, bullet.size + eps);
}

Unit& findUnit(World& world, int id) {
    return *find_if(world.units.begin(), world.units.end(), [id](const auto& unit) { return unit.id == id; });
}

const Unit& findUnit(const World& world, int id) {
    return static_cast<const Unit&>(findUnit(const_cast<World&>(world), id));
}

void removeUnreachableLootBoxes(const Unit& unit, World& world, size_t ticks) {
    auto& loot = world.lootBoxes;
    auto v = unit.center();
    constexpr auto closenessX = unitSize.x + lootBoxSize.x;
    constexpr auto closenessY = unitSize.y + lootBoxSize.y;
    loot.erase(remove_if(loot.begin(), loot.end(), [&v, ticks](const auto& box) {
        constexpr auto maxSpeedX = 10.0 / ticksPerSecond;
        constexpr auto maxSpeedY = 20.0 / ticksPerSecond;
        auto b = box.center();
        return abs(v.x - b.x) > (unitSize.x + lootBoxSize.x) / 2 + maxSpeedX * ticks + EPS &&
            abs(v.y - b.y) > (unitSize.y + lootBoxSize.y) / 2 + maxSpeedY * ticks + EPS;
    }), loot.end());
}

pair<int, World> simulationPrediction;

void simulate(
    int myId, const Level& level, World& world, const vector<UnitAction>& moves, int updatesPerTick, size_t ticks,
    const function<void(size_t, const World&)>& callback
) {
    auto& me = findUnit(world, myId);
    auto alpha = 1.0 / ticksPerSecond / updatesPerTick;

    auto& x = me.position.x;
    auto& y = me.position.y;
    auto ux = unitSize.x;
    auto uy = unitSize.y;
    auto half = ux / 2;

    removeUnreachableLootBoxes(me, world, ticks);

    for (size_t tick = 0; tick < ticks; tick++) {
        auto closestLootBox = minBy<LootBox>(world.lootBoxes, [&me](const auto& box) {
            return box.center().sqrDist(me.center());
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

                bullet.position += bullet.velocity * alpha;
                auto bx = bullet.position.x;
                auto by = bullet.position.y;

                auto s = bullet.size / 2;
                if (isWall(level, bx - s, by - s) || isWall(level, bx - s, by + s) ||
                    isWall(level, bx + s, by - s) || isWall(level, bx + s, by + s)) {
                    auto explosion = bullet.explosionParams;
                    if (explosion.has_value()) {
                        auto size = explosion->radius;
                        for (auto& unit : world.units) {
                            if (intersects(unit, Vec(bx, by), 2 * size, 2 * size)) {
                                unit.health -= explosion->damage;
                            }
                        }
                    }
                    fastRemove(world.bullets, bullet);
                    goto nextBullet;
                }

                // TODO: optimize
                for (auto& unit : world.units) {
                    if (unit.id != bullet.unitId && intersectsBullet(unit, bullet, 1e-8 /* TODO */)) {
                        unit.health -= bullet.damage;
                        fastRemove(world.bullets, bullet);
                        goto nextBullet;
                    }
                }
                i++;
nextBullet:;
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
                weapon->fireTimer -= alpha;
                if (weapon->fireTimer <= 0 && move.shoot) {
                    auto& bp = wp.bullet;
                    auto aim = move.aim;
                    auto explosion = weapon->type == WeaponType::ROCKET_LAUNCHER ? optional<ExplosionParams>(rocketLauncherParams.explosion) : nullopt;
                    world.bullets.push_back(Bullet(weapon->type, me.id, me.playerId, me.position, aim.normalize() * bp.speed, bp.damage, bp.size, explosion));
                    if (--weapon->magazine == 0) {
                        weapon->magazine = wp.magazineSize;
                        weapon->fireTimer = wp.reloadTime;
                    } else {
                        weapon->fireTimer = wp.fireRate;
                    }
                }
            }

            if (closestLootBox && intersects(me, *closestLootBox)) {
                auto item = closestLootBox->item;
                if (item.isWeapon() && (!me.weapon || move.swapWeapon)) {
                    me.weapon = Weapon();
                    me.weapon->type = item.weaponType();
                    WeaponParams params;
                    switch (me.weapon->type) {
                        case WeaponType::PISTOL: params = pistolParams; break;
                        case WeaponType::ASSAULT_RIFLE: params = assaultRifleParams; break;
                        case WeaponType::ROCKET_LAUNCHER: params = rocketLauncherParams; break;
                    }
                    me.weapon->params = params;
                    me.weapon->magazine = params.magazineSize;
                    me.weapon->spread = params.minSpread; // ???
                    me.weapon->fireTimer = params.reloadTime;
                    me.weapon->lastAngle = optional<double>(atan2(move.aim.y, move.aim.x));
                    fastRemove(world.lootBoxes, *closestLootBox);
                } else if (item.isHealthPack() && item.health() > 0 && me.health < 100) {
                    me.health = min(me.health + item.health(), 100);
                    fastRemove(world.lootBoxes, *closestLootBox);
                }
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

        callback(tick, world);
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
        ans += "\n  " + bullet.toString();
    }
    return ans;
}

bool simulation = false;
int simulationErrors = 0;

UnitAction checkSimulation(int myId, const Game& game, Debug& debug) {
    auto& me = findUnit(game.world, myId);
    auto nearestEnemy = minBy(game.world.units, [&me](const auto& other) {
        return other.playerId != me.playerId ? me.position.sqrDist(other.position) : 1e100;
    });

    auto microticks = updatesPerTick;
    static auto moves = getRandomMoveSequence(microticks, /* nearestEnemy->position - me.position */ Vec(0, -1));
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
    log(debug, string("cur: ") + findUnit(world, myId).toString());

    auto currentTick = game.currentTick;
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
            log(debug, string("next: ") + me.toString());
            simulationPrediction = make_pair(myId, world);
        }
    });
    ans = *moves->begin();
    moves->erase(moves->begin());
    return ans;
}

typedef vector<UnitAction> Track;

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

bool needToShoot(const Unit& me, const Game& game, Track track, const Vec& aim) {
    if (!me.weapon) return false;
    if (me.weapon->fireTimer > 0.0) return false;
    if (me.weapon->type != WeaponType::ROCKET_LAUNCHER) return true;

    auto world1 = game.world;
    simulate(
        me.id, game.level, world1, track, 4, track.size(),
        [&](size_t tick, const World& world) { }
    );
    int expectedHealth = findUnit(world1, me.id).health;

    track[0].shoot = true;
    track[0].aim = aim;

    auto world2 = game.world;
    simulate(
        me.id, game.level, world2, track, 4, track.size(),
        [&](size_t tick, const World& world) { }
    );
    int actualHealth = findUnit(world2, me.id).health;
    return actualHealth >= expectedHealth;
}

vector<Track> savedTracks;

UnitAction MyStrategy::getAction(const Unit& myUnit, const Game& game, Debug& debug) {
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

    auto myId = myUnit.id;
    auto& me = findUnit(game.world, myId);
    if (simulation) return checkSimulation(myId, game, debug);

    auto nearestEnemy = minBy(game.world.units, [&me](const auto& other) {
        return other.playerId != me.playerId ? me.position.sqrDist(other.position) : 1e100;
    });
    auto nearestWeapon = minBy(game.world.lootBoxes, [&me](const auto& lootBox) {
        return lootBox.item.isWeapon() ? me.position.sqrDist(lootBox.position) : 1e100;
    });
    auto magazineAtStart = me.weapon ? me.weapon->magazine : -1;

    constexpr size_t trackLen = 80;
    constexpr size_t tracksToSave = 10;
    constexpr size_t microticks = 4;

    constexpr size_t estimateCutoff = 20;
    constexpr size_t estimateEveryNth = 10;

    auto tracks = generateTracks(trackLen, me, nearestEnemy);
    // cout << "### " << tick << " | " << tracks.size() << " tracks | " << me.toString() << endl;
    for (auto& track : savedTracks) track.push_back(track.back()), tracks.push_back(track);
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

    for (size_t i = 0; i < tracksToSave && i < tracks.size(); i++) savedTracks.push_back(tracks[indices[i]]);

    auto aim = nearestEnemy->position - me.position;
    auto shoot = needToShoot(me, game, tracks[indices.front()], aim);
    auto ans = tracks.empty() ? UnitAction() : tracks[indices.front()].front();
    // cout << tick << " " << ans.toString() << endl;
    // cout << tick << " " << me.toString() << endl;
    ans.aim = aim;
    ans.shoot = shoot;
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
    log(debug, string("Me: ") + me.position.toString());
    log(debug, string("Target: ") + target.toString());
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
