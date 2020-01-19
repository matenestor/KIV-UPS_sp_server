#include "Lobby.hpp"

// ---------- CONSTRUCTORS & DESTRUCTORS





Lobby::Lobby() {
    this->games = std::vector<RoomHnefatafl>();
    this->roomsTotal = 0;
}





// ---------- PRIVATE METHODS







// ----- GETTERS


roomsIterator Lobby::getRoomById(const int& id) {
    auto wanted = this->games.end();

    for (auto itr = this->games.begin(); itr != this->games.end(); ++itr) {
        if (itr->getRoomId() == id) {
            wanted = itr;
            break;
        }
    }

    return wanted;
}





// ---------- PUBLIC METHODS





void Lobby::createRoom(clientsIterator& client1, clientsIterator& client2) {
    this->roomsTotal += 1;
    this->games.emplace_back(this->roomsTotal, client1, client2);
}


void Lobby::destroyRoom(const int& id) {
    auto onTurn = this->getPlayerOnTurn(id);
    auto onStand = this->getPlayerOnTurn(id);

    // set states of both players to Waiting and room id to Lobby"
    if (onTurn->getState() != Disconnected) {
        onTurn->setState(Waiting);
        onTurn->setRoomId(0);
    }
    if (onStand->getState() != Disconnected) {
        onStand->setState(Waiting);
        onStand->setRoomId(0);
    }

    this->games.erase(this->getRoomById(id));
}


bool Lobby::moveInRoom(const int& id, const std::string& coordinates) {
    auto room = this->getRoomById(id);
    bool moved = room->move(coordinates);

    return moved;
}


void Lobby::reassignPlayerIterator(clientsIterator& client) {
    auto room = this->getRoomById(client->getRoomId());

    // clients must be compared by Nick
    if (client->getNick() == room->getPlayerOnTurn()->getNick()) {
        // set pointer to new instance to player on turn
        room->reassignPlayerOnTurn(client);
    }
    else {
        // set pointer to new instance to player on stand
        room->reassignPlayerOnStand(client);
    }
}


// ----- GETTERS


clientsIterator& Lobby::getOpponentOf(const clientsIterator& client) {
    // get room where client is
    auto room = this->getRoomById(client->getRoomId());

    // get other client in room as opponent
    return client->getNick() == room->getPlayerOnTurn()->getNick()
            ? room->getPlayerOnStand()
            : room->getPlayerOnTurn();
}

const int& Lobby::getRoomsTotal() const {
    return this->roomsTotal;
}

const GameState& Lobby::getRoomStatus(const int& id) {
    return this->getRoomById(id)->getGameStatus();
}

clientsIterator& Lobby::getPlayerOnTurn(const int& id) {
    return this->getRoomById(id)->getPlayerOnTurn();
}

clientsIterator& Lobby::getPlayerOnStand(const int& id) {
    return this->getRoomById(id)->getPlayerOnStand();
}

std::string Lobby::getPlayfieldString(const int& id) {
    return this->getRoomById(id)->getPlayfieldString();
}
