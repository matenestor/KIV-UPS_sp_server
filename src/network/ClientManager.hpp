#ifndef CLIENTMANAGER_HPP
#define CLIENTMANAGER_HPP

#include <vector>

#include "../game/Lobby.hpp"
#include "Client.hpp"
#include "protocol.hpp"


using clientsIterator = std::vector<Client>::iterator;

class ClientManager {
private:

    /** Lobby takes care of waiting and playing clients. */
    Lobby lobby;

    /** Vector of clients. */
    std::vector<Client> clients;

    // connected, disconnected and reconnected clients
    int cli_connected;
    int cli_disconnected;
    int cli_reconnected;

    /** Total sent bytes. ClientManager is only sending. */
    int bytesSend;

    /** Route parsed client's request. */
    int routeRequest(Client&, request&);

    /** Handle reconnection. */
    void handleReconnection(Client&, clientsIterator&, const std::string&);

    // requests
    int requestConnect(Client&, const std::string&, State);
    int requestMove(Client&, const std::string&);
    int requestLeave(Client&);
    int requestPing(Client&, State);
    int requestPong(Client&);
    int requestChat(Client&, const std::string&);

    /** Sets Id and State to clients, who starts to play.. */
    void startGame(const int&, Client& cli1, Client& cli2);

    /** Compose message, which is send to client, who just entered a game. */
    std::string composeMsgInGame(const std::string&, const std::string&);
    /** Compose message, which is send to client, who have been reconnected to game. */
    std::string composeMsgInGameRecn(Client&);
    /** Compose message, which is send to client, who have been reconnected to lobby. */
    std::string composeMsgInLobbyRecn();

public:

    ClientManager();

    /** Process received message for current client. */
    int process(Client&, clientData&);
    /** Create new client connection. */
    void createClient(const std::string&, const int&);
    /** Erase client from vector. */
    clientsIterator eraseClient(clientsIterator& client);
    /** Erase longest disconnected client from vector. */
    void eraseLongestDisconnectedClient();

    /** Send message to client. */
    int sendToClient(Client&, const std::string&);
    /** Send message to client's opponent, when in game. */
    void sendToOpponentOf(Client&, const std::string&);

    /** Find connected client in private vector by nick. */
    clientsIterator findClientByNick(const std::string&);
//    /** Find connected client in private vector by ip address. */
//    clientsIterator findClientByIp(const std::string&);
    /** Find connected client in private vector by both nick and ip address. */
    clientsIterator findClientByNickAndIp(const std::string&, const std::string&);

    /** Finds out, if there are some disconnected clients in vector. */
    bool isDisconnectedClient();

    /** Checks Waiting clients and tells Lobby to send them to game. */
    void moveWaitingClientsToPlay();

    // getters
    [[nodiscard]] int getCountClients() const;
    [[nodiscard]] const int& getCountConnected() const;
    [[nodiscard]] const int& getCountDisconnected() const;
    [[nodiscard]] const int& getCountReconnected() const;
    [[nodiscard]] const int& getBytesSend() const;
    [[nodiscard]] const int& getRoomsTotal() const;

    /** Access to private list of clients. */
    std::vector<Client>& getVectorOfClients();

    // setters
    void setDisconnected(clientsIterator&);
    void setBadSocket(clientsIterator&, const int&);

    // printers
    void prAllClients() const;
};

#endif
