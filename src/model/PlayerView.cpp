#include "PlayerView.hpp"

PlayerView::PlayerView() { }
PlayerView::PlayerView(int myId, Game game) : myId(myId), game(game) { }
PlayerView PlayerView::readFrom(InputStream& stream) {
    PlayerView result;
    result.myId = stream.readInt();
    result.game = Game::readFrom(stream);
    return result;
}
