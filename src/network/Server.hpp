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
    /** Default size of buffer. */
    constexpr static const int SIZE_BUFF = 1024;
    /** Default length of messages for receiving. */
    constexpr static const int SIZE_RECV = SIZE_BUFF - 1;
    /** Longest valid message server may accept (chat). */
    constexpr static const int LONGEST_MSG = 106;

    /** Handles received messages. */
    PacketHandler hndPacket;
    /** Manages connected clients. */
    ClientManager mngClient;

    /** Port number to be connected to. */
    int port;

    /** Server socket index for file descriptor. */
    int server_socket;
	/** Server address. */
    struct sockaddr_in server_address{};
	/** Sockets for select. */
    fd_set sockets{};
    /** Socket numbers. */
    std::vector<int> client_sockets;

    /** Buffer for receiving messages. */
    char buffer[SIZE_BUFF]{};

    // received and sent bytes count
    int bytes_recv;
    int bytes_send;

    // connected, disconnected and reconnected clients
    int cli_connected;
    int cli_disconnected;
    int cli_reconnected;

    // --- METHODS ---

	/** Initialize server. (called from constructor) */
	void init();
	/** Shutddown server. (called in Server::run() after while loop. */
	void shutdown();

    /** Main loop for updating clients. */
    void updateClients(fd_set&, fd_set&);

    /** Accept new client connections. */
	void acceptClient();
	/** Receive message from client. */
	int readClient(const int&);
    /** Serve client according to received message. */
    int serveClient(const int&);
    /** Pinging clients, in order to prevent unnecessary waiting for them. */
    void pingClients();
    /** Disconnect client with bad socket. */
    vecIterator::iterator disconnectClient(vecIterator::iterator& socket, const char* reason);

    /** Calls methods to close both server and client sockets. */
    void closeSockets();
    /** Close client sockets. */
    void closeClientSockets();
    /** Close server socket. */
    void closeServerSocket();

    /** Clear buffer for message receiving. */
    void clearBuffer(char*);
    /** Inserts part of received message from buffer in readClient() to class buffer. */
    void insertToBuffer(char*, char*);

public:
	/** Constructor, which does everything. */
    explicit Server(int);
	/** Empty constructor, inherits from constructor with port parameter. */
    Server();

    /** Runs server. */
    void run();
    /** Print server statistics. */
    void prStats();

    /** Get server's port. */
    int getPort();
};

#endif
