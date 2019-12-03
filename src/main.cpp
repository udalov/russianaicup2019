#include "Debug.hpp"
#include "MyStrategy.hpp"
#include "TcpStream.hpp"
#include "model/PlayerMessageGame.hpp"
#include "model/ServerMessageGame.hpp"
#include <string>
#include <unordered_map>

void run(const std::string &host, int port, const std::string &token) {
  auto tcpStream = TcpStream(host, port);
  auto inputStream = getInputStream(&tcpStream);
  auto outputStream = getOutputStream(&tcpStream);
  outputStream->write(token);
  outputStream->flush();
  MyStrategy myStrategy;
  Debug debug(*outputStream);
  while (true) {
    auto message = ServerMessageGame::readFrom(*inputStream);
    const auto& playerView = message.playerView;
    if (!playerView) {
      break;
    }
    std::unordered_map<int, UnitAction> actions;
    for (const Unit &unit : playerView->game.units) {
      if (unit.playerId == playerView->myId) {
        actions.emplace(std::make_pair(
            unit.id,
            myStrategy.getAction(unit, playerView->game, debug)));
      }
    }
    PlayerMessageGame::ActionMessage(actions).writeTo(*outputStream);
    outputStream->flush();
  }
}

int main(int argc, char *argv[]) {
  std::string host = argc < 2 ? "127.0.0.1" : argv[1];
  int port = argc < 3 ? 31001 : atoi(argv[2]);
  std::string token = argc < 4 ? "0000000000000000" : argv[3];
  run(host, port, token);
  return 0;
}
