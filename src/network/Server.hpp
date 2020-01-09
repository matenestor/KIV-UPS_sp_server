#ifndef SERVER_HPP
#define SERVER_HPP

// sockaddr_in, arpa/inet.h -> htons()
#include <netinet/in.h>

#include <vector>

#include "ClientManager.hpp"
#include "PacketHandler.hpp"


using vecIterator = std::vector<int, std::allocator<int>>;

class Server {
private:
    // --- ATTRIBUTES ---

    /** Default size of queue for new connections. */
    constexpr static const int BACK_LOG = 5;
    /** Default port number. */
    constexpr static const int DEFAULT_PORT = 10000;
    /** Seconds before timeout. */
    constexpr static const int TIMEOUT_SEC = 30;
    constexpr static const int TIMEOUT_USEC = 1;

    /** Client manager takes care of client sessions. */
//    ClientManager mngClient; // TODO
    /** Packet handler parses received messages and creates messages for send according to protocol. */
    PacketHandler hndPacket; // TODO

    /** Port number to be connected to. */
    int port;

    /** Server socket index for file descriptor. */
    int server_socket;
	/** Server address. */
    struct sockaddr_in server_address{};
	/** Sockets for select. */
    fd_set sockets{};
    /** Socket numbers. */
    std::vector<int> socket_nums;

    // received and sent bytes count
    int bytes_recv;
    int bytes_send;

    // --- METHODS ---

	/** Initialize server. (called from constructor) */
	void init();
	/** Shutddown server. (called in Server::run() after while loop. */
	void shutdown();

    /** Accept new client connections. */
	void acceptClient();
	/** Read what client sent. */
//	void readClient(const int&);
    /** Pinging clients, in order to prevent unnecessary waiting for them. */
    void pingClients();

    int readFromClient(const int&);
    int writeToClient(const int&);
    vecIterator::iterator closeClient(vecIterator::iterator&, const char*);

    void closeSockets();
    void closeClientSockets();
    void closeServerSocket();

    void prStats();

public:
	/** Constructor, which does everything. */
    explicit Server(int);
	/** Empty constructor, inherits from constructor with port parameter. */
    Server();

    /** Runs server. */
    void run();
    /** Get server's port. */
    int getPort();
};

#endif
