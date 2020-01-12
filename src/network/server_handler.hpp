#ifndef SERVER_HANDLER_HPP
#define SERVER_HANDLER_HPP


struct Defaults {
    // default ip address
    char def_addr[16];
    // default port number
    int def_port;
    // default count of client
    int def_clients;
    // default count of game rooms
    int def_rooms;
};

void server_setup(Defaults&);


#endif
