#ifndef PACKET_HANDLER_HPP
#define PACKET_HANDLER_HPP

#include "protocol.hpp"


// TODO longterm: better architecture, inline vs. namespace vs. in helper class ?

/******************************************************************************
 *
 * 	Check if received message is according to protocol.
 *
 */
inline int isValidFormat(const std::string& msg) {
    // same reason for 1 as false like in Server::serveClient()
    return std::regex_match(msg, Protocol::rgx_valid_format) ? 0 : 1;
}


/******************************************************************************
 *
 * 	Parse message.
 *
 */
inline void parseMsg(std::string msg, clientData& data) {
    std::smatch match;

    // search every brackets with data in message and emplace it to vector
    while (regex_search(msg, match, Protocol::rgx_data)) {
        data.emplace(match.str());
        msg = match.suffix();
    }
}


#endif
