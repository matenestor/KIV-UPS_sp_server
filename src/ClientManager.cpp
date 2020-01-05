#include <iostream>
#include "ClientManager.hpp"
#include "Logger.hpp"

// ---------- CONSTRUCTORS & DESTRUCTORS





/******************************************************************************
 *
 *
 *
 */
ClientManager::ClientManager() {
    this->clients = std::vector<Client>();
//    this->lobby = std::make_unique<Lobby>();
//    this->gameRoom = std::make_unique<GameRoom>();
}





// ---------- PRIVATE METHODS










// ---------- PUBLIC METHODS




void ClientManager::createClient(const std::string& nick, const int& socket) {
    this->clients.emplace_back(nick, socket);
    logger->info("Created new client [%s] on socket [%d].", nick.c_str(), socket);
}

void ClientManager::removeClient(Client* cli) {
    std::string nick("");
    int socket = -1;

    for (auto itr = this->clients.begin(); itr != this->clients.end(); ++itr) {
        if ((*itr).getSocket() == cli->getSocket()) {
            // save client's data in this scope for logger
            nick = (*itr).getNick();
            socket = (*itr).getSocket();

            // remove client
            clients.erase(itr);

            logger->info("Removed client [%s] on socket [%d].", nick.c_str(), socket);
            break;
        }
    }
}

Client* ClientManager::findClientBySocket(const u_int socket) {
    Client* wanted = nullptr;

    for (auto& cli : this->clients) {
        if (cli.getSocket() == socket) {
            wanted = &cli;
            break;
        }
    }

    return wanted;
}


Client* ClientManager::findClientByNick(const std::string& nick) {
    Client* wanted = nullptr;

    for (auto& cli : this->clients) {
        if (cli.getNick() == nick) {
            wanted = &cli;
            break;
        }
    }

    return wanted;
}

// ----- SETTERS

void ClientManager::setClientState(Client* const client, State state) {
    client->setState(state);
}

// ----- GETTERS

int ClientManager::getClientsCount() const {
    return this->clients.size();
}

u_int ClientManager::getClientSocket(Client* client) const {
    return client->getSocket();
}

int ClientManager::getClientIdRoom(Client* client) const {
    return client->getIdRoom();
}

const std::string& ClientManager::getClientNick(Client* client) const {
    return client->getNick();
}

State ClientManager::getClientState(Client* client) const {
    return client->getState();
}

// ----- PRINTERS

void ClientManager::toString() const {
    logger->debug("Printing all clients.");

    for (const auto& client : clients) {
        logger->debug(client.toString().c_str());
    }
}
