#include "RoomHnefatafl.hpp"
#include "../system/Logger.hpp"


// ---------- CONSTRUCTORS & DESTRUCTORS





RoomHnefatafl::RoomHnefatafl(const int& id, Client& pB, Client& pW) {
    logger->debug("HNEF black [%s] white [%s]", pB.getNick().c_str(), pW.getNick().c_str());

    this->roomId = id;
    this->gameState = Playing;

    // black players start the game
    this->onTurn = &pB;
    this->onStand = &pW;
}





// ---------- PRIVATE METHODS





void RoomHnefatafl::swapPlayers() {
    this->onTurn->setState(PlayingOnStand);
    this->onStand->setState(PlayingOnTurn);
    std::swap(this->onStand, this->onTurn);
}





// ---------- PUBLIC METHODS





bool RoomHnefatafl::move(const std::string& coordinates) {
    bool moved = false;

    // TODO
    logger->debug("Player [%s] moves to [%s].", this->onTurn->getNick().c_str(), coordinates.c_str());
    this->swapPlayers();
    moved = true;

    return moved;
}





// ----- GETTERS


const int& RoomHnefatafl::getRoomId() const {
    return this->roomId;
}

const GameState& RoomHnefatafl::getGameStatus() const {
    return this->gameState;
}

Client* RoomHnefatafl::getPlayerOnTurn() const {
    return this->onTurn;
}

Client* RoomHnefatafl::getPlayerOnStand() const {
    return this->onStand;
}

std::string RoomHnefatafl::getPlayfieldString() const {
    std::string pf_str;

    for (const auto& row : pf) {
        for (const auto& field : row) {
            pf_str += std::to_string(field);
        }
    }

    return pf_str;
}
