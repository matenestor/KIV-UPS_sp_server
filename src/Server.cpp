// socket()
#include <sys/socket.h>
#include <sys/types.h>
// iotctl()
#include <sys/ioctl.h>
// close()
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>

#include "Logger.hpp"
#include "Server.hpp"


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

//    this->client_socket = 0;
//    this->fd = 0;
//    this->cbuf = 0;
//    this->len_addr = 0;
//    this->a2read = 0;
//    this->peer_addr = {0};
//    this->tests = {0};

    try {
        // initialize server
        this->init();
    }
    catch (const std::exception& ex) {
        logger->error("%s (%s). [port: %d]", ex.what(), std::strerror(errno), p);
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
    this->server_address.sin_addr.s_addr = INADDR_ANY;

    // --- INIT BIND ---

    // bind server socket with address
    rv = bind(this->server_socket, (struct sockaddr *) &(this->server_address), sizeof(struct sockaddr_in));

    // check if socket is bound
    if (rv != 0) {
        throw std::runtime_error(std::string("Unable to bind server socket with server address. "));
    }

    // --- INIT LISTEN ---

    // start listening on server socket
    rv = listen(this->server_socket, BACK_LOG);

    // check if server is listening
    if (rv != 0) {
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
    int client_socket;

    // address of incoming connection
    struct sockaddr_in address_peer{};
    // length of address of incoming connection
    socklen_t address_len;

    // accept new connection
    client_socket = accept(this->server_socket, (struct sockaddr *) &address_peer, &address_len);
    // set new connection to file descriptor
    FD_SET(client_socket, &(this->sockets));

    logger->info("New connection on socket [%d].", client_socket);
}


/******************************************************************************
 *
 *
 *
 */
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





// ---------- PUBLIC





/******************************************************************************
 *
 */
void Server::run() {
    // return value for checks of success
    int rv;
    // file descriptor for checks of changes
    int fd;

    // sockets for comparing changes on sockets
    fd_set fds_read{}, fds_write{}, fds_except{};



//    // TODO ping after time t ?
//    struct timeval tv{0};
//    tv.tv_sec = 1;


    std::cout << "> RUN" << std::endl;

    // TODO this is one big todo now...
//    while (1) {
//        // create copy of client sockets, in order to compare it after select
//        fds_read   = sockets;
//        fds_write  = sockets;
//        fds_except = sockets;
//
//        // call 'select' which finds out, if there was some change on file descriptors
//        rv = select(FD_SETSIZE, &fds_read, &fds_write, &fds_except, nullptr);
//
//        if (rv < 0) {
//            throw std::runtime_error(std::string("Server crashed. File descriptors error. "));
//        }
//
//        // fd = 3 -- leave out stdin, stdout, stderr
//        // TODO recreate to list of connections (FD_SETSIZE has value of 65536... or 64?... on Debian VM i have 1024)
//        for (fd = 3; fd < FD_SETSIZE; fd++) {
//
//            // socket is in file descriptors, which can be read from
//            if (FD_ISSET(fd, &fds_read)) {
//                // server socket -- request for new connection
//                if (fd == this->server_socket) {
//                    this->acceptClient();
//                }
//                // client socket -- read received data
//                else {
//                    this->readClient(fd);
//                }
//            }
//        }
//
//        break;
//    }
}

// ----- GETTERS

u_int Server::getPort() {
    return this->port;
}
