#ifndef CLIENTMANAGER_HPP
#define CLIENTMANAGER_HPP


#include <memory>
#include <vector>
#include "Client.hpp"
#include "Lobby.hpp"
#include "GameRoom.hpp"

class ClientManager {
private:
    /** Vector of clients. */
    std::vector<Client> clients;

    /** Lobby for waiting wait. */
    Lobby lobby;
//    std::unique_ptr<Lobby> lobby;
    /** GameRoom takes care of playing wait. */
    GameRoom gameRoom;
//    std::unique_ptr<GameRoom> gameRoom;

public:
    ClientManager();

    /** Create new client connection. */
    void createClient(const std::string&, const int&);
    /** Remove connected player. */
    void removeClient(Client*);
    /** Find connected client in private vector by socket. */
    Client* findClientBySocket(u_int);
    /** Find connected client in private vector by nick. */
    Client* findClientByNick(const std::string&);

    // setters
    void setClientState(Client*, State);

    // getters
    /** Get clients count */
    [[nodiscard]] int getClientsCount() const;
    u_int getClientSocket(Client*) const;
    int getClientIdRoom(Client*) const;
    const std::string& getClientNick(Client*) const;
    State getClientState(Client*) const;

    // printers
    void toString() const;
};

#endif
