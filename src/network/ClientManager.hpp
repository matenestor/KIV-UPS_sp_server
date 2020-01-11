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

    /** Process parsed client's request. */
    int processRequest(Request&);

public:
    ClientManager();

    /** Process received message for current client. */
    int process(const int&, ClientData&);

    /** Create new client connection. */
    void createClient(const int&);
    /** Disconnect client with bad socket. */
    clientsIterator closeClient(clientsIterator& client, const char* reason);
    /** Asynchronous pinging clients. */
    void pingClients();
    /** Find connected client in private vector by socket. */
    Client* findClientBySocket(int);
    /** Find connected client in private vector by nick. */
    Client* findClientByNick(const std::string&);

    // setters
    void setClientState(Client*, State);

    // getters
    /** Get clients count */
    [[nodiscard]] int getClientsCount() const;
    int getClientSocket(Client*) const;
    int getClientIdRoom(Client*) const;
    const std::string& getClientNick(Client*) const;
    State getClientState(Client*) const;

    std::vector<Client>& getVectorOfClients();

    int& getClientsConnected();
    int& getClientsDisconnected();
    int& getClientsReconnected();

    // printers
    void prAllClients() const;
};

#endif
