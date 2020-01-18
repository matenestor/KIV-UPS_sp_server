#ifndef ROOM_HNEFATAFL_HPP
#define ROOM_HNEFATAFL_HPP

#include <string>

#include "../network/Client.hpp"


enum GameState {
    Playing,
    Gameover
};

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
    int pf[SIZE][SIZE];

    /** Swaps player on turn with player on stand. */
    void swapPlayers();

public:

    RoomHnefatafl(const int&, Client&, Client&);

    /** TODO */
    bool move(const std::string&);

    // getters
    [[nodiscard]] const int& getRoomId() const;
    [[nodiscard]] const GameState& getGameStatus() const;
    [[nodiscard]] Client* getPlayerOnTurn() const;
    [[nodiscard]] Client* getPlayerOnStand() const;
    [[nodiscard]] std::string getPlayfieldString() const;

};


#endif
