#ifndef OPCODES_HPP
#define OPCODES_HPP


enum Opcode {

    // protocol headings
    PRTCL_NUL   = 0x00, // null
    PRTCL_SOH   = 0x01, // start of heading
    PRTCL_STX   = 0x02, // start of text
    PRTCL_ETX   = 0x03, // end of text
    PRTCL_EOT   = 0x04, // end of transmission
    PRTCL_ENQ   = 0x05, // enquiry
    PRTCL_ACK   = 0x06, // acknowledge
    PRTCL_DLE   = 0x07, // data link escape

    // user control
    USR_NAME    = 0x08, // user name
    USR_MVFR    = 0x09, // user move from
    USR_MVTO    = 0x0A, // user move to
    USR_GMOV    = 0x0B, // user game over
    USR_AGAIN   = 0x0C, // user play again
    USR_EXIT    = 0x0D, // user exit
    USR_RECN    = 0x0E, // user reconnect request
    USR_KICK    = 0x0F, // user kick
};


#endif
