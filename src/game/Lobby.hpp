#ifndef LOBBY_HPP
#define LOBBY_HPP

#include <vector>

#include "../network/Client.hpp"
#include "RoomHnefatafl.hpp"


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
    int createRoom(const std::string&, const std::string&);
    /** Destroys a room with finished game. */
    void destroyRoom(const int&, Client&, Client&);
    /** Send coordinated to room with given id. */
    bool moveInRoom(const int&, const std::string&);

    // getters
    [[nodiscard]] std::string getOpponentOf(Client&);
    [[nodiscard]] const int& getRoomsTotal() const;
    [[nodiscard]] const GameState& getRoomStatus(const int&);
    std::string getNickofPlayerOnTurn(const int&);
    std::string getNickofPlayerOnStand(const int&);
    std::string getPlayfieldString(const int&);

};


#endif
