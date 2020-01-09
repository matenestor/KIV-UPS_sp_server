#ifndef PACKETHANDLER_HPP
#define PACKETHANDLER_HPP


#include "opcodes.hpp"

class PacketHandler {
public:
    int isValidSOT(const char*);
    int isValidEOT(const char*);
};


#endif
