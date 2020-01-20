#include "Lobby.hpp"
#include "../system/Logger.hpp"

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





int Lobby::createRoom(const std::string& client1, const std::string& client2) {
    this->roomsTotal += 1;
    this->games.emplace_back(this->roomsTotal, client1, client2);

    return this->roomsTotal;
}


void Lobby::destroyRoom(const int& id, Client& client1, Client& client2) {
    State stateClient1 = client1.getState();
    State stateClient2 = client2.getState();

    // set states of both players to Waiting according to their connection/disconnection
    // client 1
    if (stateClient1 == Pinged || stateClient1 == Lost || stateClient1 == Disconnected) {
        client1.setStateLast(Waiting);
    }
    else {
        client1.setState(Waiting);
    }

    // client 2
    if (stateClient2 == Pinged || stateClient2 == Lost || stateClient2 == Disconnected) {
        client2.setStateLast(Waiting);
    }
    else {
        client2.setState(Waiting);
    }

    // erase room only, when client is not in Lobby
    if (id != 0) {
        this->games.erase(this->getRoomById(id));

        logger->info("Room id [%d] destroyed.", id);
    }

    // set client's room id to Lobby
    client1.setRoomId(0);
    client2.setRoomId(0);
}


bool Lobby::moveInRoom(const int& id, const std::string& coordinates) {
    auto room = this->getRoomById(id);
    bool moved = room->move(coordinates);

    return moved;
}


//void Lobby::reassignPlayerIterator(clientsIterator& client) {
//    auto room = this->getRoomById(client->getRoomId());
//
//    // clients must be compared by Nick
//    if (client->getNick() == room->getPlayerOnTurn()->getNick()) {
//        // set pointer to new instance to player on turn
//        room->reassignPlayerOnTurn(client);
//    }
//    else {
//        // set pointer to new instance to player on stand
//        room->reassignPlayerOnStand(client);
//    }
//}


// ----- GETTERS


std::string Lobby::getOpponentOf(Client& client) {
    // get room where client is
    auto room = this->getRoomById(client.getRoomId());

    // get other client in room as opponent
    return client.getNick() == room->getPlayerOnTurn()
            ? room->getPlayerOnStand()
            : room->getPlayerOnTurn();
}

const int& Lobby::getRoomsTotal() const {
    return this->roomsTotal;
}

const GameState& Lobby::getRoomStatus(const int& id) {
    return this->getRoomById(id)->getGameStatus();
}

std::string Lobby::getNickofPlayerOnTurn(const int& id) {
    return this->getRoomById(id)->getPlayerOnTurn();
}

std::string Lobby::getNickofPlayerOnStand(const int& id) {
    return this->getRoomById(id)->getPlayerOnStand();
}

std::string Lobby::getPlayfieldString(const int& id) {
    return this->getRoomById(id)->getPlayfieldString();
}
