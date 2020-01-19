#ifndef ROOM_HNEFATAFL_HPP
#define ROOM_HNEFATAFL_HPP

#include <string>
#include <array>

#include "../network/Client.hpp"


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
    Client* onTurn;
    /** Player standing by. */
    Client* onStand;

    /** Playfield. */
    Playfield<SIZE> pf;

    /** Swaps player on turn with player on stand. */
    void swapPlayers();

public:

    RoomHnefatafl(const int&, Client&, Client&);

    /** TODO */
    bool move(const std::string&);

    /** Reassign client pointer, who is on turn, to new instance after reconnection. */
    void reassignPlayerOnTurn(Client&);
    /** Reassign client pointer, who is on stand, to new instance after reconnection. */
    void reassignPlayerOnStand(Client&);

    // getters
    [[nodiscard]] const int& getRoomId() const;
    [[nodiscard]] const GameState& getGameStatus() const;
    [[nodiscard]] Client* getPlayerOnTurn() const;
    [[nodiscard]] Client* getPlayerOnStand() const;
    [[nodiscard]] std::string getPlayfieldString() const;

};


#endif
