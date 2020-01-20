#include "../system/Logger.hpp"
#include "RoomHnefatafl.hpp"


// ---------- CONSTRUCTORS & DESTRUCTORS





RoomHnefatafl::RoomHnefatafl(const int& id, const std::string& pB, const std::string& pW) {
    // init room id and state
    this->roomId = id;
    this->gameState = Playing;

    // black player starts the game
    this->onTurn = pB;
    this->onStand = pW;

    // save player's nick for whole game, who is black and who is white
    this->black = pB;
    this->white = pW;

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

    logger->info("Game started with clients [%s] as black and [%s] as white. Room id [%d].", pB.c_str(), pW.c_str(), id);
}





// ---------- PRIVATE METHODS





// ----- MOVE CHECKERS


bool RoomHnefatafl::isWithinPf() {
    return     xFrom >= 0 && xFrom < SIZE
            && yFrom >= 0 && yFrom < SIZE
            && xTo >= 0   && xTo < SIZE
            && yTo >= 0   && yTo < SIZE;
}


bool RoomHnefatafl::isMoveable() {
    return this->isOrthogonal() && this->isFreePath();
}


bool RoomHnefatafl::isOrthogonal() {
    return xFrom == xTo || yFrom == yTo;
}


bool RoomHnefatafl::isFreePath() {
    int i;
    bool freePath = true;

    // horizontal move
    if (xFrom == xTo) {
        for (i = xFrom; i < xTo; ++i) {
            if (pf[yFrom][i] != F_Escape || pf[yFrom][i] != F_Throne) {
                freePath = false;
            }
        }
    }
    // vertical move
    else if (yFrom == yTo) {
        for (i = yFrom; i < yTo; ++i) {
            if (pf[i][xFrom] != F_Escape || pf[i][xFrom] != F_Throne) {
                freePath = false;
            }
        }
    }

    return freePath;
}


// ----- GAME LOGIC


void RoomHnefatafl::parseMove(const std::string& coorStr) {
    // parse move coordinates message
    this->xFrom = std::stoi(coorStr.substr(0, 2));
    this->yFrom = std::stoi(coorStr.substr(2, 2));
    this->xTo = std::stoi(coorStr.substr(4, 2));
    this->yTo = std::stoi(coorStr.substr(6, 2));
}


bool RoomHnefatafl::isValidMove() {
    bool valid = false;

    // check if all coordinated are within playfield
    if (this->isWithinPf()) {
        Field from = pf[yFrom][xFrom];
        Field to = pf[yTo][xTo];

        // black on move
        if (onTurn == black && from == S_Black) {
            // black warrior can move only on empty fields
            if (to == F_Empty) {
                valid = this->isMoveable();
            }
        }
            // white on move, moving with warrior stone
        else if (onTurn == white && from == S_White) {
            // white warrior can move only on empty fields
            if (to == F_Empty) {
                valid = this->isMoveable();
            }
        }
            // white on move, moving with King stone
        else if (onTurn == white && from == S_King) {
            // King can move on every field
            if (to == F_Empty || to == F_Throne || to == F_Escape) {
                valid = this->isMoveable();
            }
        }
    }

    return valid;
}


void RoomHnefatafl::move() {

}


void RoomHnefatafl::checkMove() {

}


void RoomHnefatafl::swapPlayers() {
    std::swap(this->onStand, this->onTurn);
}





// ---------- PUBLIC METHODS





bool RoomHnefatafl::processMove(const std::string& coorStr) {
    bool moved = false;

    // parse coordinates in numbers
    this->parseMove(coorStr);

    // check if move is valid
    if (this->isValidMove()) {
        logger->trace("Player [%s] moves to [%s].", this->onTurn.c_str(), coorStr.c_str());

        // then move pieces
        this->move();
        // and swap players
        this->swapPlayers();
        moved = true;
    }

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
