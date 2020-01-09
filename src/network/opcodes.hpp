#ifndef OPCODES_HPP
#define OPCODES_HPP


enum Opcode {

    // operation codes
    OC_NUL   = 0x00, // null
    OC_SOH   = 0x01, // start of heading
    OC_STX   = 0x02, // start of text
    OC_ETX   = 0x03, // end of text
    OC_EOT   = 0x04, // end of transmission
    OC_ENQ   = 0x05, // enquiry
    OC_ACK   = 0x06, // acknowledge
    OC_DLE   = 0x07, // data link escape

    // client codes
    CC_NAME    = 0x08, // user name
    CC_CNCT    = 0x09, // user connect request
    CC_RECN    = 0x0A, // user reconnect request
    CC_MVFR    = 0x0B, // user move from
    CC_MVTO    = 0x0C, // user move to
    CC_GMOV    = 0x0D, // user game over
    CC_AGAIN   = 0x0E, // user play again
    CC_EXIT    = 0x0F, // user exit
    CC_KICK    = 0x10, // user kick
};


#endif
