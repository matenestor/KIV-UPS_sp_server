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
    constexpr static const int LONG_PING = 2;

    /** Counter of long inaccessibility pings. */
    int cntrPings;

    /** IP address of client. */
    std::string ipAddress;
    /** Socket client is connected to. */
    int socketNum;

    /** Room where player is located. (0 == lobby) */
    int roomId;
    /** Player"s nick sent by the player. */
    std::string nick;
    /** Player's state during connection. */
    State state;
    /** Store last client's state after pinging. */
    State stateLast;

public:

    Client(const std::string&, const int&);

    /** Decreases counter during long inaccessibility. */
    void decreaseInaccessCount();
    /** Resets counter of long inaccessibility */
    void resetInaccessCount();

    // getters
    [[nodiscard]] const std::string& getIpAddr() const;
    [[nodiscard]] const int& getSocket() const;
    [[nodiscard]] const int& getRoomId() const;
    [[nodiscard]] const std::string& getNick() const;
    [[nodiscard]] State getState() const;
    [[nodiscard]] State getStateLast() const;
    [[nodiscard]] const int& getInaccessCount() const;

    // setters
    void setSocket(const int&);
    void setRoomId(const int&);
    void setState(State s);
    void setStateLast(State s);
    void setNick(const std::string&);

    // printers
    [[nodiscard]] std::string toStringState() const;
    [[nodiscard]] std::string toString() const;
};

#endif
