#include <iostream>
#include <unistd.h>

#include "../system/Logger.hpp"
#include "ClientManager.hpp"
#include "Server.hpp"


// ---------- CONSTRUCTORS & DESTRUCTORS





/******************************************************************************
 *
 *
 *
 */
ClientManager::ClientManager() {
    this->clients = std::vector<Client>();
    this->cli_connected = 0;
    this->cli_disconnected = 0;
    this->cli_reconnected = 0;
}





// ---------- PRIVATE METHODS




int ClientManager::processRequest(Request& request) {
    int processed = 0;

    // loop over every data in queue
    while (!request.empty()) {
        // TODO
        logger->debug("dato [%s] in requst", request.front().c_str());
        request.pop();
    }

    return processed;
}





// ---------- PUBLIC METHODS





/******************************************************************************
 *
 *
 *
 */
int ClientManager::process(const int& client_num, ClientData& data) {
    int processed = 0;

    Request queue = Request();
    std::string subdata{};
    std::smatch match_sd, match_kv;

    // loop over every data in queue
    while (!data.empty()) {
        // parse every subdata from data [^,]+
        while (regex_search(data.front(), match_sd, Protocol::rgx_subdata)) {
            subdata = match_sd.str();

            // parse every key-value from subdata [^:]+
            while (regex_search(subdata, match_kv, Protocol::rgx_key_value)) {
                // insert it to queue request vector
                queue.emplace(match_kv.str());
                subdata = match_kv.suffix();
            }

            data.front() = match_sd.suffix();
        }

        data.pop();

        // finally process client's request
        if (this->processRequest(queue) != 0) {
            // if request couldn't be processed, stop and set return value to failure
            // eg. if client sent message in valid format, but it is not valid in terms
            // of server logic or game logic (sbdy is h4ck1ng w/ t3ln3t..)
            processed = 1;
            // clear queue of request data before leaving
            while (!data.empty()) {
                data.pop();
            }
            break;
        }
    }

    return processed;
}


void ClientManager::createClient(const int& socket) {
    this->clients.emplace_back(socket);
    logger->info("Created new client on socket [%d].", socket);
}


clientsIterator ClientManager::closeClient(clientsIterator& client, const char* reason) {
    close(client->getSocket());
    this->cli_disconnected += 1;

    logger->info("Client on socket [%d] closed [%s], [%s]", client->getSocket(), reason, std::strerror(errno));

    return this->clients.erase(client);
}


void ClientManager::pingClients() {
    int sent = 0;

    // ping all clients and disconnect those, who can't answer immediately
    for (auto itr = this->clients.begin(); itr != this->clients.end(); ) {
        sent = send(itr->getSocket(), Protocol::OP_PING.c_str(), 4, 0);

        if (sent <= 0) {
            itr = this->closeClient(itr, "client not available");
            continue;
        }

        ++itr;
    }
}


Client* ClientManager::findClientBySocket(const int socket) {
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

int ClientManager::getClientSocket(Client* client) const {
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

std::vector<Client>& ClientManager::getVectorOfClients() {
    return this->clients;
}

int& ClientManager::getClientsConnected() {
    return this->cli_connected;
}

int& ClientManager::getClientsDisconnected() {
    return this->cli_disconnected;
}

int& ClientManager::getClientsReconnected() {
    return this->cli_reconnected;
}

// ----- PRINTERS

void ClientManager::prAllClients() const {
    logger->debug("Printing all clients.");

    for (const auto& client : clients) {
        logger->debug(client.toString().c_str());
    }
}
