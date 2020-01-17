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
int ClientManager::routeRequest(Client& client, request& rqst) {
    int processed = 0;
    State state;
    std::string key;

    // loop over every data in queue
    while (!rqst.empty()) {
        // get current state of client
        state = client.getState();
        // get first element in queue
        key = rqst.front();

        logger->debug("KEY [%s] socket [%d] nick [%s] state [%s].", key.c_str(), client.getSocket(), client.getNick().c_str(), client.toStringState().c_str());

        // connection request
        if (key == Protocol::CC_CONN) {
            // key is ok
            rqst.pop();

            processed = this->requestConnect(client, state, rqst.front());
        }
        // move in game request
        else if (key == Protocol::CC_MOVE && state == PlayingTurn) {
            // key is ok
            rqst.pop();

            processed = this->requestMove(client, state, rqst.front());
        }
        // leave the game request
        else if (key == Protocol::CC_LEAV && (state == PlayingTurn || state == PlayingStandby)) {
            processed = this->requestLeave(client, state);
        }
        // ping enquiry
        else if (key == Protocol::OP_PING) {
            processed = this->requestPing(client, state);
        }
        // pong acknowledge (pong may still come after short inaccessibility)
        else if (key == Protocol::OP_PONG) {
            processed = this->requestPong(client);
        }
        // chat message
        else if (key == Protocol::OP_CHAT && (state == PlayingTurn || state == PlayingStandby)) {
            // key is ok
            rqst.pop();

            processed = this->requestChat(client, rqst.front());
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


/******************************************************************************
 *
 *  If there does not exist any client in vector and current state of client
 *  is either New or Pinged or Lost or Disconnected, then the method sets new nick
 *  and state Waiting to new client. If state is neither of these, client is hacking server
 *  and -1 is returned.
 *  If there exists some client in vector and current socket is same, like the one of client in vector,
 *  and state is either Pinged or Lost or Disconnected, it means, that the client is reconnecting
 *  and state is set accordingly. If state is neither of these, client is hacking.
 *  After success 0 is returned, else -1.
 *
 */
int ClientManager::requestConnect(Client& client, State state, const std::string& nick) {
    logger->debug("REQUEST connect VALUE [%s] socket [%d] nick [%s] state [%s].", nick.c_str(), client.getSocket(), client.getNick().c_str(), client.toStringState().c_str());

    int rv = 0;

    auto itr_end = this->clients.end();
    auto client_other = this->findClientByNick(nick);

    // no client with this nick exists
    if (client_other == itr_end) {
        // new client without name
        // client.getStateLast() == New reason:
        //  clients with name already are not able to rename themselves
        // state == Pinged reason:
        //  it is possible that connection request will come after creating client and just before
        //  receiving pong request, because time between select and Server::updateClient() is not mutexed..
        // state == Lost, Disconnected reason: (hidden reconnection)
        //  nick, which client chose could be used.. client stays connected for how long it is necessary,
        //  in order to choose different nick and without internet it might lead to being Lost or even Disconnected
        if (client.getStateLast() == New && (state == New || state == Pinged || state == Lost || state == Disconnected)) {
            client.setNick(nick);
            client.setState(Waiting);
            // TODO move client to Lobby
        }
        else {
            rv = -1;
        }
    }
    // some client already has this nick
    else {
        // client, who is this request being processed for, is the same client, that has this name -- reconnect request
        if (client_other->getSocket() == client.getSocket()) {
            if (state == New || state == Pinged || state == Lost || state == Disconnected) {
                client.setState(client.getStateLast());
                client.resetInaccessCount();
                // TODO full reconnect
            }
            // this is rare situation, but still possible
            else {
                logger->info("Client [%s] on socket [%d] with state [%s] probably lost connection "
                             "and tried to reconnect, but was already connected. No problem.",
                             client.toStringState().c_str(), client.getSocket(), client.getState());
            }
        }
        // client chose same nick, like client on different socket -- name already used
        else {
            this->sendToClient(client, Protocol::SC_NICK_USED);
        }
    }

    return rv;
}



int ClientManager::requestMove(Client& client, State state, const std::string& position) {
    logger->debug("REQUEST move VALUE [%s] socket [%d] nick [%s] state [%s].", position.c_str(), client.getSocket(), client.getNick().c_str(), client.toStringState().c_str());

    int rv = 0;

    // TODO move in client's GameHnefatafl -> swap states of players (if both Playing*)

    return rv;
}


int ClientManager::requestLeave(Client& client, State state) {
    int rv = 0;

    // TODO notify oppoent about client Leaving, move opponent to Lobby, and quit their game

    return rv;
}


int ClientManager::requestPing(Client& client, State state) {
    logger->debug("REQUEST ping socket [%d] nick [%s] state [%s].", client.getSocket(), client.getNick().c_str(), client.toStringState().c_str());

    if (state == Pinged || state == Lost) {
        client.setState(client.getStateLast());
    }
    this->sendToClient(client, Protocol::OP_PONG);

    return 0;
}


int ClientManager::requestPong(Client& client) {
    client.setState(client.getStateLast());

    return 0;
}


int ClientManager::requestChat(Client& client, const std::string& message) {
    logger->debug("REQUEST chat VALUE [%s] socket [%d] nick [%s] state [%s].", message.c_str(), client.getSocket(), client.getNick().c_str(), client.toStringState().c_str());

    // TODO reply to opponent

    return 0;
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
        if (this->routeRequest(client, rqst) != 0) {
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
    this->cli_connected += 1;

    this->clients.emplace_back(socket);
}


clientsIterator ClientManager::eraseClient(clientsIterator& client) {
    return this->clients.erase(client);
}


void ClientManager::eraseLongestDisconnectedClient() {
    // TODO longterm: how to write this with <algorithm>

    auto longestDiscCli = this->clients.end();

    // find first disconnected client
    for (auto cli = this->clients.begin(); cli != this->clients.end(); ++cli) {
        if (cli->getState() == Disconnected) {
            longestDiscCli = cli;
            break;
        }
    }

    // check other clients, if they are disconnected for longer time
    for (auto cli = longestDiscCli + 1; cli != clients.end(); ++cli) {
        if (cli->getState() == Disconnected && cli->getInaccessCount() < longestDiscCli->getInaccessCount()) {
            longestDiscCli = cli;
        }
    }

    // always should be true, because it is used after isDisconnectedClient()
    if (longestDiscCli != this->clients.end()) {
        this->clients.erase(longestDiscCli);
    }
}


/******************************************************************************
 *
 * 	On successful send of message returns count of sent bytes, on failure returns -1.
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


bool ClientManager::isDisconnectedClient() {
    bool isDiscCli = false;

    for (auto & client : this->clients) {
        if (client.getState() == Disconnected) {
            isDiscCli = true;
            break;
        }
    }

    return isDiscCli;
}


bool ClientManager::isClientWithSocket(const int& sock) {
    bool isCliWSock = false;

    for (auto & client : this->clients) {
        if (client.getSocket() == sock) {
            isCliWSock = true;
            break;
        }
    }

    return isCliWSock;
}


// ----- GETTERS


int ClientManager::getCountClients() const {
    return this->clients.size();
}

const int& ClientManager::getCountConnected() const {
    return this->cli_connected;
}

const int& ClientManager::getCountDisconnected() const {
    return this->cli_disconnected;
}

const int& ClientManager::getCountReconnected() const {
    return this->cli_reconnected;
}

const int& ClientManager::getBytesSend() const {
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


void ClientManager::setClientState(clientsIterator& client) {
    this->cli_disconnected += 1;
    client->setState(Disconnected);
}


// ----- PRINTERS

void ClientManager::prAllClients() const {
    logger->debug("--- Printing all clients. ---");

    for (const auto& client : clients) {
        logger->debug(client.toString().c_str());
    }

    logger->debug("--- Printing all clients. --- DONE");
}
