#ifndef CLIENTMANAGER_HPP
#define CLIENTMANAGER_HPP

#include "../game/Lobby.hpp"
#include "../game/GameRoom.hpp"

#include "Client.hpp"
#include "protocol.hpp"


using clientsIterator = std::vector<Client>::iterator;

class ClientManager {
private:

    /** Vector of clients. */
    std::vector<Client> clients;

    /** Lobby takes care of waiting clients. */
    Lobby lobby;
    /** GameRoom takes care of playing clients. */
    GameRoom gameRoom;

    // connected, disconnected and reconnected clients
    int cli_connected;
    int cli_disconnected;
    int cli_reconnected;

    /** Total sent bytes. ClientManager is only sending. */
    int bytesSend;

    /** Process parsed client's request. */
    int processRequest(Client&, request&);

public:

    ClientManager();

    /** Process received message for current client. */
    int process(Client&, clientData&);
    /** Create new client connection. */
    void createClient(const int&);
    /** Erase client from vector. */
    clientsIterator eraseClient(clientsIterator& client);

    /** Send message to client. */
    int sendToClient(Client&, const std::string&);

    /** Find connected client in private vector by socket. */
    clientsIterator findClientBySocket(int);
    /** Find connected client in private vector by nick. */
    clientsIterator findClientByNick(const std::string&);

    // getters
    [[nodiscard]] int getCountClients() const;
    int& getCountConnected();
    int& getCountDisconnected();
    int& getCountReconnected();
    int& getBytesSend();

    /** Access to private vector of clients. */
    std::vector<Client>& getVectorOfClients();

    // printers
    void prAllClients() const;
};

#endif
