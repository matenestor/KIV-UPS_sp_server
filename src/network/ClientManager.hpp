#ifndef CLIENTMANAGER_HPP
#define CLIENTMANAGER_HPP

#include "../game/Lobby.hpp"

#include "Client.hpp"
#include "protocol.hpp"


using clientsIterator = std::vector<Client>::iterator;

class ClientManager {
private:

    /** Vector of clients. */
    std::vector<Client> clients;

    /** Lobby takes care of client's sessions. */
    Lobby lobby;

    // connected, disconnected and reconnected clients
    int cli_connected;
    int cli_disconnected;
    int cli_reconnected;

    /** Total sent bytes. ClientManager is only sending. */
    int bytesSend;

    /** Route parsed client's request. */
    int routeRequest(Client&, request&);

    // requests
    int requestConnect(Client&, State, const std::string&);
    int requestMove(Client&, State, const std::string&);
    int requestLeave(Client&, State);
    int requestPing(Client&, State);
    int requestPong(Client&);
    int requestChat(Client&, const std::string&);

public:

    ClientManager();

    /** Process received message for current client. */
    int process(Client&, clientData&);
    /** Create new client connection. */
    void createClient(const int&);
    /** Erase client from vector. */
    clientsIterator eraseClient(clientsIterator& client);
    /** Erase longest disconnected client from vector. */
    void eraseLongestDisconnectedClient();

    /** Send message to client. */
    int sendToClient(Client&, const std::string&);

    /** Find connected client in private vector by nick. */
    clientsIterator findClientByNick(const std::string&);

    /** Finds out, if there are some disconnected clients in vector. */
    bool isDisconnectedClient();
    /** Finds out, if there are some clients with given socket. */
    bool isClientWithSocket(const int&);

    // getters
    [[nodiscard]] int getCountClients() const;
    [[nodiscard]] const int& getCountConnected() const;
    [[nodiscard]] const int& getCountDisconnected() const;
    [[nodiscard]] const int& getCountReconnected() const;
    [[nodiscard]] const int& getBytesSend() const;

    /** Access to private vector of clients. */
    std::vector<Client>& getVectorOfClients();

    // setters
    void setClientState(clientsIterator&);

    // printers
    void prAllClients() const;
};

#endif
