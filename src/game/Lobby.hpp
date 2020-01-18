#ifndef LOBBY_HPP
#define LOBBY_HPP

#include <vector>

#include "RoomHnefatafl.hpp"


class Client;

using roomsIterator = std::vector<RoomHnefatafl>::iterator;

class Lobby {
private:

    /** Current ongoing games. */
    std::vector<RoomHnefatafl> games;

    /** Count of rooms ever created. */
    int roomsTotal;

    /** Get iterator to room with given id. */
    roomsIterator getRoomById(const int&);

public:
    Lobby();

    /** Creates a room with new game. */
    void createRoom(Client&, Client&);
    /** Destroys a room with finished game. */
    void destroyRoom(const int&);
    /** Send coordinated to room with given id. */
    bool moveInRoom(const int&, const std::string&);

    // getters
    [[nodiscard]] const int& getRoomsTotal() const;
    [[nodiscard]] const GameState& getRoomStatus(const int&);
    Client* getPlayerOnTurn(const int&);
    Client* getPlayerOnStand(const int&);

};


#endif
