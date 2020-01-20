#include <sstream>

#include "Client.hpp"


// ---------- CONSTRUCTORS & DESTRUCTORS





Client::Client(const std::string& ip, const int& sock) {
    this->flagToDisconnect = false;
    this->flagToErase = false;
    this->discReason = "none";
    this->cntrPings = LONG_PING;
    this->ipAddress = ip;
    this->socketNum = sock;
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


void Client::setSocket(const int& sock) {
    this->socketNum = sock;
}

void Client::setRoomId(const int& id) {
    this->roomId = id;
}

void Client::setState(State s) {
    if (!(this->state == Pinged || this->state == Lost || this->state == Disconnected)) {
        this->stateLast = this->state;
    }
    this->state = s;
}

void Client::setStateLast(State s) {
    this->stateLast = s;
}

void Client::setNick(const std::string& n) {
    this->nick = n;
}

void Client::setFlagToDisconnect(const bool& value, const std::string& reason) {
    this->flagToDisconnect = value;
    this->discReason = reason;
}

void Client::setFlagToErase(const bool& value) {
    this->flagToErase = value;
}


// ----- GETTERS


const std::string& Client::getIpAddr() const {
    return this->ipAddress;
}

const int& Client::getSocket() const {
    return this->socketNum;
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

const bool& Client::getFlagToDisconnect() const {
    return this->flagToDisconnect;
}

const bool& Client::getFlagToErase() const {
    return this->flagToErase;
}

const char* Client::getReason() const {
    return this->discReason.c_str();
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
            state_str = "on turn";
            break;
        case PlayingOnStand:
            state_str = "on stand";
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

    out << "socket ["     << this->socketNum
        << "], nick ["    << this->nick
        << "], state ["   << this->toStringState()
        << "], roomId ["  << this->roomId
        << "]" << std::endl;

    return out.str();
}
