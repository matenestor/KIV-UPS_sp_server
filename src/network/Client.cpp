#include <sstream>

#include "Client.hpp"


// ---------- CONSTRUCTORS & DESTRUCTORS





/******************************************************************************
 *
 *
 *
 */
Client::Client(const std::string& n, const int& s) {
    this->socket = s;
    this->id_room = 0;
    this->nick = n;
    this->state = New;
}





// ---------- PRIVATE METHODS










// ---------- PUBLIC METHODS





// ----- SETTERS

void Client::setState(State s) {
    this->state = s;
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

// ----- PRINTERS

std::string Client::toString() const {
    std::stringstream out;

    out << "socket ["     << this->socket
        << "], nick ["    << this->nick
        << "], state ["   << static_cast<char>(this->state)
        << "], id_room [" << this->id_room
        << "]";

    return out.str();
}
