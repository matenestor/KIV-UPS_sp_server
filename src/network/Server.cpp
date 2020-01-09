// socket()
#include <sys/socket.h>
#include <sys/types.h>
// close()
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>

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
    this->cnt_connected = 0;
    this->cnt_disconnected = 0;
    this->cnt_reconnected = 0;

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

    // accept new connection // TODO setSocketClient()
    client_socket = accept(this->server_socket, (struct sockaddr *) &peer_addr, &peer_addr_len);

    if (client_socket > 0) {
        // set new connection to file descriptor
        FD_SET(client_socket, &(this->sockets));
        // add new socket number to vector, for update loop
        this->client_sockets.emplace_back(client_socket);

        this->cnt_connected += 1;

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
void Server::pingClients() {
    logger->info("no connection messages");
}


// ----- CLIENT MESSAGES


// buffer for receiving and sending TODO this location of buffer is tmp

int Server::readFromClient(const int& socket) {
    int bytes = 0;
    int BUFF_SIZE = 16;
    char buffer[16];

    std::memset(buffer, '\0', BUFF_SIZE);
    bytes = recv(socket, buffer, BUFF_SIZE - 1, 0);

//    if (this-hndPacket.isValidSOT(buffer_step)) {
//        this->bytes_recv += bytes;
//    }
//
//    if (this->hndPacket.isValidEOT(buffer_step)) {
//        this->bytes_recv += bytes;
//    }

//    enqueue(socket, std::string(buffer));
    this->bytes_recv += bytes;

    logger->debug("received: [%d] [%s]", bytes, buffer);

    if (std::string(buffer) == "x") {
        // simulate disconnection
        bytes = 0;
    }

    return bytes;
}

//int Server::writeToClient(const int& socket) {
//    int bytes = 1;
//    int len = 1;
//
//    if (queue.find(socket) != queue.end())
//        len = queue.at(socket).begin()->size();
//
//    std::memset(buffer, '\0', BUFF_SIZE);
//    strcpy(buffer, dequeue(socket).c_str());
//    bytes = send(socket, buffer, len, 0);
//    this->bytes_send += bytes;
//
//    logger->debug("sent: %s", buffer);
//
//    return bytes;
//}


// ----- SOCKET CLOSING


vecIterator::iterator Server::disconnectClient(vecIterator::iterator& socket, const char* reason) {
    close(*socket);
    FD_CLR(*socket, &(this->sockets));
    this->cnt_disconnected += 1;

    logger->info("Client on socket [%d] closed [%s],", *socket, reason);

    return this->client_sockets.erase(socket);
}


void Server::closeClientSockets() {
    for (auto itr = client_sockets.begin(); itr != client_sockets.end(); ) {
        itr = disconnectClient(itr, "server shutdown");
    }
}


void Server::closeServerSocket() {
    close(this->server_socket);
    FD_CLR(this->server_socket, &(this->sockets));

    logger->debug("Server socket closed.");
}


void Server::closeSockets() {
    this->closeClientSockets();
    this->closeServerSocket();
}


// ----- OTHERS


void Server::shutdown() {
    this->closeSockets();
}


// ----- UPDATE


void Server::updateClients(fd_set& fds_read, fd_set& fds_except) {
    int received = 0;

    logger->debug(">>> DEBUG FDS START -------------------- \\/\\/\\/");

    // server socket -- request for new connection
    if (FD_ISSET(this->server_socket, &fds_read)) {
        this->acceptClient();
    }

    // loop over all connected clients
    for (auto itr = client_sockets.begin(); itr != client_sockets.end(); ) {
        if (this->client_sockets.empty()) {
            logger->debug("WARNING: Looping over empty socket numbers vector! breaking.. ");
            break;
        }

        // except file descriptor change
        if (FD_ISSET(*itr, &fds_except)) {
            logger->debug("client [%d] exception TRUE", *itr);
            itr = this->disconnectClient(itr, "except file descriptor error");
            continue;
        }

        // read file descriptor change
        if (FD_ISSET(*itr, &fds_read)) {
            logger->debug("client [%d] read TRUE", *itr);

            received = this->readFromClient(*itr);

            if (received < 0) {
                logger->debug("corrupt recv from client [%d], closing connection [%s]", *itr, std::strerror(errno));
                itr = this->disconnectClient(itr, "no message received -- violation or timeout");
                continue;
            }
            else if (received == 0) {
                logger->debug("Client on socket [%d] logout.", *itr);
                itr = this->disconnectClient(itr, "logout");
                continue;
            }
            else {
                logger->debug("sth was receiver successfully");
            }
        }

        ++itr;
    }

    logger->debug(">>> DEBUG FDS END ---------------------- /\\/\\/\\\n");
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

    // TODO this is one big todo now...
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

        // ping after timeout
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
    logger->debug(">>> STATS ------------------------------ <<<");
    // print statistics
    for (const auto& num : this->client_sockets) {
        logger->debug("client socket: [%d]", num);
    }
    logger->debug("bytes received: %d", this->bytes_recv);
    logger->debug("bytes sent: %d\n\n", this->bytes_send);
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



void Server::readClient(const int& fd) {
    char cbuf;
    int a2read;

    // pocet bajtu co je pripraveno ke cteni
    ioctl(fd, FIONREAD, &a2read);
    // mame co cist
    if (a2read > 0) {
        recv(fd, &cbuf, 1, 0);
        printf("Prijato %c\n", cbuf);
        read(fd, &cbuf, 1);
        printf("Prijato %c\n", cbuf);
    }
        // na socketu se stalo neco spatneho
    else {
        close(fd);
        FD_CLR(fd, &sockets);
        printf("Klient se odpojil a byl odebran ze sady socketu\n");
    }
}

 */