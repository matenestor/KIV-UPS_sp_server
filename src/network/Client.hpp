#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>


enum State {
    New            = 'n',
    Waiting        = 'w',
    PlayingTurn    = 't',
    PlayingStandby = 's',
    Disconnected   = 'd'
};


class Client {
private:
    /** Socket client is connected to. */
    int socket;
    /** Room where player is located. (0 == lobby) */
    int id_room;
    /** Player"s nick sent by the player. */
    std::string nick;
    /** Player's state during connection. */
    State state;

public:
    Client(const int&);

    // setters
    void setNick(const std::string&);
    void setState(State s);

    // getters
    [[nodiscard]] int getSocket() const;
    [[nodiscard]] int getIdRoom() const;
    [[nodiscard]] const std::string& getNick() const;
    [[nodiscard]] State getState() const;

    // printers
    [[nodiscard]] std::string toString() const;
};

#endif
