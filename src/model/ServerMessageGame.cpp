#include "ServerMessageGame.hpp"

ServerMessageGame::ServerMessageGame() { }
ServerMessageGame::ServerMessageGame(std::shared_ptr<PlayerView> playerView) : playerView(playerView) { }
ServerMessageGame ServerMessageGame::readFrom(InputStream& stream) {
    ServerMessageGame result;
    if (stream.readBool()) {
        result.playerView = std::shared_ptr<PlayerView>(new PlayerView());
        *result.playerView = PlayerView::readFrom(stream);
    } else {
        result.playerView = std::shared_ptr<PlayerView>();
    }
    return result;
}
