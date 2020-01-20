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

        logger->trace("KEY [%s] socket [%d] nick [%s] state [%s].", key.c_str(), client.getSocket(), client.getNick().c_str(), client.toStringState().c_str());

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
        // response, that client understands, what server told
        else if (key == Protocol::CC_OK) {
            // just to have response to everything
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
void ClientManager::handleReconnection(Client& client, clientsIterator& clientOtherIpaddr, const std::string& nick) {
    State clientOtherIpaddrState = clientOtherIpaddr->getState();

    // note: client == clientOtherIpaddr for `if` and `else if`
    //  in `else`, there really have been created new instance, so copying values of old one,
    //  and setting pointer to the new one in game, if client is Playing, is required

    // possible state, when client application finds out, that it is without internet
    // for less than one server ping period, so it sends a connect request, but server did not marked
    // the client as Pinged... or somebody is again sending messages with telnet
    if (clientOtherIpaddrState == Waiting || clientOtherIpaddrState == PlayingOnTurn || clientOtherIpaddrState == PlayingOnStand) {
        this->sendToClient(client, Protocol::SC_NICK_USED);
    }
        // short inaccessibility reconnection (without stealing from expired instance)
    else if (clientOtherIpaddrState == Pinged || clientOtherIpaddrState == Lost) {
        client.setState(client.getStateLast());

        // client was in Lobby -- send message about being back in Lobby
        if (client.getRoomId() == 0) {
            this->sendToClient(client, this->composeMsgInLobbyRecn());
        }
        // client was in game -- send message about being back in game
        else {
            // and send game status
            this->sendToClient(client, this->composeMsgInGameRecn(client));
            // inform opponent
            this->sendToOpponentOf(client, Protocol::SC_OPN_RECN);
        }

        this->cli_reconnected += 1;
        logger->info("Client [%s] on socket [%d] in room id [%d] reconnected.", client.getNick().c_str(), client.getSocket(), client.getRoomId());
    }
        // long inaccessibility reconnection -- state Disconnected (with stealing from expired instance)
    else {
        // set nick as before disconnections
        client.setNick(nick);
        // set last state as before disconnection
        client.setState(clientOtherIpaddr->getStateLast());
        // set room id as before disconnection
        client.setRoomId(clientOtherIpaddr->getRoomId());

        // reset inaccessibility ping count
        client.resetInaccessCount();

        // this instance will not be used anymore, so set its nick to nothing
        clientOtherIpaddr->setNick("");
        // and set erase flag
        clientOtherIpaddr->setFlagToErase(true);

        // client was in Lobby -- send message about being back in Lobby
        if (client.getRoomId() == 0) {
            this->sendToClient(client, this->composeMsgInLobbyRecn());
        }
        else {
            // and send game status
            this->sendToClient(client, this->composeMsgInGameRecn(client));
            // inform opponent
            this->sendToOpponentOf(client, Protocol::SC_OPN_RECN);
        }

        this->cli_reconnected += 1;
        logger->info("Client [%s] on socket [%d] in room id [%d] reconnected.", client.getNick().c_str(), client.getSocket(), client.getRoomId());
    }
}


/******************************************************************************
 *
 *  If there does not exist any client in vector and current state of client
 *  is either New or Pinged or Lost or Disconnected, then the method sets new nick
 *  and state Waiting to new client. If state is neither of these, client is hacking server
 *  and -1 is returned.
 *  If there exists some client in vector with same name and IP is different, then some other client
 *  chose same name and if notified, that it is already used. If IP is same, then
 *  method for handling reconnection is called.
 *  After success 0 is returned, else -1.
 *
 */
int ClientManager::requestConnect(Client& client, const std::string& nick, State state) {
    logger->trace("REQUEST connect VALUE [%s] socket [%d] nick [%s] state [%s].", nick.c_str(), client.getSocket(), client.getNick().c_str(), client.toStringState().c_str());

    int rv = 0;

    auto clientNone = this->clients.end();
    // is there some client with this nick already?
    auto clientOtherNick = this->findClientByNick(nick);
    // is there some client with this ip address already?
    auto clientOtherIpaddr = this->findClientByNickAndIp(nick, client.getIpAddr());

    // no client with this nick exists -- new client
    if (clientOtherNick == clientNone) {
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
            // set also state last, because it is somehow possible to get name without changing `State` properly...
            client.setStateLast(Waiting);
            this->sendToClient(client, Protocol::SC_RESP_CONN);
            this->sendToClient(client, Protocol::SC_IN_LOBBY);
        }
        else {
            rv = -1;
        }
    }
    // some client already has this nick
    else {

        // note: client == clientOtherNick

        // client does not have same ip address -- distant client
        if (clientOtherIpaddr == clientNone) {
            this->sendToClient(client, Protocol::SC_NICK_USED);
        }
        // client has also same ip address -- local client
        else {
            this->handleReconnection(client, clientOtherIpaddr, nick);
        }
    }

    return rv;
}


int ClientManager::requestMove(Client& client, const std::string& coordinates) {
    logger->trace("REQUEST move VALUE [%s] socket [%d] nick [%s] state [%s].", coordinates.c_str(), client.getSocket(), client.getNick().c_str(), client.toStringState().c_str());

    int rv = 0;
    int roomId = client.getRoomId();

    // make move in room, where is client, who made this request
    bool moved = this->lobby.moveInRoom(roomId, coordinates);

    if (moved) {
        // get clients in changed room (they already have swapped nicks)
        std::string nickOnTurn = this->lobby.getNickofPlayerOnTurn(roomId);
        std::string nickOnStand = this->lobby.getNickofPlayerOnStand(roomId);
        auto onTurn = this->findClientByNick(nickOnTurn);
        auto onStand = this->findClientByNick(nickOnStand);

        // update their states (this is here, because i can't have any link in class RoomHnefatafl
        // to the clients, except for their nicks, because of iterator magic

        // check if opponent (now is on turn) is Pinged/Lost/Disconnected
        if (onTurn->getState() == Pinged || onTurn->getState() == Lost || onTurn->getState() == Disconnected) {
            // if yes, set last state, in order to keep consistency
            onTurn->setStateLast(PlayingOnTurn);
        }
        else {
            // else set actual state
            onTurn->setState(PlayingOnTurn);
        }
        // update also state of client, who just moved
        onStand->setState(PlayingOnStand);

        // after successful move, inform requesting client and the opponent, where client moved
        this->sendToClient(*onStand, Protocol::SC_MV_VALID);
        this->sendToClient(*onTurn, Protocol::SC_OPN_MOVE + Protocol::OP_INI + coordinates);

        // when game is over, send clients to lobby and destroy their room
        if (this->lobby.getRoomStatus(roomId) == Gameover) {
            // if the game is over, the winner did last move, so looser is now on turn
            this->sendToClient(*onTurn, Protocol::SC_GO_LOSS);
            this->sendToClient(*onStand, Protocol::SC_GO_WIN);

            // finally destroy the finished game room
            this->lobby.destroyRoom(roomId, *onTurn, *onStand);
        }
    }

    rv = moved ? 0 : -1;

    return rv;
}


int ClientManager::requestLeave(Client& client) {
    // notify opponent about client Leaving and move them to Lobby
    this->sendToOpponentOf(client, Protocol::SC_OPN_LEAVE);
    // destroy their game, because one player does not want to play anymore
    auto opponent = this->findClientByNick(this->lobby.getOpponentOf(client));
    this->lobby.destroyRoom(client.getRoomId(), client, *opponent);

    return 0;
}


int ClientManager::requestPing(Client& client, State state) {
    logger->trace("PING request socket [%d] nick [%s] state [%s].", client.getSocket(), client.getNick().c_str(), client.toStringState().c_str());

    if (state == Pinged || state == Lost) {
        client.setState(client.getStateLast());
    }
    this->sendToClient(client, Protocol::OP_PONG);

    return 0;
}


int ClientManager::requestPong(Client& client) {
    logger->trace("PONG request socket [%d] nick [%s] state [%s].", client.getSocket(), client.getNick().c_str(), client.toStringState().c_str());

    client.setState(client.getStateLast());

    return 0;
}


int ClientManager::requestChat(Client& client, const std::string& _msg) {
    std::string msg = Protocol::OP_CHAT + Protocol::OP_INI + _msg;
    this->sendToOpponentOf(client, msg);

    return 0;
}


void ClientManager::startGame(const int& id, Client& cli1, Client& cli2) {
    // set room Id to clients
    cli1.setRoomId(id);
    cli2.setRoomId(id);

    // first client will be black, and black starts the game
    cli1.setState(PlayingOnTurn);
    cli2.setState(PlayingOnStand);

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


std::string ClientManager::composeMsgInGameRecn(Client& client) {
    // {rr,ig,ty,op:onick,pf:0..9}
    return Protocol::SC_RESP_RECN + Protocol::OP_SEP
           + Protocol::SC_IN_GAME + Protocol::OP_SEP
           + (client.getState() == PlayingOnTurn ? Protocol::SC_TURN_YOU : Protocol::SC_TURN_OPN) + Protocol::OP_SEP
           + Protocol::SC_OPN_NAME + Protocol::OP_INI + this->lobby.getOpponentOf(client) + Protocol::OP_SEP
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
    if (!client->getNick().empty()) {

        // before erasing check if client is in game.. if yes, destroy the game
        if (client->getRoomId() != 0 && (client->getStateLast() == PlayingOnTurn || client->getStateLast() == PlayingOnStand)) {
            // get opponent of client, who is going to be erased
            auto opponent = this->findClientByNick(this->lobby.getOpponentOf(*client));

            // if opponent is not also disconnected, send the message
            if (opponent->getState() != Disconnected) {
                // send message to opponent, that the client can't reconnect anymore
                this->sendToClient(*opponent, Protocol::SC_OPN_GONE);
                this->sendToClient(*client, Protocol::SC_IN_LOBBY);
            }

            // destroy their room
            this->lobby.destroyRoom(client->getRoomId(), *client, *opponent);
        }

        logger->info("Client [%s] completely disconnected.", client->getNick().c_str());
    }

    // finally erase client, who have been disconnected for long time
    return this->clients.erase(client);
}


void ClientManager::eraseLongestDisconnectedClient() {
    // TO-DO longterm: how to write this with <algorithm>

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

    if (client.getSocket() < 0) {
        logger->warning("Sending SKIPPED, message [%s] to client [%s] on socket [%d].", buff, client.getNick().c_str(), client.getSocket());
        failed_send_count = 0;
    }
    else {
        logger->trace("Sending message [%s] to client [%s] on socket [%d].", buff, client.getNick().c_str(), client.getSocket());
    }

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


void ClientManager::sendToOpponentOf(Client& client, const std::string& msg) {
    // get nick of opponent
    std::string nick_opponent = this->lobby.getOpponentOf(client);
    // find instance of opponent
    auto opponent = this->findClientByNick(nick_opponent);

    // never should get here, because when instance of client is erased, the game room is destroyed
    if (opponent == this->clients.end()) {
        logger->error("Cannot send message to opponent, who doesn't exist.");
    }
    // this will not send message to client who is disconnected
    else if (opponent->getState() == Disconnected) {
        logger->warning("Cannot send message to opponent [%s], who is disconnected.", opponent->getNick().c_str());
    }
    // send message to opponent
    else {
        this->sendToClient(*opponent, msg);
    }
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


//clientsIterator ClientManager::findClientByIp(const std::string& ip) {
//    auto wanted = this->clients.end();
//
//    for (auto cli = clients.begin(); cli != clients.end(); ++cli) {
//        if (cli->getIpAddr() == ip) {
//            wanted = cli;
//            break;
//        }
//    }
//
//    return wanted;
//}


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
    int roomId = 0;

    for (auto cli1 = this->clients.begin(); cli1 != this->clients.end(); ++cli1) {
        // first Waiting client found
        if (cli1->getState() == Waiting) {

            for (auto cli2 = cli1 + 1; cli2 != this->clients.end(); ++cli2) {
                // second Waiting client found
                if (cli2->getState() == Waiting) {
                    // create game for them
                    roomId = this->lobby.createRoom(cli1->getNick(), cli2->getNick());
                    // initialize new room
                    this->startGame(roomId, *cli1, *cli2);

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

std::string ClientManager::toStringAllClients() const {
    std::stringstream ss;

    ss << "\n--- Printing all clients. ---\n";

    for (const auto& client : clients) {
        ss << client.toString();
    }

    ss << "--- Printing all clients. --- DONE" << std::endl;

    return ss.str();
}
