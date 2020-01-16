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

    int sock = client.getSocket();
    State state = client.getState();

    // loop over every data in queue
    while (!rqst.empty()) {
        state = client.getState();

        // get first in queue
        key = rqst.front();

        logger->debug("Processing KEY [%s] of client on socket [%d] with state [%s].",
                        key.c_str(), sock, client.toStringState().c_str());

        // connection request
        if (key == Protocol::CC_CONN && state == New) {
            // key is ok
            rqst.pop();
            // take its value
            value = rqst.front();

            logger->debug("Processing VALUE [%s] of client on socket [%d] with state [%s].",
                            value.c_str(), sock, client.toStringState().c_str());

            client.setNick(value);
            client.setState(Waiting);
            // TODO move client to Lobby
        }
        // reconnection request
        else if (key == Protocol::CC_RECN && (state == Lost || state == Pinged)) {
            // TODO reconnect
        }
        // move in game request
        else if (key == Protocol::CC_MOVE && state == PlayingTurn) {
            // TODO move in client's GameHnefatafl -> swap states of players (if both Playing*)
        }
        // leave the game request
        else if (key == Protocol::CC_LEAV && (state == PlayingTurn || state == PlayingStandby)) {
            // TODO notify oppoent about client Leaving, move opponent to Lobby, and quit their game
        }
        // ping enquiry
        else if (key == Protocol::OP_PING && state != Lost) {
            this->sendToClient(client, Protocol::OP_PONG);
        }
        // pong acknowledge
        else if (key == Protocol::OP_PONG && state == Pinged) {
            client.setState(client.getStateLast());
        }
        // violation of server logic leads to disconnection of client
        else {
            processed = -1;
            break;
        }

        // go to next request element in queue
        rqst.pop();
    }

    return processed;
}





// ---------- PUBLIC METHODS





/******************************************************************************
 *
 *  Takes request from client and parses individual elements.
 *  Eg. from "c:nick" it makes "c" and "nick".
 *  Then sends this for individual processing, which if fails,
 *  breaks the loop and -1 is returned, else 0, when success.
 *
 */
int ClientManager::process(Client& client, clientData& data) {
    int processed = 0;

    request rqst = request();
    std::smatch match;

    // loop over every data in rqst queue {...}
    while (!data.empty()) {
        // parse every key-value from data R("[^:]+")
        while (regex_search(data.front(), match, Protocol::rgx_key_value)) {
            // insert it to rqst queue
            rqst.emplace(match.str());
            // process next part of string in data
            data.front() = match.suffix();
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


/******************************************************************************
 *
 *  Creates client in vector of clients.
 *  note: This method MUST be called only from Server::acceptConnection() in sake of consistency!
 *  Because Server would then access socket, which is not opened, when created elsewhere.
 *
 */
void ClientManager::createClient(const int& socket) {
    this->cli_connected += 1;

    this->clients.emplace_back(socket);
}


/******************************************************************************
 *
 *  Erases client from vector of clients.
 *  note: This method MUST be called only from Server::closeConnection() in sake of consistency!
 *  Because Server also has to clear file descriptor of client's socket.
 *
 */
clientsIterator ClientManager::eraseClient(clientsIterator& client) {
    this->cli_disconnected += 1;

    // TODO if client's state is Playing* or stateLast was Playing*,
    //  notify opponent about Disconnetion, move opponent to Lobby, and quit game

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

    int msg_len = msg.length();
    // + 1 for null terminating char
    char buff[msg_len + 1];
    char* p_buff = buff;

    // fill buffer with the message
    strcpy(buff, msg.c_str());

    int sent = 0;
    int sent_total = 0;
    int failed_send_count = 3;

    logger->debug("Sending message [%s] to client on socket [%d].", buff, client.getSocket());

    // send the message
    while (msg_len > 0 && failed_send_count > 0) {
        sent = send(client.getSocket(), p_buff, msg_len * sizeof(char), 0);

        if (sent > 0) {
            sent_total += sent;
            p_buff += sent;
            msg_len -= sent;
        }
        else {
            --failed_send_count;
        }
    }

    // increment even when send() was not finished
    this->bytesSend += sent_total;

    // if was unable to send message to client 3 times, mark client as Lost
    if (failed_send_count == 0) {
        client.setState(Lost);
        sent_total = -1;
    }

    return sent_total;
}


clientsIterator ClientManager::findClientBySocket(const int socket) {
    auto wanted = this->clients.end();

    for (auto cli = clients.begin(); cli != clients.end(); ++cli) {
        if (cli->getSocket() == socket) {
            wanted = cli;
            break;
        }
    }

    return wanted;
}


clientsIterator ClientManager::findClientByNick(const std::string& nick) {
    auto wanted = this->clients.end();

    for (auto cli = clients.begin(); cli != clients.end(); ++cli) {
        if (cli->getNick() == nick) {
            wanted = cli;
            break;
        }
    }

    return wanted;
}


// ----- GETTERS


int ClientManager::getCountClients() const {
    return this->clients.size();
}

int& ClientManager::getCountConnected() {
    return this->cli_connected;
}

int& ClientManager::getCountDisconnected() {
    return this->cli_disconnected;
}

int& ClientManager::getCountReconnected() {
    return this->cli_reconnected;
}

int& ClientManager::getBytesSend() {
    return this->bytesSend;
}

/******************************************************************************
 *
 * 	Get access to vector of clients, so Server is able to update them.
 *
 */
std::vector<Client>& ClientManager::getVectorOfClients() {
    return this->clients;
}


// ----- PRINTERS


void ClientManager::prAllClients() const {
    logger->debug("--- Printing all clients. ---");

    for (const auto& client : clients) {
        logger->debug(client.toString().c_str());
    }

    logger->debug("--- Printing all clients. --- DONE");
}
