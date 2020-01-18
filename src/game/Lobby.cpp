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





void Lobby::createRoom(Client& client1, Client& client2) {
    this->roomsTotal += 1;
    this->games.emplace_back(this->roomsTotal, client1, client2);
}


void Lobby::destroyRoom(const int& id) {
    this->games.erase(this->getRoomById(id));
}


bool Lobby::moveInRoom(const int& id, const std::string& coordinates) {
    auto room = this->getRoomById(id);
    bool moved = room->move(coordinates);

    return moved;
}


// ----- GETTERS


const int& Lobby::getRoomsTotal() const {
    return this->roomsTotal;
}

const GameState& Lobby::getRoomStatus(const int& id) {
    return this->getRoomById(id)->getGameStatus();
}

Client* Lobby::getPlayerOnTurn(const int& id) {
    return this->getRoomById(id)->getPlayerOnTurn();
}

Client* Lobby::getPlayerOnStand(const int& id) {
    return this->getRoomById(id)->getPlayerOnStand();
}

Client* Lobby::getOponnentOf(const Client& client) {
    // get room where client is
    auto room = this->getRoomById(client.getRoomId());

    // get other client in room as opponent
    return client == *room->getPlayerOnTurn() ? room->getPlayerOnStand() : room->getPlayerOnTurn();
}
