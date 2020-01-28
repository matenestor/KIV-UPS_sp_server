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


bool RoomHnefatafl::isOrthogonal() {
    return xFrom == xTo || yFrom == yTo;
}


bool RoomHnefatafl::isFreePath() {
    int i, shift;
    bool freePath = true;

    // horizontal move
    if (yFrom == yTo) {
        // move right or left
        shift = xFrom < xTo ? 1 : -1;

        // field next to the one, which is being moved from
        i = xFrom + shift;
        // while not on end position
        while (i != xTo) {
            // if place is not empty and throne, it has to be something else,
            // so path is not free, thus invalid move was received by client
            if (pf[yFrom][i] != F_Empty && pf[yFrom][i] != F_Throne) {
                freePath = false;
                break;
            }
            i += shift;
        }
    }

    // vertical move
    else if (xFrom == xTo) {
        // move down or up
        shift = yFrom < yTo ? 1 : -1;

        i = yFrom + shift;

        while (i != yTo) {
            if (pf[i][xFrom] != F_Empty && pf[i][xFrom] != F_Throne) {
                freePath = false;
                break;
            }
            i += shift;
        }
    }

    return freePath;
}


// ----- GAME LOGIC


void RoomHnefatafl::parseMove(const std::string& coorStr) {
    // parse move coordinates message according to the protocol
    this->xFrom = std::stoi(coorStr.substr(0, 2));
    this->yFrom = std::stoi(coorStr.substr(2, 2));
    this->xTo   = std::stoi(coorStr.substr(4, 2));
    this->yTo   = std::stoi(coorStr.substr(6, 2));
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
                valid = this->isOrthogonal() && this->isFreePath();
            }
        }
            // white on move, moving with warrior stone
        else if (onTurn == white && from == S_White) {
            // white warrior can move only on empty fields
            if (to == F_Empty) {
                valid = this->isOrthogonal() && this->isFreePath();
            }
        }
            // white on move, moving with King stone
        else if (onTurn == white && from == S_King) {
            // King can move on every field
            if (to == F_Empty || to == F_Throne || to == F_Escape) {
                valid = this->isOrthogonal() && this->isFreePath();
            }
        }
    }

    return valid;
}


void RoomHnefatafl::move() {
    // move stone to wanted position
    pf[yTo][xTo] = pf[yFrom][xFrom];

    // move with King from Throne, which is in middle
    if (yFrom == (int) SIZE/2 && xFrom == (int) SIZE/2) {
        pf[yFrom][xFrom] = F_Throne;
    }
    // move from regular field
    else {
        pf[yFrom][xFrom] = F_Empty;
    }
}


void RoomHnefatafl::checkCaptures() {
    // king was moved
    if (pf[yTo][xTo] == S_King) {
        // King is corner, which is Escape field -- White wins
        if ((xTo == 0 || xTo == SIZE-1) && (yTo == 0 || yTo == SIZE-1)) {
            this->gameState = Gameover;
        }
    }
    // white defender was moved
    else if (pf[yTo][xTo] == S_White) {
        this->checkCaptureWarrior(&RoomHnefatafl::isSurroundedBlack);
    }
    // black attacker was moved
    else if (pf[yTo][xTo] == S_Black) {
        this->checkCaptureWarrior(&RoomHnefatafl::isSurroundedWhite);

        // black captured the white King
        if (this->isCapturedKing()) {
            this->gameState = Gameover;
        }
    }
}


void RoomHnefatafl::checkCaptureWarrior(bool (RoomHnefatafl::*surrounded)(const Field&, const Field&)) {
    // rather first check if the position is really not out of bounds,
    // do not combine there two conditions (same goes for other conditions here)
    if ((xTo + 2) < SIZE) {
        // if stone is captured
        if ((this->*surrounded)(pf[yTo][xTo + 1], pf[yTo][xTo + 2])) {
            // capture it
            pf[yTo][xTo + 1] = F_Empty;
        }
    }
    if ((xTo - 2) >= 0) {
        if ((this->*surrounded)(pf[yTo][xTo - 1], pf[yTo][xTo - 2])) {
            pf[yTo][xTo - 1] = F_Empty;
        }
    }
    if ((yTo + 2) < SIZE) {
        if ((this->*surrounded)(pf[yTo + 1][xTo], pf[yTo + 2][xTo])) {
            pf[yTo + 1][xTo] = F_Empty;
        }
    }
    if ((yTo - 2) >= 0) {
        if ((this->*surrounded)(pf[yTo - 1][xTo], pf[yTo - 2][xTo])) {
            pf[yTo - 1][xTo] = F_Empty;
        }
    }
}


bool RoomHnefatafl::isSurroundedBlack(const Field& fieldAdjacent, const Field& fieldAlly) {
    // warrior is between two allied stones, or next to the warrior
    // is Throne, or Kings's escape field -> black warrior is captured
    return fieldAdjacent == S_Black && (fieldAlly == S_White || fieldAlly == F_Throne || fieldAlly == F_Escape);
}


bool RoomHnefatafl::isSurroundedWhite(const Field& fieldAdjacent, const Field& fieldAlly) {
    // warrior is between two allied stones, or next to the warrior
    // is Throne, or Kings's escape field -> white warrior is captured
    // Black player may also capture, when the ally field is Throne or Escape
    return fieldAdjacent == S_White && (fieldAlly == S_Black || fieldAlly == F_Throne || fieldAlly == F_Escape);
}


bool RoomHnefatafl::isCapturedKing() {
    bool captured = false;

    // on right of theblack stone, there is a King
    if ((xTo + 1) < SIZE) {
        if (pf[yTo][xTo + 1] == S_King) {
            captured = this->isSurroundedKing(this->yTo, this->xTo + 1);
        }
    }

    // on left of the black stone, there is a King
    if ((xTo - 1) >= 0) {
        if (pf[yTo][xTo - 1] == S_King) {
            captured = this->isSurroundedKing(this->yTo, this->xTo - 1);
        }
    }

    // down of the black stone, there is a King
    if ((yTo + 1) < SIZE) {
        if (pf[yTo + 1][xTo] == S_King) {
            captured = this->isSurroundedKing(this->yTo + 1, this->xTo);
        }
    }

    // up of the black stone, there is a King
    if ((yTo - 1) >= 0) {
        if (pf[yTo - 1][xTo] == S_King) {
            captured = this->isSurroundedKing(this->yTo - 1, this->xTo);
        }
    }

    return captured;
}


bool RoomHnefatafl::isSurroundedKing(const int& y, const int& x) {
    // find at least one side, where King has free space to revert this bool
    bool surrounded = true;

    // right
    // if this condition is false, then King is on border of playfield
    // and still may be surrounded, so check also other directions
    if ((x + 1) < SIZE) {
        // there is some field next to the King and if it is F_Empty or S_White, then King is not captured
        // note: if the adjacent field is F_Escape or F_Throne, King can be captured
        if (pf[y][x + 1] == F_Empty || pf[y][x + 1] == S_White) {
            surrounded = false;
        }
    }

    // left
    if ((x - 1) >= 0) {
        if (pf[y][x - 1] == F_Empty || pf[y][x - 1] == S_White) {
            surrounded = false;
        }
    }

    // down
    if ((y + 1) < SIZE) {
        if (pf[y + 1][x] == F_Empty || pf[y + 1][x] == S_White) {
            surrounded = false;
        }
    }

    // up
    if ((y - 1) >= 0) {
        if (pf[y - 1][x] == F_Empty || pf[y - 1][x] == S_White) {
            surrounded = false;
        }
    }

    return surrounded;
}


// ----- PLAYERS SWAP


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
        // check situation after move
        this->checkCaptures();
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
