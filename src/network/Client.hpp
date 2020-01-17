#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>


enum State {
    New,
    Waiting,
    PlayingTurn,
    PlayingStandby,
    Pinged,
    Lost,
    Disconnected
};


class Client {
private:
    /** Count of pings during long inaccessibility -- duration. */
    constexpr static const int LONG_PING = 5;

    /** Counter of long inaccessibility pings. */
    int cntr_pings;
    /** Socket client is connected to. */
    int socket;
    /** Room where player is located. (0 == lobby) */
    int id_room;
    /** Player"s nick sent by the player. */
    std::string nick;
    /** Player's state during connection. */
    State state;
    /** Store last client's state after pinging. */
    State stateLast;

public:
    Client(const int&);

    /** Decreases counter during long inaccessibility. */
    void decreaseInaccessCount();
    /** Resets counter of long inaccessibility */
    void resetInaccessCount();

    // setters
    void setNick(const std::string&);
    void setState(State s);

    // getters
    [[nodiscard]] const int& getSocket() const;
    [[nodiscard]] const int& getIdRoom() const;
    [[nodiscard]] const std::string& getNick() const;
    [[nodiscard]] State getState() const;
    [[nodiscard]] State getStateLast() const;
    [[nodiscard]] const int& getInaccessCount() const;

    // printers
    [[nodiscard]] std::string toStringState() const;
    [[nodiscard]] std::string toString() const;
};

#endif
