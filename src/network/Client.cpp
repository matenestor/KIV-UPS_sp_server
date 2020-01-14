#include <sstream>

#include "Client.hpp"


// ---------- CONSTRUCTORS & DESTRUCTORS





/******************************************************************************
 *
 *
 *
 */
Client::Client(const int& s) {
    this->socket = s;
    this->id_room = 0;
    this->state = New;
}





// ---------- PRIVATE METHODS










// ---------- PUBLIC METHODS





// ----- SETTERS

void Client::setState(State s) {
    this->state = s;
}

void Client::setStateLast(State s) {
    this->stateLast = s;
}

void Client::setNick(const std::string& n) {
    this->nick = n;
}

// ----- GETTERS

int Client::getSocket() const {
    return this->socket;
}

int Client::getIdRoom() const {
    return this->id_room;
}

const std::string& Client::getNick() const {
    return this->nick;
}

State Client::getState() const {
    return this->state;
}

State Client::getStateLast() const {
    return this->stateLast;
}

// ----- PRINTERS

std::string Client::toStringState() const {
    std::string state_str;
    State state = this->getState();

    switch (state) {
        case New:
            state_str = "new";
            break;
        case Waiting:
            state_str = "waiting";
            break;
        case PlayingTurn:
            state_str = "playing, on turn";
            break;
        case PlayingStandby:
            state_str = "playing, standby";
            break;
        case Pinged:
            state_str = "pinged";
            break;
        case ToDisconnect:
            state_str = "to disconnect";
            break;
        case Lost:
            state_str = "lost";
            break;
        default:
            // never should get here
            state_str = "unknown";
    }

    return state_str;
}

std::string Client::toString() const {
    std::stringstream out;

    out << "socket ["     << this->socket
        << "], nick ["    << this->nick
        << "], state ["   << this->toStringState()
        << "], id_room [" << this->id_room
        << "]";

    return out.str();
}
