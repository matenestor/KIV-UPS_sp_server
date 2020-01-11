// socket()
#include <sys/socket.h>
#include <sys/types.h>
// close()
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <sys/ioctl.h>

#include "../system/Logger.hpp"
#include "../system/signal.hpp"
#include "Server.hpp"


// ---------- CONSTRUCTORS & DESTRUCTORS ----------





/******************************************************************************
 *
 * Constructor initializes variables to default values and then initializes
 * itself. If something could not be initialized, throws an exception.
 *
 * @param p
 *
 */
Server::Server(const int p) {
    // basic initialization
    this->port = p;
    this->server_socket = 0;
    this->server_address = {0};
    this->sockets = {0};
    this->client_sockets = std::vector<int>();
    this->bytes_recv = 0;
    this->bytes_send = 0;
    this->cli_connected = 0;
    this->cli_disconnected = 0;
    this->cli_reconnected = 0;

    this->clearBuffer();

    // initialize server
    try {
        this->init();
    }
    catch (const std::exception& ex) {
        logger->error("%s [%s]. [port: %d]", ex.what(), std::strerror(errno), p);
        throw std::runtime_error("Unable to create a Server instance.");
    }
}


/******************************************************************************
 *
 * Empty constructor without parameters. Inherits from constructor,
 * where is everything implemented and passes there DEFAULT_PORT value.
 *
 */
Server::Server() : Server(DEFAULT_PORT) {
    // empty
}





// ---------- PRIVATE ----------





/******************************************************************************
 *
 * Called from a constructor.
 * Creates server's socket, if fails, throws an exception.
 * Initializes server's address and port.
 * Binds server's socket with address, if fails, throws an exception.
 * Starts listening on given port, if fails, throws an exception.
 * Initializes file descriptors -- clears wait file descriptor
 * and sets server file descriptor.
 *
 */
void Server::init() {
    // --- INIT SOCKET ---

    // create server socket
    this->server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // check socket creation
    if (this->server_socket < 0) {
        throw std::runtime_error(std::string("Unable to create server socket. "));
    }

    // --- INIT ADDRESS ---

    // fill server socket
    this->server_address.sin_family = AF_INET;
    this->server_address.sin_port = htons(this->port);
    // TODO implement assignable own IP address
    this->server_address.sin_addr.s_addr = INADDR_ANY;

    // --- INIT BIND ---

    // bind server socket with address
    if (bind(this->server_socket, (struct sockaddr *) &(this->server_address), sizeof(struct sockaddr_in)) != 0) {
        throw std::runtime_error(std::string("Unable to bind server socket with server address. "));
    }

    // --- INIT LISTEN ---

    // start listening on server socket
    if (listen(this->server_socket, BACK_LOG) != 0) {
        throw std::runtime_error(std::string("Unable to listen on server socket. "));
    }

    // --- INIT FD ---

    // clear client sockets
    FD_ZERO(&(this->sockets));
    // set server socket
    FD_SET(this->server_socket, &(this->sockets));
}


void Server::shutdown() {
    this->closeSockets();
}


// ----- UPDATE


/******************************************************************************
 *
 *
 *
 */
void Server::updateClients(fd_set& fds_read, fd_set& fds_except) {
    int received = 0;

    // server socket -- request for new connection
    if (FD_ISSET(this->server_socket, &fds_read)) {
        this->acceptClient();
    }

    // loop over all connected clients
    for (auto itr = client_sockets.begin(); itr != client_sockets.end(); ) {

        // except file descriptor change
        if (FD_ISSET(*itr, &fds_except)) {
            itr = this->disconnectClient(itr, "except file descriptor error");
            continue;
        }

        // read file descriptor change
        if (FD_ISSET(*itr, &fds_read)) {
            ioctl(*itr, FIONREAD, &received);

            // if there is something in receive buffer, read
            if (received > 0) {
                // this variable may change to non-zero value according to what is client sending
                received = this->readClient(*itr);

                // successful message receive
                if (received > 0) {
                    if (this->serveClient(*itr) != 0) {
                        itr = this->disconnectClient(itr, "violation of protocol");
                        continue;
                    }
                }
            }

            // client logout
            if (received == 0) {
                itr = this->disconnectClient(itr, "logout");
                continue;
            }
            // bad socket
            else if (received < 0) {
                itr = this->disconnectClient(itr, "no message received");
                continue;
            }
        }

        ++itr;
    }
}


// ----- CLIENT MANAGING


/******************************************************************************
 *
 *
 *
 */
void Server::acceptClient() {
    // client socket index for file descriptor.
    int client_socket = 0;

    // address of incoming connection
    struct sockaddr_in peer_addr{};
    // length of address of incoming connection
    socklen_t peer_addr_len{};

    // accept new connection
    client_socket = accept(this->server_socket, (struct sockaddr *) &peer_addr, &peer_addr_len);

    if (client_socket > 0) {
        // set new connection to file descriptor
        FD_SET(client_socket, &(this->sockets));
        this->cli_connected += 1;
        // add new socket number to vector, for update loop
        this->client_sockets.emplace_back(client_socket);

        logger->info("New connection on socket [%d].", client_socket);
    }
    else {
        logger->error("New connection could not be established. [%s]", std::strerror(errno));
    }
}


/******************************************************************************
 *
 *
 *
 */
int Server::readClient(const int& socket) {
    int received = 0;
    int received_total = 0;
    this->clearBuffer();

    do {
        received = recv(socket, this->buffer, SIZE_RECV, 0);

        // read from client, until buffer is full (27 == longest valid message server may accept)
        if (received_total < SIZE_RECV - 27 && received >= 0) {
            // increment total received bytes in this reading from socket
            received_total += received;
            // increment total received bytes in server lifetime (even during flooding)
            this->bytes_recv += received_total;
        }
        // client is flooding the server (not possible to fill 1KB buffer
        // during turn-based game, with used protocol, like this one)
        else {
            received = -1;
            received_total = -1;
        }
    } while (received > 0);

    return received_total;
}


/******************************************************************************
 *
 *
 * 	@return If client was successfully served, returns 0 else return 1.
 *
 */
int Server::serveClient(const int& client) {
    // according to C standards, it is better to return 0 on success,
    // so this code doesn't give headaches on return values
    int valid = 0;
    ClientData data = ClientData();

    if (this->hndPacket.isValidFormat(this->buffer) == 0) {
        this->hndPacket.parseMsg(this->buffer, data);
    }
    else {
        valid = 1;
    }

    // always should be true, when message is in valid format
    if (!data.empty()) {
        valid = this->mngClient.process(client, data);
    }

    return valid;
}


// TODO
void Server::pingClients() {
    logger->info("pinging");
}


vecIterator::iterator Server::disconnectClient(vecIterator::iterator& socket, const char* reason) {
    close(*socket);
    FD_CLR(*socket, &(this->sockets));
    this->cli_disconnected += 1;

    logger->info("Client on socket [%d] closed [%s], [%s]", *socket, reason, std::strerror(errno));

    return this->client_sockets.erase(socket);
}


// ----- SOCKET CLOSING


void Server::closeSockets() {
    this->closeClientSockets();
    this->closeServerSocket();
}


void Server::closeClientSockets() {
    for (auto itr = client_sockets.begin(); itr != client_sockets.end(); ) {
        itr = disconnectClient(itr, "server shutdown");
    }
}


void Server::closeServerSocket() {
    close(this->server_socket);
    FD_CLR(this->server_socket, &(this->sockets));

    logger->info("Server socket closed.");
}


// ----- OTHERS


void Server::clearBuffer() {
    std::memset(this->buffer, 0, SIZE_BUFF);
}





// ---------- PUBLIC ----------





/******************************************************************************
 *
 */
void Server::run() {
    // return value of select
    int activity = 0;

    // sockets for comparing changes on sockets
    fd_set fds_read{}, fds_except{};

    // tv_sec seconds before timeout
    struct timeval tv{0};
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = TIMEOUT_USEC;

    // set server as running (this is inline volatile std::sig_atomic_t variable in signal.hpp)
    isRunning = 1;

    while (1) {
        // create copy of client sockets, in order to compare it after select
        fds_read   = sockets;
        fds_except = sockets;

        // reset timeout timer
        tv.tv_sec = TIMEOUT_SEC;
        tv.tv_usec = TIMEOUT_USEC;

        // if still running, call 'select' which finds out, if there were some changes on file descriptors
        if (isRunning) {
            activity = select(FD_SETSIZE, &fds_read, nullptr, &fds_except, &tv);
        }

        // Server most of the time waits on select() above and when SIGINT signal comes,
        // it is caught by signalHandler() in main.cpp, which sets the isRunning variable to 0.
        // Then select() is released, so this condition breaks the loop, in order to
        // the server may shutdown properly
        if (isRunning == 0)
            break;

        // ping after timeout TODO
        if (activity == 0) {
            this->pingClients();
            continue;
        }

        // crash server after error on select
        if (activity < 0) {
            this->shutdown();
            throw std::runtime_error(std::string("select is negative.."));
        }

        // update clients -- accept, recv
        this->updateClients(fds_read, fds_except);
    }

    // safely shutdown server after SIGINT
    this->shutdown();
}


void Server::prStats() {
    logger->info("Clients connected: %d",    this->cli_connected);
    logger->info("Clients disconnected: %d", this->cli_disconnected);
    logger->info("Clients reconnected: %d",  this->cli_reconnected);
    logger->info("Bytes received: %d",       this->bytes_recv);
    logger->info("Bytes sent: %d",           this->bytes_send);
}


// ----- GETTERS


int Server::getPort() {
    return this->port;
}









/* WASTELAND


// --- TODO tmp functions for testing the chat communication way ---

using MsgList = std::vector<std::string>;
using QueueTable = std::map<int, MsgList>;
QueueTable queue = QueueTable();

void enqueue(const int& sock, const std::string& msg) {
    if (queue.find(sock) == queue.end()) {
        queue.insert(std::pair<int, MsgList>(sock, MsgList()));
    }
    queue.at(sock).emplace_back(msg);
}

std::string dequeue(const int& sock) {
    std::string first;
    if (queue.find(sock) == queue.end()) {
        first = "<corrupted-sock>\n";
    }
    else {
        if (queue.at(sock).begin() == queue.at(sock).end()) {
            first = "<empty>\n";
        }
        else {
            first = *queue.at(sock).begin();
            queue.at(sock).pop_back();
        }
    }
    return first;
}

 */