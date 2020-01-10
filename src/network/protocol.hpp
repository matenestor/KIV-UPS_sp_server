#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <regex>


using ClientData = std::vector<std::string>;

namespace Protocol {

    // examples of protocol message

    // C -> S
    // {c,n:nick}
    // {m:07050710}

    // S -> C
    // {rr,il}
    // {rr,ig,ty,op:onick,pf:0..9}
    // -> {rr,ig,ty,op:onick,pf:0000000000111111111122222222223333333333444444444455555555550000000000111111111122222222223333333333}
    // {ig,ty,op:nick}

    // operation codes
    static const std::string OP_SOH     ("{"); // start of header
    static const std::string OP_EOT     ("}"); // end of transmission
    static const std::string OP_SEP     (","); // separator
    static const std::string OP_INI     (":"); // initializer of data
    static const std::string OP_PING    (">"); // enquiry
    static const std::string OP_PONG    ("<"); // acknowledge

    // client codes
    static const std::string CC_CONN    ("c"); // connect request
    static const std::string CC_RECN    ("r"); // reconnect request
    static const std::string CC_NAME    ("n"); // name
    static const std::string CC_MOVE    ("m"); // move
    static const std::string CC_LEAV    ("l"); // leave game
    static const std::string CC_EXIT    ("e"); // exit connection

    // server codes
    static const std::string SC_RESP_CONN    ("rc"); // response connect
    static const std::string SC_RESP_RECN    ("rr"); // response reconnect
    static const std::string SC_RESP_LEAVE   ("rl"); // response leave
    static const std::string SC_IN_LOBBY     ("il"); // client moved to lobby
    static const std::string SC_IN_GAME      ("ig"); // client moved to game
    static const std::string SC_MV_VALID     ("mv"); // valid move
    static const std::string SC_TURN_YOU     ("ty"); // your's turn
    static const std::string SC_TURN_OPN     ("to"); // opponent's turn
    static const std::string SC_PLAYFIELD    ("pf"); // playfield
    static const std::string SC_GO_WIN       ("gw"); // game over win
    static const std::string SC_GO_LOSS      ("gl"); // game over loss
    static const std::string SC_OPN_NAME     ("on"); // opponent's name
    static const std::string SC_OPN_MOVE     ("om"); // opponent's move
    static const std::string SC_OPN_LEAVE    ("ol"); // opponent left the game
    static const std::string SC_OPN_EXIT     ("oe"); // opponent disconnected
    static const std::string SC_OPN_RECN     ("or"); // opponent reconnected
    static const std::string SC_MANY_PLRS    ("t");  // too many players message
    static const std::string SC_NAME_USED    ("u");  // name is already used
    static const std::string SC_KICK         ("k");  // kick client

    // chat code
    static const std::string OP_CHAT         ("ch"); // chat


    // client regex: (\{(<|>|rc|rr,il|rr,ig,(ty|to),on:\w{3,20},pf:\d{100}|rl|il|ig,(ty|to),on:\w{3,20}|mv|gw|gl|om:\d{8}|ol|oe|or|t|u|k|ch:\w{1,100})\})+

    // server regex -- valid format: (\{(<|>|c,n:\w{3,20}|r,n:\w{3,20}|m:\d{8}|l|e|ch:\w{1,100})\})+
    static const std::regex validFormat("(\\"+OP_SOH+"("
                                         +OP_PING+"|"
                                         +OP_PONG+"|"
                                         +CC_CONN+OP_SEP+CC_NAME+OP_INI+"\\w{3,20}|"
                                         +CC_RECN+OP_SEP+CC_NAME+OP_INI+"\\w{3,20}|"
                                         +CC_MOVE+OP_INI+"\\d{8}|"
                                         +CC_LEAV+"|"
                                         +CC_EXIT+"|"
                                         +OP_CHAT+OP_INI+"\\w{1,100}"
                                         +")\\"+OP_EOT+")+");

    // server regex -- valid data: \{[\w,:<>]+\}
    static const std::regex dataFormat("\\"+OP_SOH+"[\\w"+OP_SEP+OP_INI+OP_PING+OP_PONG+"]+\\"+OP_EOT);
};


#endif
