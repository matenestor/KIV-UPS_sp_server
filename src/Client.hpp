#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <array>
#include <sstream>


enum State {
    Waiting      = 'w',
    Playing      = 'p',
    Disconnected = 'd'
};


class Client {
private:
    /** Socket client is connected to. */
    u_int socket;
    /** Room where player is located. (0 == lobby) */
    int id_room;
    /** Player"s nick sent by the player. */
    std::string nick;
    /** Player's state during connection. */
    State state;

public:
    Client(const std::string&, u_int);

    // setters
    void setState(State s);

    // getters
    [[nodiscard]] u_int getSocket() const;
    [[nodiscard]] int getIdRoom() const;
    [[nodiscard]] const std::string& getNick() const;
    [[nodiscard]] State getState() const;

    // printers
    std::string toString() const;
};

#endif
