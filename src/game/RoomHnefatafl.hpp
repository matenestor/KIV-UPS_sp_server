#ifndef ROOM_HNEFATAFL_HPP
#define ROOM_HNEFATAFL_HPP

#include <string>
#include <array>
#include <vector>

#include "../network/Client.hpp"


using clientsIterator = std::vector<Client>::iterator;

enum GameState {
    Playing,
    Gameover
};

enum Field {
    F_Empty  = 0,
    F_Throne = 1,
    F_Escape = 2,
    S_Black  = 3,
    S_White  = 4,
    S_King   = 5
};

template <std::size_t N>
using Playfield = std::array<std::array<Field, N>, N>;

class RoomHnefatafl {
private:

    /** Size of playfield. */
    constexpr static const int SIZE = 11;

    /** Id of current room of instance. */
    int roomId;
    /** Flag for determining end of the game. */
    GameState gameState;

    /** Player on turn. */
    clientsIterator onTurn;
    /** Player standing by. */
    clientsIterator onStand;

    /** Playfield. */
    Playfield<SIZE> pf;

    /** Swaps player on turn with player on stand. */
    void swapPlayers();

public:

    RoomHnefatafl(const int&, clientsIterator&, clientsIterator&);

    /** TODO */
    bool move(const std::string&);

    /** Reassign client pointer, who is on turn, to new instance after reconnection. */
    void reassignPlayerOnTurn(clientsIterator&);
    /** Reassign client pointer, who is on stand, to new instance after reconnection. */
    void reassignPlayerOnStand(clientsIterator&);

    // getters
    [[nodiscard]] const int& getRoomId() const;
    [[nodiscard]] const GameState& getGameStatus() const;
    [[nodiscard]] clientsIterator& getPlayerOnTurn();
    [[nodiscard]] clientsIterator& getPlayerOnStand();
    [[nodiscard]] std::string getPlayfieldString() const;

};


#endif
