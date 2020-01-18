#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>


enum State {
    New,
    Waiting,
    PlayingOnTurn,
    PlayingOnStand,
    Pinged,
    Lost,
    Disconnected
};


class Client {
private:
    /** Count of pings during long inaccessibility -- duration. */
    constexpr static const int LONG_PING = 5;

    /** Counter of long inaccessibility pings. */
    int cntrPings;
    /** Socket client is connected to. */
    int socket;
    /** Room where player is located. (0 == lobby) */
    int roomId;
    /** Player"s nick sent by the player. */
    std::string nick;
    /** Player's state during connection. */
    State state;
    /** Store last client's state after pinging. */
    State stateLast;

public:

    Client(const int&);

    /** Compare if two clients are same */
    friend bool operator==(const Client&, const Client&);

    /** Decreases counter during long inaccessibility. */
    void decreaseInaccessCount();
    /** Resets counter of long inaccessibility */
    void resetInaccessCount();

    // setters
    void setRoomId(const int&);
    void setNick(const std::string&);
    void setState(State s);

    // getters
    [[nodiscard]] const int& getSocket() const;
    [[nodiscard]] const int& getRoomId() const;
    [[nodiscard]] const std::string& getNick() const;
    [[nodiscard]] State getState() const;
    [[nodiscard]] State getStateLast() const;
    [[nodiscard]] const int& getInaccessCount() const;

    // printers
    [[nodiscard]] std::string toStringState() const;
    [[nodiscard]] std::string toString() const;
};

#endif
