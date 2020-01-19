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

            processed = this->requestConnect(client, rqst.front(), state);
        }
        // move in game request
        else if (key == Protocol::CC_MOVE && (state == PlayingOnTurn || state == Pinged)) {
            // key is ok
            rqst.pop();

            processed = this->requestMove(client, rqst.front());
        }
        // leave the game request
        else if (key == Protocol::CC_LEAV && (state == PlayingOnTurn || state == PlayingOnStand || state == Pinged)) {
            processed = this->requestLeave(client);
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
        else if (key == Protocol::OP_CHAT && (state == PlayingOnTurn || state == PlayingOnStand || state == Pinged)) {
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
 * 	IF: client is correctly connected, but somebody tries to connect with same name on same IP
 * 	ELSE IF: client lost connection for a while
 * 	ELSE: client disconnected and doing reconnection
 *
 * 	advice: dont try to understand complete flow, because even i do not
 *
 */
void ClientManager::handleReconnection(Client& client, clientsIterator& client_other_ipaddr, const std::string& nick) {
    State client_other_ipaddr_state = client_other_ipaddr->getState();

    // note: client == client_other_ipaddr for `if` and `else if`
    //  in `else`, there really have been created new instance, so copying values of old one,
    //  and setting pointer to the new one in game, if client is Playing, is required

    // possible state, when client application finds out, that it is without internet
    // for less than one server ping period, so it sends a connect request, but server did not marked
    // the client as Pinged... or somebody is again sending messages with telnet
    if (client_other_ipaddr_state == Waiting || client_other_ipaddr_state == PlayingOnTurn || client_other_ipaddr_state == PlayingOnStand) {
        this->sendToClient(client, Protocol::SC_NICK_USED);
    }
        // short inaccessibility reconnection (without stealing from expired instance)
    else if (client_other_ipaddr_state == Pinged || client_other_ipaddr_state == Lost) {
        client.setState(client.getStateLast());

        // client was in Lobby -- send message about being back in Lobby
        if (client.getRoomId() == 0) {
            this->sendToClient(client, this->composeMsgInLobbyRecn());
        }
            // client was in game -- send message about being back in game
        else {
            this->sendToClient(client, this->composeMsgInGameRecn(client));
        }
    }
        // long inaccessibility reconnection -- state Disconnected (with stealing from expired instance)
    else {
        // set nick as before disconnections
        client.setNick(nick);
        // set last state as before disconnection
        client.setState(client_other_ipaddr->getStateLast());
        // set room id as before disconnection
        client.setRoomId(client_other_ipaddr->getRoomId());

        // reset inaccessibility ping count
        client.resetInaccessCount();

        // this instance will not be used anymore, so set its nick to nothing.. and hope it will get erased safely
        client_other_ipaddr->setNick("");

        // client was in Lobby -- send message about being back in Lobby
        if (client.getRoomId() == 0) {
            this->sendToClient(client, this->composeMsgInLobbyRecn());
        }
        else {
            // if client was not in Lobby, change iterator in the room to this new reconnected instance
            auto toReassign = this->findClientByNick(nick);
            this->lobby.reassignPlayerIterator(toReassign);
            // and send game status
            this->sendToClient(client, this->composeMsgInGameRecn(client));
        }
    }
}


/******************************************************************************
 *
 *  TODO
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
int ClientManager::requestConnect(Client& client, const std::string& nick, State state) {
    logger->debug("REQUEST connect VALUE [%s] socket [%d] nick [%s] state [%s].", nick.c_str(), client.getSocket(), client.getNick().c_str(), client.toStringState().c_str());

    int rv = 0;

    auto client_none = this->clients.end();
    // is there some client with this nick already?
    auto client_other_nick = this->findClientByNick(nick);
    // is there some client with this ip address already?
    auto client_other_ipaddr = this->findClientByNickAndIp(nick, client.getIpAddr());

    // no client with this nick exists -- new client
    if (client_other_nick == client_none) {
        // new client without name
        // client.getStateLast() == New reason:
        //  clients with name already are not able to rename themselves
        // state == Pinged reason:
        //  it is possible that connection request will come after creating client and just before
        //  receiving pong request, because time between select and Server::updateClient() is not mutexed or client might had lost connection
        // state == Lost, Disconnected reason: (hidden reconnection)
        //  nick, which client chose could be used.. client stays connected for how long it is necessary,
        //  in order to choose different nick and without internet it might lead to being Lost or even Disconnected
        if (client.getStateLast() == New && (state == New || state == Pinged || state == Lost || state == Disconnected)) {
            client.setNick(nick);
            client.setState(Waiting);
            this->sendToClient(client, Protocol::SC_RESP_CONN);
            this->sendToClient(client, Protocol::SC_IN_LOBBY);
        }
        else {
            rv = -1;
        }
    }
    // some client already has this nick
    else {

        // note: client == client_other_nick

        // client does not have same ip address -- distant client
        if (client_other_ipaddr == client_none) {
            this->sendToClient(client, Protocol::SC_NICK_USED);
        }
        // client has also same ip address -- local client
        else {
            this->handleReconnection(client, client_other_ipaddr, nick);
        }
    }

    return rv;
}


int ClientManager::requestMove(Client& client, const std::string& coordinates) {
    logger->debug("REQUEST move VALUE [%s] socket [%d] nick [%s] state [%s].", coordinates.c_str(), client.getSocket(), client.getNick().c_str(), client.toStringState().c_str());

    int rv = 0;
    int roomId = client.getRoomId();

    bool moved = this->lobby.moveInRoom(roomId, coordinates);

    if (moved) {
        auto onTurn = this->lobby.getPlayerOnTurn(roomId);
        auto onStand = this->lobby.getPlayerOnStand(roomId);

        // after successful move, the opponent on turn
        this->sendToClient(*onStand, Protocol::SC_MV_VALID);
        this->sendToClient(*onTurn, Protocol::SC_OPN_MOVE + Protocol::OP_INI + coordinates);

        // when game is over, send clients to lobby and destroy their room
        if (this->lobby.getRoomStatus(roomId) == Gameover) {
            // if the game is over, the winner did last move, so looser is now on turn
            this->sendToClient(*onTurn, Protocol::SC_GO_LOSS);
            this->sendToClient(*onStand, Protocol::SC_GO_WIN);

            // finally destroy the finished game room
            this->lobby.destroyRoom(roomId);
        }
    }

    rv = moved ? 0 : -1;

    return rv;
}


int ClientManager::requestLeave(Client& client) {
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


void ClientManager::startGame(Client& cli1, Client& cli2) {
    // send message to players, who just started playing
    this->sendToClient(cli1, this->composeMsgInGame(Protocol::SC_TURN_YOU, cli2.getNick()));
    this->sendToClient(cli2, this->composeMsgInGame(Protocol::SC_TURN_OPN, cli1.getNick()));
}


// ----- COMPOSERS


std::string ClientManager::composeMsgInGame(const std::string& turn, const std::string& nick) {
    // eg. {ig,ty,on:nick12}
    return Protocol::SC_IN_GAME + Protocol::OP_SEP + turn + Protocol::OP_SEP
         + Protocol::SC_OPN_NAME + Protocol::OP_INI + nick;
}


std::string ClientManager::composeMsgInGameRecn(const Client& client) {
    auto clientIterator = this->findClientByNick(client.getNick());

    // {rr,ig,ty,op:onick,pf:0..9}
    return Protocol::SC_RESP_RECN + Protocol::OP_SEP
           + Protocol::SC_IN_GAME + Protocol::OP_SEP
           + (client.getState() == PlayingOnTurn ? Protocol::SC_TURN_YOU : Protocol::SC_TURN_OPN) + Protocol::OP_SEP
           + Protocol::SC_OPN_NAME + Protocol::OP_INI + this->lobby.getOpponentOf(clientIterator)->getNick()
           + Protocol::SC_PLAYFIELD + Protocol::OP_INI + this->lobby.getPlayfieldString(client.getRoomId());
}


std::string ClientManager::composeMsgInLobbyRecn() {
    // {rr,il}
    return Protocol::SC_RESP_RECN + Protocol::OP_SEP + Protocol::SC_IN_LOBBY;
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


void ClientManager::createClient(const std::string& ip, const int& sock) {
    this->cli_connected += 1;

    this->clients.emplace_back(ip, sock);
}


clientsIterator ClientManager::eraseClient(clientsIterator& client) {
    // before erasing check if client is in game.. if yes, destroy the game
    if (client->getStateLast() == PlayingOnTurn || client->getStateLast() == PlayingOnTurn) {
        this->lobby.destroyRoom(client->getRoomId());
    }

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

    // always should be true, because it is used after isDisconnectedClient(),
    // thus there must be client, who is Disconnected
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

    // if was unable to send message to client 3 times
    if (failed_send_count == 0) {
        sent_total = -1;
    }

    return sent_total;
}


void ClientManager::sendToOpponentOf(const clientsIterator& client, const std::string& msg) {
    this->sendToClient(*this->lobby.getOpponentOf(client), msg);
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


clientsIterator ClientManager::findClientByIp(const std::string& ip) {
    auto wanted = this->clients.end();

    for (auto cli = clients.begin(); cli != clients.end(); ++cli) {
        if (cli->getIpAddr() == ip) {
            wanted = cli;
            break;
        }
    }

    return wanted;
}


clientsIterator ClientManager::findClientByNickAndIp(const std::string& nick, const std::string& ip) {
    auto wanted = this->clients.end();

    for (auto cli = clients.begin(); cli != clients.end(); ++cli) {
        if (cli->getNick() == nick && cli->getIpAddr() == ip) {
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


/******************************************************************************
 *
 * 	Finds every two Waiting clients and sends them to play a game in time O(n).
 *
 */
void ClientManager::moveWaitingClientsToPlay() {
    for (auto cli1 = this->clients.begin(); cli1 != this->clients.end(); ++cli1) {
        // first Waiting client found
        if (cli1->getState() == Waiting) {

            for (auto cli2 = cli1 + 1; cli2 != this->clients.end(); ++cli2) {
                // second Waiting client found
                if (cli2->getState() == Waiting) {
                    // create game for them
                    this->lobby.createRoom(cli1, cli2);
                    // initialize new room
                    this->startGame(*cli1, *cli2);

                    // continue searching from position next to second client
                    cli1 = cli2;
                    break;
                }
            }
        }
    }
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

const int& ClientManager::getRoomsTotal() const {
    return this->lobby.getRoomsTotal();
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


void ClientManager::setDisconnected(clientsIterator& client) {
    this->cli_disconnected += 1;
    client->setState(Disconnected);
}


void ClientManager::setBadSocket(clientsIterator& client, const int& badsock) {
    client->setSocket(badsock);
}


// ----- PRINTERS

void ClientManager::prAllClients() const {
    logger->debug("--- Printing all clients. ---");

    for (const auto& client : clients) {
        logger->debug(client.toString().c_str());
    }

    logger->debug("--- Printing all clients. --- DONE");
}
