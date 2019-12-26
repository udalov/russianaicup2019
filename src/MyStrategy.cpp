#include "MyStrategy.hpp"

#include <iostream>
#include <numeric>
#include <vector>
#include "Const.h"
#include "util.h"
#include "Simulation.h"
#include "checkSimulation.h"

using namespace std;

constexpr auto defaultHighResCutoff = 5;
constexpr auto defaultMicroticks = 4;

MyStrategy::MyStrategy() : params() {}
MyStrategy::MyStrategy(unordered_map<string, string> params) : params(move(params)) {}

vector<Track> generateTracks(size_t len, const Unit &me, size_t randomTracks) {
    vector<Track> ans;
    ans.reserve(9 + randomTracks);

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

    for (size_t times = 0; times < randomTracks; times++) {
        size_t t1 = randomInt() % ans.size();
        size_t t2;
        do {
            t2 = randomInt() % ans.size();
        } while (t1 == t2);
        size_t mid = randomInt() % len;
        for (size_t i = 0; i < mid; i++) t[i] = ans[t1][i];
        for (size_t i = mid; i < len; i++) t[i] = ans[t2][i];
        ans.push_back(t);
    }

    return ans;
}

const int DX[] = {1, 0, -1, 0};
const int DY[] = {0, 1, 0, -1};
vector<int> TEMP_QUEUE;

struct Paths {
    vector<vector<Tile>> tiles;
    size_t n, m;
    vector<vector<int>> answers;

    explicit Paths(const vector<vector<Tile>>& tiles) : tiles(tiles), n(tiles.size()), m(tiles[0].size()),
        answers(n * m, vector<int>(n * m, -1)) {}

    constexpr size_t enc(size_t x, size_t y) const { return x * m + y; }

    int distance(size_t x1, size_t y1, size_t x2, size_t y2) {
        auto start = enc(x1, y1);
        auto finish = enc(x2, y2);
        auto& ans = answers.at(start);
        if (ans[start] != -1) return ans[finish];

        auto& q = TEMP_QUEUE;
        q.clear();
        q.push_back(start);
        ans[start] = 0;
        size_t qb = 0;
        while (qb < q.size()) {
            auto v = q[qb++];
            auto x = v / m, y = v % m;
            for (size_t d = 0; d < 4; d++) {
                auto xx = x + DX[d], yy = y + DY[d];
                auto vv = enc(xx, yy);
                if (tiles[xx][yy] != Tile::WALL && ans[vv] == -1) {
                    ans[vv] = ans[v] + 1;
                    q.push_back(vv);
                }
            }
        }

        return ans[finish];
    }

    int distance(const Vec& from, const Vec& to) {
        return distance(
            static_cast<size_t>(from.x), static_cast<size_t>(from.y),
            static_cast<size_t>(to.x), static_cast<size_t>(to.y)
        );
    }
};

unique_ptr<Paths> paths = nullptr;

double smartDistance(const Vec& v, const Vec& w) {
    return (paths->distance(v, w) + v.distance(w)) / 2.0;
}

double cosBetween(const Vec& v, const Vec& w) {
    return v.dot(w) / sqrt(v.sqrLen() * w.sqrLen());
}

// Returns a value in [0, 1]; 0 is bad, 1 is good.
double surroundScore(const World& world, const Unit& me) {
    const Unit *ally = nullptr;
    for (auto& unit : world.units) if (unit.id != me.id && unit.playerId == me.playerId) { ally = &unit; break; }
    if (!ally) return 0.0;

    auto ans = 0.0;
    auto k = 0;
    for (auto& enemy : world.units) if (enemy.playerId != me.playerId) {
        ans += (1 - cosBetween(me.position - enemy.position, ally->position - enemy.position)) / 2.0;
        k++;
    }
    return ans / k;
}

constexpr auto surroundScoreWeight = 10.0; // TODO

double estimate(const World &world, int myId, const Unit& nearestEnemy, const LootBox *nearestWeapon, double desiredDistanceToEnemy) {
    auto& me = findUnit(world, myId);
    auto score = 0.0;
    for (auto& unit : world.units) {
        // TODO: intelligent suicide
        if (unit.playerId == me.playerId) score += 2 * unit.health;
        else score -= unit.health;
    }
    score *= 10000;

    if (!me.weapon && nearestWeapon) {
        score += -100.0 - smartDistance(me.position, nearestWeapon->position);
    } else {
        score += -10.0 - abs(smartDistance(me.position, nearestEnemy.position) - desiredDistanceToEnemy);
    }

    score += surroundScore(world, me) * surroundScoreWeight;

    return score;
}

int getMyHealthScore(const World& world, int playerId) {
    int ans = 0;
    for (auto& unit : world.units) if (unit.playerId == playerId) ans += unit.health;
    ans *= 10000;
    for (auto& box : world.lootBoxes) if (box.item.isHealthPack()) ans += box.item.health();
    return ans;
}

bool needToShoot(const Unit& me, const Game& game, Track track, Vec& aim) {
    if (!me.weapon) return false;
    if (me.weapon->fireTimer > 0.0) return false;
    // if (me.weapon->type != WeaponType::ROCKET_LAUNCHER) return true;
    if (me.weapon->type != WeaponType::ROCKET_LAUNCHER && none_of(
        game.world.units.begin(), game.world.units.end(), [&me](const auto& unit) { return unit.id != me.id && unit.playerId == me.playerId; }
    )) return true;

    auto size = min(track.size(), 30ul);

    auto world1 = game.world;
    simulate(
        me.id, game.level, world1, track, defaultMicroticks, defaultHighResCutoff, size, size,
        [&](size_t tick, const World& world) { }
    );
    int expectedHealthScore = getMyHealthScore(world1, me.playerId);

    track[0].shoot = true;
    track[0].aim = aim;

    auto world2 = game.world;
    simulate(
        me.id, game.level, world2, track, defaultMicroticks, defaultHighResCutoff, size, size,
        [&](size_t tick, const World& world) { }
    );
    int actualHealthScore = getMyHealthScore(world2, me.playerId);
    // cout << "### " << game.currentTick << " me " << me.toString() << " expected " << expectedHealthScore << " actual " << actualHealthScore;
    // cout << " result " << findUnit(world2, me.id).toString() << endl;
    return actualHealthScore >= expectedHealthScore;
}

struct WallSegment {
    size_t x1, y1, x2, y2;

    WallSegment(size_t x1, size_t y1, size_t x2, size_t y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}
};

void drawRect(Debug& debug, double x1, double y1, double x2, double y2) {
    auto r = (100 + randomInt() % 100) / 255.0;
    auto g = (100 + randomInt() % 100) / 255.0;
    auto b = (100 + randomInt() % 100) / 255.0;
    debug.draw(CustomData::Rect(Vec(x1, y1), Vec(x2 - x1, y2 - y1), ColorFloat(r, g, b, 1.0)));
}

void drawSegment(Debug& debug, const WallSegment& segment) {
    auto dx = segment.x1 == segment.x2 ? 0.1 : 0.0;
    auto dy = segment.y1 == segment.y2 ? 0.1 : 0.0;
    drawRect(debug, segment.x1 - dx, segment.y1 - dy, segment.x2 + dx, segment.y2 + dy);
}

vector<WallSegment> getWallSegments(const Level& level, Debug& debug) {
    vector<WallSegment> ans;

    auto& tiles = level.tiles;
    auto n = tiles.size(), m = tiles.front().size();
    constexpr auto W = Tile::WALL;
    for (size_t i = 0; i < n - 1; i++) {
        for (size_t j = 0; j < m; j++) {
            size_t k = j;
            while (k < m && (tiles[i][k] == W) != (tiles[i + 1][k] == W)) k++;
            if (k > j) {
                ans.emplace_back(i + 1, j, i + 1, k);
                j = k;
            }
        }
    }
    for (size_t j = 0; j < m - 1; j++) {
        for (size_t i = 0; i < n; i++) {
            size_t k = i;
            while (k < n && (tiles[k][j] == W) != (tiles[k][j + 1] == W)) k++;
            if (k > i) {
                ans.emplace_back(i, j + 1, k, j + 1);
                i = k;
            }
        }
    }

    /*
    cerr << ans.size() << " wall segments" << endl;
    for (auto& segment : ans) {
        drawSegment(debug, segment);
    }
    */

    return ans;
}

Vec rotate(const Vec& v, double a) {
    auto cosA = cos(a);
    auto sinA = sin(a);
    return Vec(cosA * v.x - sinA * v.y, sinA * v.x + cosA * v.y);
}

constexpr auto PP = 2 * M_PI;

/*
double getAngle(const Vec& v, double increment = 0.0) {
    auto angle = atan2(v.y, v.x) + increment;
    return angle < 0.0 ? angle + PP : angle >= PP ? angle - PP : angle;
}
*/

double angleDiff(double a, double b) {
    return min(abs(a - b), min(abs(a - b - PP), abs(a - b + PP)));
}

void drawRay(Debug& debug, const Vec& start, const Vec& direction, const ColorFloat& color) {
    debug.draw(CustomData::Line(start, start + direction.normalize() * 30.0, 0.05, color));
}

bool intersectSegments(const WallSegment& s, const Vec& p1, const Vec& p2) {
    if (s.x1 == s.x2) {
        auto x = s.x1;
        if ((p1.x < x) == (p2.x < x)) return false;
        if (abs(p1.x - p2.x) < 1e-4) return false;
        auto y = p1.y + (p2.y-p1.y)*(x-p1.x)/(p2.x-p1.x);
        return s.y1 <= y && y <= s.y2;
    } else if (s.y1 == s.y2) {
        auto y = s.y1;
        if ((p1.y < y) == (p2.y < y)) return false;
        if (abs(p1.y - p2.y) < 1e-4) return false;
        auto x = p1.x + (p2.x-p1.x)*(y-p1.y)/(p2.y-p1.y);
        return s.x1 <= x && x <= s.x2;
    } else {
        return false;
    }
}

Vec adjustAimIfNeeded(const Unit& me, const Vec& aim, const vector<WallSegment>& segments, Debug& debug) {
    if (!me.weapon.has_value()) return aim;
    if (me.weapon->type != WeaponType::ROCKET_LAUNCHER) return aim;

    auto pos = me.center();
    auto& wp = me.weapon->params;
    auto shootTicks = (size_t) ceil(me.weapon->fireTimer * ticksPerSecond);
    auto spreadBase = me.weapon->spread - shootTicks * wp.aimSpeed / ticksPerSecond;

    auto lastAngle = me.weapon->lastAngle.has_value() ? *me.weapon->lastAngle : M_PI;
    // drawRay(debug, pos, Vec(cos(lastAngle), sin(lastAngle)), ColorFloat(0.5,0.4,0.3,1.0));

    constexpr auto lengthThreshold = 4.0;
    constexpr static auto squareCorners = array<Vec, 4> { Vec(1,1), Vec(-1,1), Vec(-1,-1), Vec(1,-1) };

    auto ans = aim;
    auto best = 1e100;
    for (size_t i = 0; i < 100; i++) {
        auto angle = i == 0 ? lastAngle : (randomInt() % 1000) / 1000.0 * PP;
        auto newSpread = max(min(spreadBase + angleDiff(lastAngle, angle), wp.maxSpread), wp.minSpread);
        auto newAim = Vec(cos(angle), sin(angle));
        // drawRay(debug, pos, newAim, ColorFloat(0.2,0.4,0.7,1.0));
        auto ok = true;
        for (auto& squareCorner : squareCorners) {
            auto p = pos + squareCorner * (wp.bullet.size / 2);
            for (auto& s : segments) {
                if (intersectSegments(s, p, p + newAim * lengthThreshold)) {
                    ok = false; break;
                }
                for (double step = newSpread; step > 0; step -= 0.1) {
                    if (intersectSegments(s, p, p + rotate(newAim, -step) * lengthThreshold) ||
                        intersectSegments(s, p, p + rotate(newAim, step) * lengthThreshold)) {
                        ok = false; break;
                    }
                }
                if (!ok) break;
            }
            if (!ok) break;
        }
        if (!ok) continue;
        auto cur = angleDiff(atan2(aim.y, aim.x), angle);
        if (cur < best) {
            best = cur;
            ans = newAim;
        }
    }

    return ans;
}

struct AllyData {
    vector<Track> savedTracks;

    AllyData() noexcept : savedTracks() {}
};

AllyData datas[2];
int minAllyId;
int maxAllyId;
bool isInitialized = false;
bool visualize = false;
bool simulateOneAction = false;
bool checkSimulationOnPredefinedMoveSequence = false;
bool quick = false;
bool optimistic = false;
bool batch = false;

World lastWorld;
World expectedWorld;
UnitAction lastAction;

UnitAction MyStrategy::getAction(const Unit& myUnit, const Game& game, Debug& debug) {
    constexpr size_t trackLen = 160;

    auto tick = game.currentTick;
    auto myId = myUnit.id;
    auto& me = findUnit(game.world, myId);
    if (!isInitialized) {
        isInitialized = true;
        if (params.find("--dump-constants") != params.end()) {
            dumpConstants(game.properties);
            terminate();
        }
        checkConstants(game.properties);
        simulateOneAction = params.find("--simulate") != params.end();
        checkSimulationOnPredefinedMoveSequence = params.find("--check-simulation") != params.end();
        batch = params.find("--batch") != params.end();
        visualize = params.find("--vis") != params.end();
        quick = params.find("--quick") != params.end();
        optimistic = params.find("--optimistic") != params.end();

        minAllyId = numeric_limits<int>::max();
        maxAllyId = numeric_limits<int>::min();
        for (auto& unit : game.world.units) {
            if (unit.playerId == me.playerId) {
                minAllyId = min(minAllyId, unit.id);
                maxAllyId = max(maxAllyId, unit.id);
            }
        }

        paths = make_unique<Paths>(game.level.tiles);
    }

    if (checkSimulationOnPredefinedMoveSequence) return checkSimulation(myId, game, debug, batch, visualize);

    static auto wallSegments = getWallSegments(game.level, debug);

    auto& data = datas[me.id != minAllyId];

    auto allyId = minAllyId + maxAllyId - myId;
    if (none_of(game.world.units.begin(), game.world.units.end(), [allyId](const auto& unit) { return unit.id == allyId; })) allyId = -1;

    auto nearestEnemy = minBy<Unit>(game.world.units, [&me](const auto& other) {
        return other.playerId != me.playerId ? me.position.sqrDist(other.position) : 1e100;
    });
    if (!nearestEnemy) return UnitAction();
    auto nearestWeapon = minBy<LootBox>(game.world.lootBoxes, [&me](const auto& box) {
        return box.item.isWeapon() ? me.position.sqrDist(box.position) : 1e100;
    });

    constexpr size_t tracksToSave = 10;

    constexpr size_t estimateCutoff = 20;
    constexpr size_t estimateEveryNth = 10;

    auto tracks = generateTracks(trackLen, me, quick ? 10 : 100);
    // cout << "### " << tick << " | " << tracks.size() << " tracks | " << me.toString() << endl;
    auto& savedTracks = data.savedTracks;
    tracks.insert(tracks.end(), savedTracks.begin(), savedTracks.end());
    savedTracks.clear();

    bool isWinning = optimistic ||
        (game.players[0].score > game.players[1].score) == (game.players[0].id == me.playerId);
    // This value is equal to 10 at tick=1200, and to 5 at tick=3600.
    auto desiredDistanceToEnemy = isWinning ? 10.0 : min((6000.0 - tick) / 480.0, 10.0);

    auto scores = vector<double>(tracks.size());
    for (size_t i = 0; i < tracks.size(); i++) {
        auto& track = tracks[i];
        auto world = game.world;
        auto score = 0.0;
        simulate(
            myId, game.level, world, track, defaultMicroticks, defaultHighResCutoff, trackLen / 2, trackLen,
            [&](size_t tick, const World& world) {
                if (tick >= estimateCutoff && tick % estimateEveryNth == 0) {
                    auto coeff = (double)(trackLen - tick) / (trackLen - estimateCutoff) * 0.5 + 0.5;
                    score += coeff * estimate(world, myId, *nearestEnemy, nearestWeapon, desiredDistanceToEnemy);
                }
            }
        );
        scores[i] = score;
    }

    auto indices = vector<size_t>(tracks.size());
    iota(indices.begin(), indices.end(), 0);

    sort(indices.begin(), indices.end(), [&scores](auto& i1, auto& i2) { return scores[i1] > scores[i2]; });

    auto& bestTrack = tracks[indices.front()];

    if (visualize) {
        debug.log(renderWorld(myId, game.world));
        auto w = game.world;
        simulate(
            myId, game.level, w, bestTrack, defaultMicroticks, defaultHighResCutoff, 4, 4,
            [&](size_t tick, const World& world) {
                debug.log(string("+") + to_string(tick) + " " + bestTrack[tick].toString() + " -> " + renderWorld(myId, world));
            }
        );
    }

    auto aim = adjustAimIfNeeded(me, nearestEnemy->position - me.position, wallSegments, debug);
    auto shoot = needToShoot(me, game, bestTrack, aim);
    auto ans = bestTrack.first();
    ans.aim = aim;
    ans.shoot = shoot;
    // cout << tick << " " << ans.toString() << endl;
    // cout << tick << " " << me.toString() << endl;

    for (size_t i = 0; i < tracksToSave && i < tracks.size(); i++) {
        auto& track = tracks[indices[i]];
        track.consume();
        savedTracks.emplace_back(track);
    }

    /*
    if (myId == minAllyId) {
        auto ss = surroundScore(game.world, me);
        cout << "### " << tick << " ss=" << ss << " ss*w=" << ss * surroundScoreWeight << endl;
    }
    */

    if (simulateOneAction) {
        auto world = game.world;
        bool ok = tick == 0 || isPredictionCorrect(me.playerId, lastWorld, expectedWorld, world);
        reportPredictionDifference(myId, expectedWorld, game, lastAction, ok, false, tick);

        simulate(myId, game.level, world, Track { ans }, updatesPerTick, 0, 1, 1);
        expectedWorld = world;
        lastWorld = game.world;
        lastAction = ans;
    }

    return ans;
}
