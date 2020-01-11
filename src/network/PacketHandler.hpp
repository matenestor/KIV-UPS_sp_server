#ifndef PACKETHANDLER_HPP
#define PACKETHANDLER_HPP

#include "protocol.hpp"


class PacketHandler {
public:
    /** Check if received message is according to protocol. */
    int isValidFormat(const std::string& msg);
    /** Parse message. */
    void parseMsg(std::string, ClientData&);
};


#endif
