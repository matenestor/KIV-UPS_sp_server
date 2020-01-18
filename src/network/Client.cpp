#include <sstream>

#include "Client.hpp"


// ---------- CONSTRUCTORS & DESTRUCTORS





Client::Client(const int& s) {
    this->cntrPings = LONG_PING;
    this->socket = s;
    this->roomId = 0;
    this->state = New;
    this->stateLast = New;
}





// ---------- PRIVATE METHODS










// ---------- PUBLIC METHODS


void Client::decreaseInaccessCount() {
    this->cntrPings -= 1;
}

void Client::resetInaccessCount() {
    this->cntrPings = LONG_PING;
}


// ----- SETTERS

void Client::setRoomId(const int& id) {
    this->roomId = id;
}

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

const int& Client::getRoomId() const {
    return this->roomId;
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
    return this->cntrPings;
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
        case PlayingOnTurn:
            state_str = "playing -- on turn";
            break;
        case PlayingOnStand:
            state_str = "playing -- on stand";
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
        << "], roomId [" << this->roomId
        << "]";

    return out.str();
}
