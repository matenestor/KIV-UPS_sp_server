#include "RoomHnefatafl.hpp"
#include "../system/Logger.hpp"


// ---------- CONSTRUCTORS & DESTRUCTORS





RoomHnefatafl::RoomHnefatafl(const int& id, const std::string& pB, const std::string& pW) {
//    logger->debug("HNEF black [%s] white [%s]", pB->getNick().c_str(), pW->getNick().c_str());

    // init room id and state
    this->roomId = id;
    this->gameState = Playing;

    // black player starts the game
    this->onTurn = pB;
    this->onStand = pW;

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
    std::swap(this->onStand, this->onTurn);
}





// ---------- PUBLIC METHODS





bool RoomHnefatafl::move(const std::string& coordinates) {
    bool moved = false;

    // TODO
    logger->debug("Player [%s] moves to [%s].", this->onTurn.c_str(), coordinates.c_str());
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

std::string RoomHnefatafl::getPlayerOnTurn() const {
    return this->onTurn;
}

std::string RoomHnefatafl::getPlayerOnStand() const {
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
