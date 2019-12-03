#include "Debug.hpp"
#include "MyStrategy.hpp"
#include "TcpStream.hpp"
#include "model/PlayerMessageGame.hpp"
#include "model/ServerMessageGame.hpp"
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
        for (const Unit& unit : playerView->game.units) {
            if (unit.playerId == playerView->myId) {
                actions.emplace(unit.id, myStrategy.getAction(unit, playerView->game, debug));
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
    run(port, move(params));
    return 0;
}
