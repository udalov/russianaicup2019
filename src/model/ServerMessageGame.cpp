#include "ServerMessageGame.hpp"

using namespace std;

ServerMessageGame::ServerMessageGame() { }
ServerMessageGame::ServerMessageGame(PlayerView&& playerView) : playerView(playerView) { }
ServerMessageGame ServerMessageGame::readFrom(InputStream& stream) {
    return stream.readBool()
        ? ServerMessageGame(PlayerView::readFrom(stream))
        : ServerMessageGame();
}
