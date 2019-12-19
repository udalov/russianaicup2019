#include "Debug.hpp"
#include "MyStrategy.hpp"
#include "TcpStream.hpp"
#include "model/PlayerMessageGame.hpp"
#include "model/ServerMessageGame.hpp"
#include <ctime>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

int run(const string& host, int port, const string& token, const unordered_map<string, string>& params) {
    auto tcpStream = TcpStream(host, port);
    auto inputStream = getInputStream(&tcpStream);
    auto outputStream = getOutputStream(&tcpStream);
    outputStream->write(token);
    outputStream->flush();
    MyStrategy myStrategy(params);
    Debug debug(*outputStream);
    auto begin = clock();
    int tick = 0;
    while (true) {
        auto message = ServerMessageGame::readFrom(*inputStream);
        const auto& playerView = message.playerView;
        if (!playerView.has_value()) break;
        unordered_map<int, UnitAction> actions;
        for (const Unit& unit : playerView->game.world.units) {
            if (unit.playerId == playerView->myId) {
                actions.emplace(unit.id, myStrategy.getAction(unit, playerView->game, debug));
            }
        }
        PlayerMessageGame::ActionMessage(Versioned(actions)).writeTo(*outputStream);
        outputStream->flush();
        if (++tick % 100 == 0) {
            auto end = clock();
#ifndef LOCAL
            cerr << "time at " << tick << ": " << (end - begin) * 1.0 / CLOCKS_PER_SEC << endl;
#endif
            begin = end;
        }
    }
    return tick;
}

int main(int argc, char *argv[]) {
    unordered_map<string, string> params;
    string host = "";
    int port = -1;
    string token = "";
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg[0] == '-') {
            params[arg] = "";
        } else if (host.empty()) {
            host = arg;
        } else if (port < 0) {
            port = stoi(arg);
        } else if (token.empty()) {
            token = arg;
        } else {
            throw runtime_error("Unexpected argument: " + arg);
        }
    }

    if (host.empty()) host = "127.0.0.1";
    if (port < 0) port = 31001;
    if (token.empty()) token = "0000000000000000";

    auto begin = clock();
    auto ticks = run(host, port, token, params);
    auto end = clock();
    auto outputTime = params.find("--time") != params.end();
#ifndef LOCAL
    outputTime = true;
#endif
    if (outputTime) {
        auto time = (end - begin) * 1.0 / CLOCKS_PER_SEC;
        cerr << (time * 1000.0 / max(ticks, 1)) << "ms per tick (" << ticks << " ticks, " << time << "s)" << endl;
    }

    return 0;
}
