#include <sys/socket.h>
#include <iostream>

#include "../system/Logger.hpp"
#include "ClientManager.hpp"


// ---------- CONSTRUCTORS & DESTRUCTORS





ClientManager::ClientManager() {
    this->clients = std::vector<Client>();

    this->cli_connected = 0;
    this->cli_disconnected = 0;
    this->cli_reconnected = 0;

    this->bytesSend = 0;
}





// ---------- PRIVATE METHODS




/******************************************************************************
 *
 * 	If everything was processed successfully, return 0, else return -1
 * 	eg. when client can't receive actual data according to one's state
 * 	or when invalid move for game is received.
 *
 */
int ClientManager::processRequest(Client& client, request& rqst) {
    int processed = 0;
    std::string key, value;

    // loop over every data in queue
    while (!rqst.empty()) {
        // get first in queue
        key = rqst.front();

        // pong acknowledge
        if (key == Protocol::OP_PONG && client.getState() == Pinged) {
            client.setState(client.getStateLast());
        }

        // connection request
        if (key == Protocol::CC_CONN && client.getState() == New) {
            // key is ok
            rqst.pop();
            // take its value
            value = rqst.front();

            client.setNick(value);
            client.setState(Waiting);
            // TODO move client to Lobby
        }
        // reconnection request
        else if (key == Protocol::CC_RECN
                && (client.getState() == Lost
                    || client.getState() == Pinged)) {
            // TODO reconnect
        }
        // move in game request
        else if (key == Protocol::CC_MOVE && client.getState() == PlayingTurn) {
            // TODO move in client's GameHnefatafl -> swap states of players (if both Playing*)
        }
        // leave the game request
        else if (key == Protocol::CC_LEAV
                && (client.getState() == PlayingTurn
                    || client.getState() == PlayingStandby)) {
            // TODO notify oppoent about client Leaving, move opponent to Lobby, and quit their game
        }
        // disconnect server request
        else if (key == Protocol::CC_DICN) {
            client.setState(ToDisconnect);
            // TODO notify oppoent about client disconnect, move opponent to Lobby, and quit their game
        }
        // ping enquiry
        else if (key == Protocol::OP_PING) {
            this->sendToClient(client, Protocol::OP_PONG);
        }
        // violation of server logic leads to disconnection of client
        else {
            processed = -1;
            break;
        }

        // go to next in queue
        rqst.pop();
    }

    return processed;
}





// ---------- PUBLIC METHODS





/******************************************************************************
 *
 *
 *
 */
int ClientManager::process(Client& client, clientData& data) {
    int processed = 0;

    request rqst = request();
    std::string subdata{};
    std::smatch match_sd, match_kv;

    // loop over every data in rqst queue {...}
    while (!data.empty()) {
        // parse every subdata from data R("[^,]+")
        while (regex_search(data.front(), match_sd, Protocol::rgx_subdata)) {
            subdata = match_sd.str();

            // parse every key-value from subdata R("[^:]+")
            while (regex_search(subdata, match_kv, Protocol::rgx_key_value)) {
                // insert it to rqst queue
                rqst.emplace(match_kv.str());
                subdata = match_kv.suffix();
            }

            data.front() = match_sd.suffix();
        }

        // pop just processed data
        data.pop();

        // finally process client's request
        if (this->processRequest(client, rqst) != 0) {
            // if request couldn't be processed, stop and set return value to -1 (failure)
            // eg. if client sent message in valid format, but it is not valid in terms
            // of server logic or game logic (sbdy is h4ck1ng w/ t3ln3t..)
            processed = -1;
            break;
        }
    }

    return processed;
}


void ClientManager::createClient(const int& socket) {
    this->clients.emplace_back(socket);
    logger->info("Created new client on socket [%d].", socket);
}


/******************************************************************************
 *
 * 	This method MUST be called only from Server class in sake of consistency!
 * 	When some client should be removed from list here, the client only must be
 * 	marked as 'toErase' and Server then disconnects the client.
 *
 * 	TODO longterm: better architecture design
 *
 */
clientsIterator ClientManager::eraseClient(clientsIterator& client, const char* reason) {
    this->cli_disconnected += 1;

    logger->info("Client on socket [%d] closed [%s], [%s]", client->getSocket(), reason, std::strerror(errno));

    return this->clients.erase(client);
}


/******************************************************************************
 *
 * 	On success returns count of sent bytes, on failure returns -1.
 *
 */
int ClientManager::sendToClient(Client& client, const std::string& _msg) {
    // close message to protocol header and footer
    std::string msg = Protocol::OP_SOH + _msg + Protocol::OP_EOT;
    // make C string from msg in buffer
    int msg_len = msg.length();
    char buff[msg_len + 1];
    strcpy(buff, msg.c_str());

    int sent = 0;
    int sent_total = 0;
    int failed_send_count = 3;

    // send the message
    while (sent_total < msg_len && failed_send_count > 0) {
        sent = send(client.getSocket(), buff, msg_len * sizeof(char), 0);

        if (sent > 0) {
            sent_total += sent;
        }
        else {
            --failed_send_count;
        }
    }

    // increment even when send() was not finished
    this->bytesSend += sent_total;

    // if was unable to send message to client 3 times, mark client to disconnect
    if (failed_send_count == 0) {
        client.setState(ToDisconnect);
        sent_total = -1;
    }

    return sent_total;
}


void ClientManager::pingClients() {
    // ping all clients and disconnect those, who can't answer immediately
    for (auto client = this->clients.begin(); client != this->clients.end(); ++client) {
        logger->debug("pinging [%d]", client->getSocket());

        // if client was already pinged and did not respond since, mark one as Lost
        if (client->getState() == Pinged) {
            // TODO if stateLast was Playing*, send massage to opponent about Lost
            client->setState(Lost);
        }
        // if client was Lost and did not respond since, mark one as Lost
        else if (client->getState() == Lost) {
            // TODO if stateLast was Playing*, send massage to opponent about Disconnetion, move opponent to Lobby, and quit game
            client->setState(ToDisconnect);
        }
        // send ping message to client
        if (this->sendToClient(*client, Protocol::OP_PING) > 0) {
            client->setStateLast(client->getState());
            client->setState(Pinged);
        }
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

int& ClientManager::getBytesSend() {
    return this->bytesSend;
}


// ----- PRINTERS


void ClientManager::prAllClients() const {
    logger->debug("--- Printing all clients. ---");

    for (const auto& client : clients) {
        logger->debug(client.toString().c_str());
    }

    logger->debug("--- Printing all clients. --- DONE");
}
