#include <sstream>

#include "Client.hpp"


// ---------- CONSTRUCTORS & DESTRUCTORS





/******************************************************************************
 *
 *
 *
 */
Client::Client(const int& s) {
    this->cntr_pings = LONG_PING;
    this->socket = s;
    this->id_room = 0;
    this->state = New;
    this->stateLast = New;
}





// ---------- PRIVATE METHODS










// ---------- PUBLIC METHODS


void Client::decreaseInaccessCount() {
    this->cntr_pings -= 1;
}

void Client::resetInaccessCount() {
    this->cntr_pings = LONG_PING;
}


// ----- SETTERS

void Client::setState(State s) {
    if (!(this->state == Pinged || this->state == Lost || this->state == Disconnected)) {
        this->stateLast = this->state;
    }
    this->state = s;
}

void Client::setNick(const std::string& n) {
    this->nick = n;
}

// ----- GETTERS

const int& Client::getSocket() const {
    return this->socket;
}

const int& Client::getIdRoom() const {
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

const int& Client::getInaccessCount() const {
    return this->cntr_pings;
}

// ----- PRINTERS

std::string Client::toStringState() const {
    std::string state_str;

    switch (this->state) {
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
        case Lost:
            state_str = "lost";
            break;
        case Disconnected:
            state_str = "disconnected";
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
