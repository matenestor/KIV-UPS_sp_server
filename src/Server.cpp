// socket()
#include <sys/socket.h>
#include <sys/types.h>
// iotctl()
#include <sys/ioctl.h>
// close()
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <map>
#include <algorithm>

#include "Logger.hpp"
#include "Server.hpp"

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

// ---------- CONSTRUCTORS & DESTRUCTORS





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
    this->socket_nums = std::vector<int>();
    this->bytes_recv = 0;
    this->bytes_send = 0;

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





// ---------- PRIVATE





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
    // return value for checks of success
    int rv;

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


/******************************************************************************
 *
 *
 *
 */
void Server::acceptClient() {
    // client socket index for file descriptor.
    int client_socket = 0;

    // address of incoming connection
    struct sockaddr_in address_peer{};
    // length of address of incoming connection
    socklen_t address_len{};

    // accept new connection // TODO setSocketClient()
    client_socket = accept(this->server_socket, (struct sockaddr *) &address_peer, &address_len);

    if (client_socket > 0) {
        // set new connection to file descriptor
        FD_SET(client_socket, &(this->sockets));
        // add new socket number to vector
        this->socket_nums.emplace_back(client_socket);

        logger->info("New connection on socket [%d].", client_socket);
    }
    else {
        logger->error("New connection could not be established. [%s]", std::strerror(errno));
    }
}


/******************************************************************************
 *
 *  TODO
 *
 */
//void Server::readClient(const int& fd) {
//    char cbuf;
//    int a2read;
//
//    // pocet bajtu co je pripraveno ke cteni
//    ioctl(fd, FIONREAD, &a2read);
//    // mame co cist
//    if (a2read > 0) {
//        recv(fd, &cbuf, 1, 0);
//        printf("Prijato %c\n", cbuf);
//        read(fd, &cbuf, 1);
//        printf("Prijato %c\n", cbuf);
//    }
//        // na socketu se stalo neco spatneho
//    else {
//        close(fd);
//        FD_CLR(fd, &sockets);
//        printf("Klient se odpojil a byl odebran ze sady socketu\n");
//    }
//}


/******************************************************************************
 *
 *  TODO
 *
 */
void Server::pingClients() {

}


// ----- SOCKET CLOSING


void Server::closeClientSockets() {
    for (const auto& num : socket_nums) {
        close(num);
        logger->debug("Client connection closed [%d]", num);
    }
}

void Server::closeServerSocket() {
    close(this->server_socket);
    logger->debug("Server socket closed.");
}

void Server::closeSockets() {
    this->closeClientSockets();
    this->closeServerSocket();
}


// ----- CLIENT MESSAGES


// buffer for receiving and sending TODO this location of buffer is tmp
int BUFF_SIZE = 256;
char buffer[256];

int Server::readFromClient(const int& socket) {
    int bytes = 0;

    std::memset(buffer, '\0', BUFF_SIZE);
    bytes = recv(socket, buffer, BUFF_SIZE, 0);
    enqueue(socket, std::string(buffer));
    this->bytes_recv += bytes;

    logger->debug("received: %s", buffer);

    return bytes;
}

int Server::writeToClient(const int& socket) {
    int bytes = 1;
    int len = 1;

    if (queue.find(socket) != queue.end())
        len = queue.at(socket).begin()->size();

    std::memset(buffer, '\0', BUFF_SIZE);
    strcpy(buffer, dequeue(socket).c_str());
    bytes = send(socket, buffer, len, 0);
    this->bytes_send += bytes;

    logger->debug("sent: %s", buffer);

    return bytes;
}

vecIterator::iterator Server::closeClient(vecIterator::iterator& socket) {
    logger->debug("Closing socket [%d]", socket);

    close(*socket);
//    FD_CLR(socket, &fds_read); TODO necessary?

    return this->socket_nums.erase(socket);
}





// ---------- PUBLIC





/******************************************************************************
 *
 */
void Server::run() {
    isRunning = true;

    // return value for checks of success
    int activity;

    // sockets for comparing changes on sockets
    fd_set fds_read{}, fds_write{}, fds_except{};

    // tv_sec seconds before timeout
    struct timeval tv{0};
    tv.tv_sec = TIMEOUT;

    // TODO this is one big todo now...
    while (isRunning) {
        // create copy of client sockets, in order to compare it after select
        fds_read   = sockets;
        fds_write  = sockets;
        fds_except = sockets;

        // call 'select' which finds out, if there was some change on file descriptors
        activity = select(FD_SETSIZE, &fds_read, &fds_write, &fds_except, &tv);

        // ping after timeout
        if (activity == 0) {
//            this->pingClients();
            logger->info("no connection messages, select release");
        }

        // TODO clear gamerooms, clients, etc??
        // crash server after error on select
        if (activity < 0) {
            this->closeSockets();
            throw std::runtime_error(std::string("select is negative.."));
        }

        // server socket -- request for new connection
        if (FD_ISSET(this->server_socket, &fds_read)) {
            this->acceptClient();
        }

        // loop over all connected clients
        for (vecIterator::iterator i = socket_nums.begin(); i != socket_nums.end(); i++) {
            if (*i > *std::max_element(this->socket_nums.begin(), this->socket_nums.end())) {
                continue;
            }

            logger->debug(">>> DEBUG FDS START -------------------- \\/\\/\\/");

            // read
            if (FD_ISSET(*i, &fds_read)) {
                logger->debug("client [%d] read TRUE", *i);

                // TODO just receive always... bug here, client disconnection corrupts this for-loop
//                this->readFromClient(*i);

                if (this->readFromClient(*i) <= 0) {
                    i = this->closeClient(i);
                    continue;
                }
            }
            else logger->debug("client [%d] read FALSE", *i);

            // write
            if (FD_ISSET(*i, &fds_write)) {
                logger->debug("client [%d] write TRUE", *i);
                if (this->writeToClient(*i) <= 0) {
                    i = this->closeClient(i);
                    continue;
                }
            }
            else logger->debug("client [%d] write FALSE", *i);

            // except
            if (FD_ISSET(*i, &fds_except)) {
                logger->debug("client [%d] exception TRUE", *i);
                i = this->closeClient(i);
                continue;
            }
            else logger->debug("client [%d] exception FALSE", *i);

            logger->debug(">>> DEBUG FDS END ---------------------- /\\/\\/\\\n");
        }

        this->prStats();

        // reset timer
        tv.tv_sec = TIMEOUT;
    }

    this->closeSockets();
}


// ----- GETTERS


int Server::getPort() {
    return this->port;
}

void Server::prStats() {
    logger->debug(">>> STATS ------------------------------ <<<");
    // print statistics
    for (const auto& num : this->socket_nums) {
        logger->debug("client socket: [%d]", num);
    }
    logger->debug("bytes received: %d", this->bytes_recv);
    logger->debug("bytes sent: %d\n\n", this->bytes_send);
}

/* WASTELAND

for (fd = 3; fd < FD_SETSIZE; fd++) {

            // socket is in file descriptors, which can be read from
            if (FD_ISSET(fd, &fds_read)) {
                if (fd == this->server_socket) {
                    this->acceptClient();
                }
                // client socket -- read received data
                else {
                    this->readClient(fd);
                }
            }
        }
 */