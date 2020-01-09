#ifndef OPCODES_HPP
#define OPCODES_HPP


enum Opcode {

    // operation codes
    OP_NUL          = 0x00, // null
    OP_SOH          = 0x01, // start of header
    OP_STX          = 0x02, // start of text
    OP_ETX          = 0x03, // end of text
    OP_EOT          = 0x04, // end of transmission
    OP_PING         = 0x05, // enquiry
    OP_PONG         = 0x06, // acknowledge

    // client codes
    CC_CONN         = 0x07, // connect request
    CC_RECN         = 0x08, // reconnect request
    CC_NAME         = 0x09, // name
    CC_MOVE         = 0x0A, // move
    CC_LEAV         = 0x0B, // leave game
    CC_EXIT         = 0x0C, // exit connection

    // server codes
    SC_RESP_CONN    = 0x0D, // response connect
    SC_RESP_RECN    = 0x0E, // response reconnect
    SC_RESP_LEAVE   = 0x0F, // response leave
    SC_IN_LOBBY     = 0x10, // client moved to lobby
    SC_IN_GAME      = 0x11, // client moved to game
    SC_MV_VALID     = 0x12, // valid move
    SC_MV_INVALID   = 0x13, // invalid move
    SC_TURN_YOU     = 0x14, // your's turn message
    SC_TURN_OPN     = 0x15, // opponent's turn message
    SC_PLAYFIELD    = 0x16, // playfield
    SC_GO_WIN       = 0x17, // game over win message
    SC_GO_LOSS      = 0x18, // game over loss message
    SC_OPN_NAME     = 0x19, // opponent's name
    SC_OPN_MOVE     = 0x1A, // opponent's move
    SC_OPN_LEAVE    = 0x1B, // opponent left the game
    SC_OPN_EXIT     = 0x1C, // opponent disconnected
    SC_OPN_RECN     = 0x1D, // opponent reconnected
    SC_MANY_PLRS    = 0x1E, // too many players message
    SC_NAME_USED    = 0x1F, // name is alredy used
    SC_KICK         = 0x20, // kick client

};


#endif

/*
 *
 * SOH
 *
 * EOT
 *
 */

