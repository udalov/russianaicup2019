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

void run(int port, unordered_map<string, string>&& params) {
    auto tcpStream = TcpStream("127.0.0.1", port);
    auto inputStream = getInputStream(&tcpStream);
    auto outputStream = getOutputStream(&tcpStream);
    outputStream->write(string("0000000000000000"));
    outputStream->flush();
    MyStrategy myStrategy(move(params));
    Debug debug(*outputStream);
    while (true) {
        auto message = ServerMessageGame::readFrom(*inputStream);
        const auto& playerView = message.playerView;
        if (!playerView) break;
        unordered_map<int, UnitAction> actions;
        for (const Unit& unit : playerView->game.world.units) {
            if (unit.playerId == playerView->myId) {
                actions.emplace(unit.id, myStrategy.getAction(unit.id, playerView->game, debug));
            }
        }
        PlayerMessageGame::ActionMessage(actions).writeTo(*outputStream);
        outputStream->flush();
    }
}

int main(int argc, char *argv[]) {
    unordered_map<string, string> params;
    int port = 31001;
    for (int i = 1; i < argc; i++) {
        auto arg = string(argv[i]);
        if (arg[0] != '-') {
            port = stoi(arg);
        } else {
            params[arg] = "";
        }
    }

    auto begin = clock();
    run(port, move(params));
    auto end = clock();
    if (params.find("--time") != params.end()) {
        cerr << (end - begin) * 1.0 / CLOCKS_PER_SEC << "s" << endl;
    }

    return 0;
}
