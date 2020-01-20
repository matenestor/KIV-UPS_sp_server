#ifndef ROOM_HNEFATAFL_HPP
#define ROOM_HNEFATAFL_HPP

#include <string>
#include <array>


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
    std::string onTurn;
    /** Player standing by. */
    std::string onStand;
    /** Black attacker player. */
    std::string black;
    /** White defending player. */
    std::string white;

    /** Position on playfield to move from. */
    int xFrom, yFrom;
    /** Position on playfield to move on. */
    int xTo, yTo;

    /** Playfield. */
    Playfield<SIZE> pf{};

    /** Parse playfield coordinated from string. */
    void parseMove(const std::string&);

    /** Check if requested move is valid */
    bool isValidMove();
    /** Check if received coordinated are within playfield. */
    bool isWithinPf();
    /** Check if move is orthogonal. */
    bool isOrthogonal();
    /** Check if path of the stone is free of other stones. */
    bool isFreePath();

    /** Move pieces on playfield. */
    void move();

    /** Check playfield status after move was made. */
    void checkMove();
    /** Check warrior stones captures. */
    void checkCaptureWarrior(const Field&);
    /** Check if black stone is surrounded. */
    bool isSurroundedBlack(const Field&, const Field&);
    /** Check if white stone is surrounded. */
    bool isSurroundedWhite(const Field&, const Field&);
    /** Check King stone capture. */
    bool isCapturedKing();
    /** Check if King stone is surrounded. */
    bool isSurroundedKing(const int&, const int&);

    /** Swaps player on turn with player on stand. */
    void swapPlayers();

public:

    RoomHnefatafl(const int&, const std::string&, const std::string&);

    /** Process requested move of played. */
    bool processMove(const std::string&);

    // getters
    [[nodiscard]] const int& getRoomId() const;
    [[nodiscard]] const GameState& getGameStatus() const;
    [[nodiscard]] std::string getPlayerOnTurn() const;
    [[nodiscard]] std::string getPlayerOnStand() const;
    [[nodiscard]] std::string getPlayfieldString() const;

};


#endif
