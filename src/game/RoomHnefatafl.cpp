#include "RoomHnefatafl.hpp"
#include "../system/Logger.hpp"


// ---------- CONSTRUCTORS & DESTRUCTORS





RoomHnefatafl::RoomHnefatafl(const int& id, clientsIterator& pB, clientsIterator& pW) {
    logger->debug("HNEF black [%s] white [%s]", pB->getNick().c_str(), pW->getNick().c_str());

    // init room id and state
    this->roomId = id;
    this->gameState = Playing;

    // black player starts the game
    this->onTurn = pB;
    this->onStand = pW;

    // set room Id to clients
    this->onTurn->setRoomId(this->roomId);
    this->onStand->setRoomId(this->roomId);

    // first client will be black, and black starts the game
    this->onTurn->setState(PlayingOnTurn);
    this->onStand->setState(PlayingOnStand);

    // start structure of playfield
    this->pf[0]  = {F_Escape, F_Empty, F_Empty, S_Black, S_Black, S_Black, S_Black, S_Black, F_Empty, F_Empty, F_Escape};
    this->pf[1]  = {F_Empty,  F_Empty, F_Empty, F_Empty, F_Empty, S_Black, F_Empty, F_Empty, F_Empty, F_Empty, F_Empty};
    this->pf[2]  = {F_Empty,  F_Empty, F_Empty, F_Empty, F_Empty, F_Empty, F_Empty, F_Empty, F_Empty, F_Empty, F_Empty};
    this->pf[3]  = {S_Black,  F_Empty, F_Empty, F_Empty, F_Empty, S_White, F_Empty, F_Empty, F_Empty, F_Empty, S_Black};
    this->pf[4]  = {S_Black,  F_Empty, F_Empty, F_Empty, S_White, S_White, S_White, F_Empty, F_Empty, F_Empty, S_Black};
    this->pf[5]  = {S_Black,  S_Black, F_Empty, S_White, S_White, S_King,  S_White, S_White, F_Empty, S_Black, S_Black};
    this->pf[6]  = {S_Black,  F_Empty, F_Empty, F_Empty, S_White, S_White, S_White, F_Empty, F_Empty, F_Empty, S_Black};
    this->pf[7]  = {S_Black,  F_Empty, F_Empty, F_Empty, F_Empty, S_White, F_Empty, F_Empty, F_Empty, F_Empty, S_Black};
    this->pf[8]  = {F_Empty,  F_Empty, F_Empty, F_Empty, F_Empty, F_Empty, F_Empty, F_Empty, F_Empty, F_Empty, F_Empty};
    this->pf[9]  = {F_Empty,  F_Empty, F_Empty, F_Empty, F_Empty, S_Black, F_Empty, F_Empty, F_Empty, F_Empty, F_Empty};
    this->pf[10] = {F_Escape, F_Empty, F_Empty, S_Black, S_Black, S_Black, S_Black, S_Black, F_Empty, F_Empty, F_Escape};
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


void RoomHnefatafl::reassignPlayerOnTurn(clientsIterator& client) {
    this->onTurn = client;
}


void RoomHnefatafl::reassignPlayerOnStand(clientsIterator& client) {
    this->onStand = client;
}





// ----- GETTERS


const int& RoomHnefatafl::getRoomId() const {
    return this->roomId;
}

const GameState& RoomHnefatafl::getGameStatus() const {
    return this->gameState;
}

clientsIterator& RoomHnefatafl::getPlayerOnTurn() {
    return this->onTurn;
}

clientsIterator& RoomHnefatafl::getPlayerOnStand() {
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
