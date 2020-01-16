#ifndef SERVER_HPP
#define SERVER_HPP

// sockaddr_in
#include <netinet/in.h>

#include <condition_variable>
#include <mutex>
#include <vector>

#include "ClientManager.hpp"


class Server {
private:
    // --- ATTRIBUTES ---

    /** Default size of queue for new connections. */
    constexpr static const int BACK_LOG = 5;
    /** Default size of buffer. */
    constexpr static const int SIZE_BUFF = 1024;
    /** Default length of messages for receiving. */
    constexpr static const int SIZE_RECV = SIZE_BUFF - 1;
    /** Longest valid message server may accept (chat). */
    constexpr static const int LONGEST_MSG = 106;
    /** Ping messages period in milliseconds. */
    constexpr static const int PING_PERIOD = 5000;

    /** Manages connected clients. */
    ClientManager mngClient;

    /** Mutex for pinging thread -- vector of clients is critical section. */
    std::mutex mtx;
    /** Condition variable for pinging thread. Release after PING_PERIOD milliseconds. */
    std::condition_variable cv;

    /** Maximum count of connected clients. One extra for the one client,
     * who reaches maximum capacity and is disconnected after "max capacity" message. */
    int maxClients;
    /** Maximum count of game rooms. */
    int maxRooms;

    /** Sockets for select. */
    fd_set sockets{};
    /** Server address. */
    struct sockaddr_in serverAddress{};
    /** Server socket index for file descriptor. */
    int serverSocket;

    /** Buffer for receiving messages. */
    char buffer[SIZE_BUFF]{};

    /** Total received bytes. Server is only receiving. */
    int bytesRecv;

    // --- METHODS ---

	/** Initialize server. (called from constructor) */
	void init(const char*, const int&);
	/** Shutddown server. (called in Server::run() after while loop. */
	void shutdown();

    /** Main loop for updating clients. */
    void updateClients(fd_set&, fd_set&);

    /** Accept new client connections. */
	void acceptConnection();
	/** Refuse connection, when server is full. */
	void refuseConnection();
	/** Close client's connection. */
    clientsIterator closeConnection(clientsIterator&, const char*);

	/** Receive message from client. */
	int readClient(const int&);
    /** Serve client according to received message. */
    int serveClient(Client&);

    /** Asynchronous pinging clients. */
    void pingClients();

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
	/** Constructor. */
    Server(const char*, const int&, const int&, const int&);

    /** Runs server. */
    void run();
    /** Print server statistics. */
    void prStats();

    /** Get server's IP addres. */
    char* getIPaddress();
    /** Get server's port. */
    int getPort();
    /** Get maximum count of clients on server. */
    int getMaxClients();
    /** Get maximum count of game rooms on server. */
    int getMaxRooms();
};

#endif
