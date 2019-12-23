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

struct AllyData {
    vector<Track> savedTracks;

    AllyData() noexcept : savedTracks() {}
};

AllyData datas[2];
int minAllyId;
int maxAllyId;
bool isInitialized = false;
bool visualize = false;
bool simulation = false;
bool quick = false;
bool optimistic = false;

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
        simulation = params.find("--simulate") != params.end();
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

    if (simulation) return checkSimulation(myId, game, debug);

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

    auto aim = nearestEnemy->position - me.position;
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

    return ans;
}
