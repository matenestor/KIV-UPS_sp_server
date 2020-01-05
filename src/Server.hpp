#ifndef SERVER_HPP
#define SERVER_HPP

// sockaddr_in, arpa/inet.h -> htons()
#include <netinet/in.h>

#include "ClientManager.hpp"


class Server {
private:
    /** Default size of queue for new connections. */
    constexpr static const int BACK_LOG = 5;
    /** Default port number. */
    constexpr static const int DEFAULT_PORT = 10000;

    /** Client manager takes care of connected/ing wait. */
    ClientManager cmng;

    /** Port number to be connected to. */
    u_int port;

    /** Server socket index for file descriptor. */
    int server_socket;
	/** Server address. */
    struct sockaddr_in server_address{};
	/** Sockets for select. */
    fd_set sockets{};

	/** Initialize server. (called from constructor) */
	void init();
    /** Accept new client connections. */
	void acceptClient();
	/** Read what client sent. */
	void readClient(const int&);

public:
	/** Constructor, which does everything. */
    explicit Server(int);
	/** Empty constructor, inherits from constructor with port parameter. */
    Server();

    /** Runs server. */
    void run();
    /** Get server's port. */
    u_int getPort();
};

#endif
