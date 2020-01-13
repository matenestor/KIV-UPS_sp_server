// inet_addr()
#include <arpa/inet.h>
// ioctl()
#include <sys/ioctl.h>
// socket()
#include <sys/socket.h>
#include <sys/types.h>
// close()
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <thread>

#include "../system/Logger.hpp"
#include "../system/signal.hpp"
#include "packet_handler.hpp"
#include "Server.hpp"


// ---------- CONSTRUCTORS & DESTRUCTORS ----------





/******************************************************************************
 *
 * Constructor initializes variables to default values and then initializes
 * itself. If something could not be initialized, throws an exception.
 *
 */

Server::Server(const char* addr, const int& port, const int& clients, const int& rooms) {
    // basic initialization
    strcpy(this->ipAddress, addr);
    this->port       = port;
    this->maxClients = clients + 1; // +1 for client, who is told, that server is full
    this->maxRooms   = rooms;

    this->sockets       = {0};
    this->serverAddress = {0};
    this->serverSocket  = 0;

    this->clearBuffer(this->buffer);

    this->bytesRecv = 0;


    // initialize server
    try {
        this->init();
    }
    catch (const std::exception& ex) {
        logger->error("%s [%s]. IP address [%s] port: [%d] ", ex.what(), std::strerror(errno), addr, port);
        throw std::runtime_error("Unable to create a Server instance.");
    }
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
    this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // check socket creation
    if (this->serverSocket < 0) {
        throw std::runtime_error(std::string("Unable to create server socket."));
    }

    // --- INIT ADDRESS ---

    // fill server socket
    this->serverAddress.sin_family = AF_INET;
    this->serverAddress.sin_port = htons(this->port);

    if (inet_pton(AF_INET, this->ipAddress, &this->serverAddress.sin_addr.s_addr) != 1) {
        throw std::runtime_error(std::string("Unable to assign IP address to server."));
    }

    // --- INIT BIND ---

    // bind server socket with address
    if (bind(this->serverSocket, (struct sockaddr *) &(this->serverAddress), sizeof(struct sockaddr_in)) != 0) {
        throw std::runtime_error(std::string("Unable to bind server socket with server address."));
    }

    // --- INIT LISTEN ---

    // start listening on server socket
    if (listen(this->serverSocket, BACK_LOG) != 0) {
        throw std::runtime_error(std::string("Unable to listen on server socket."));
    }

    // --- INIT FD ---

    // clear client sockets
    FD_ZERO(&(this->sockets));
    // set server socket
    FD_SET(this->serverSocket, &(this->sockets));
}


/******************************************************************************
 *
 * 	First close sockets, then notify pinging thread.
 *
 */
void Server::shutdown() {
    this->closeSockets();
    this->cv.notify_one();
}


// ----- UPDATE


/******************************************************************************
 *
 *  Accept new client connection, if max capacity is reached, refuse last connected client.
 *  Loop over all connected clients. Check for changes on except file descriptor
 *  and read file descriptor.
 *  If there is change on read file descriptor, try to read message of client and serve,
 *  if there is something in buffer, or if there is nothing in buffer,
 *  it means, that client logged out.
 *  During errors, disconnect client.
 *
 */
void Server::updateClients(fd_set& fds_read, fd_set& fds_except) {
    int received = 0;
    int client_socket = 0;

    // server socket -- request for new connection
    if (FD_ISSET(this->serverSocket, &fds_read)) {
        this->acceptClient();
    }

    // check if capacity for connected clients is not full
    if (this->mngClient.getClientsCount() == this->maxClients) {
        this->refuseClient();
    }

    // loop over all connected clients
    for (auto cli = this->mngClient.getVectorOfClients().begin();
              cli != this->mngClient.getVectorOfClients().end();
              /* no increment -- increment in the end of loop or by erase() */ ) {

        client_socket = cli->getSocket();

        // except file descriptor change
        if (FD_ISSET(client_socket, &fds_except)) {
            cli = this->closeClient(cli, "except file descriptor error");
            continue;
        }

        // read file descriptor change
        if (FD_ISSET(client_socket, &fds_read)) {
            ioctl(client_socket, FIONREAD, &received);

            // if there is something in receive buffer, read
            if (received > 0) {
                // this variable may change to non-zero value according to what is client sending
                received = this->readClient(client_socket);

                // successful message receive
                if (received > 0) {
                    if (this->serveClient(*cli) != 0) {
                        cli = this->closeClient(cli, "violation of protocol");
                        continue;
                    }
                }
            }

            // client logout
            if (received == 0) {
                cli = this->closeClient(cli, "logout");
                continue;
            }
            // bad socket
            else if (received < 0) {
                cli = this->closeClient(cli, "no message received");
                continue;
            }
        }

        ++cli;
    }

//    this->mngClient.prAllClients();
}


// ----- CLIENT MANAGING


void Server::acceptClient() {
    // client socket index for file descriptor.
    int client_socket = 0;

    // address of incoming connection
    struct sockaddr_in peer_addr{};
    // length of address of incoming connection
    socklen_t peer_addr_len{};

    // accept new connection
    client_socket = accept(this->serverSocket, (struct sockaddr *) &peer_addr, &peer_addr_len);

    if (client_socket > 0) {
        // set new connection to file descriptor
        FD_SET(client_socket, &(this->sockets));
        this->mngClient.getClientsConnected() += 1;
        // add new socket number to vector, for update loop
        this->mngClient.createClient(client_socket);

        logger->info("New connection on socket [%d].", client_socket);
    }
    else {
        logger->error("New connection could not be established. [%s]", std::strerror(errno));
    }
}


void Server::refuseClient() {
    // new connected client
    auto newClient = this->mngClient.getVectorOfClients().end() - 1;

    // message about too many connections
    this->mngClient.sendToClient(newClient, Protocol::SC_MANY_CLNT);

    // and close connection
    this->closeClient(newClient, "server is full");
}


/******************************************************************************
 *
 * 	The only valid method to call, in order to disconnect client.
 * 	It clears socket in server's list of sockets, closes it,
 * 	and then tells ClientManager to delete the client from its list.
 *
 */
clientsIterator Server::closeClient(clientsIterator& client, const char* reason) {
    // server remove connection
    FD_CLR(client->getSocket(), &(this->sockets));
    // close socket
    close(client->getSocket());

    // client manager remove connection.. and return iterator on next client
    return this->mngClient.eraseClient(client, reason);
}


/******************************************************************************
 *
 *  Read while there is something in buffer to read or while class buffer is not full.
 *  If class buffer is full, disconnect client, becauase client is flooding the server.
 *
 */
int Server::readClient(const int& socket) {
    int in_buffer = 0;
    int received = 0;
    int received_total = 0;

    // local step buffer receiving possible not whole data
    char step_buffer[SIZE_BUFF]{'\0'};
    // clear server's buffer
    this->clearBuffer(this->buffer);

    while(1) {
        ioctl(socket, FIONREAD, &in_buffer);

        // something is still there to read
        if (in_buffer > 0) {
            received = recv(socket, step_buffer, SIZE_RECV, 0);

            // read from client, until buffer is full
            if (received_total < SIZE_RECV - LONGEST_MSG && received >= 0) {
                // increment total received bytes in this reading from socket
                received_total += received;
                // increment total received bytes in server lifetime (even during flooding)
                this->bytesRecv += received_total;

                // copy step buffer to class buffer
                this->insertToBuffer(this->buffer, step_buffer);
                this->clearBuffer(step_buffer);
            }
        }
        // end of receiving
        else if (in_buffer == 0) {
            logger->debug("End of receiving. Received: [%s]", this->buffer);
            break;
        }

        // client is flooding the server (not possible to fill 1KB buffer
        // during turn-based game, with used protocol, like this one)
        if (received_total > SIZE_RECV - LONGEST_MSG || in_buffer < 0 || received < 0) {
            logger->warning("Server is being flooded. Going to disconnect client [%d].", socket);
            received_total = -1;
            break;
        }
    }

    return received_total;
}


/******************************************************************************
 *
 *
 * 	@return If client was successfully served, returns 0, else return 1.
 *
 */
int Server::serveClient(Client& client) {
    // according to C standards, it is better to return 0 on success,
    // so this code doesn't give headaches on return values
    int valid = 0;
    clientData data = clientData();

    if (isValidFormat(this->buffer) == 0) {
        parseMsg(this->buffer, data);
    }
    else {
        logger->warning("Server received invalid data. Going to disconnect client [%d].", client.getSocket());
        valid = 1;
    }

    // always should be true, when message is in valid format
    if (!data.empty()) {
        valid = this->mngClient.process(client.getSocket(), data);
    }

    return valid;
}


/******************************************************************************
 *
 * 	Called by pinging thread to ping clients.
 *
 */
void Server::pingClients() {
    while (isRunning) {
        std::unique_lock<std::mutex> lock(mtx);
        this->mngClient.pingClients();
        this->cv.wait_for(lock, std::chrono::milliseconds(PING_PERIOD));
    }
}


// ----- SOCKET CLOSING


void Server::closeSockets() {
    this->closeClientSockets();
    this->closeServerSocket();
}


void Server::closeClientSockets() {
    for (auto client = this->mngClient.getVectorOfClients().begin();
              client != this->mngClient.getVectorOfClients().end();
              /* no increment -- auto-incremented by erase() */) {
        client = this->closeClient(client, "Server shutdown.");
    }
}


void Server::closeServerSocket() {
    close(this->serverSocket);
    FD_CLR(this->serverSocket, &(this->sockets));

    logger->info("Server socket closed.");
}


// ----- OTHERS


void Server::clearBuffer(char* p_buff) {
    std::memset(p_buff, 0, SIZE_BUFF);
}


void Server::insertToBuffer(char* p_dst, char* p_src) {
    // copy everything, except newline characters, from source to destination
    while (*p_src) {
        if (!(*p_src == '\n' || *p_src == '\r')) {
            *p_dst = *p_src;
            ++p_dst;
        }
        ++p_src;
    }
}




// ---------- PUBLIC ----------





/******************************************************************************
 *
 * 	Set running flag to 1, start pinging thread and enter main server loop.
 * 	Make copy of main file descriptor made during initialization.
 * 	Wait on select for changes and then either break or update clients.
 * 	Breaks out only after CTRL+C or error on select.
 * 	After loop, safely shutdown -- close all sockets and notify pinging thread,
 * 	which is also joined in the end.
 * 	If server broke out from loop, because of error on select, throw an exception.
 *
 */
void Server::run() {
    // return value of select
    int activity = 0;
    int crash = 0;

    // sockets for comparing changes on sockets
    fd_set fdsRead{}, fdsExcept{};

    // set server as running (this is inline volatile std::sig_atomic_t variable in signal.hpp)
    isRunning = 1;

    // create pinging thread
    std::thread pingThread(&Server::pingClients, this);

    while (isRunning) {
        // create copy of client sockets, in order to compare it after select
        fdsRead   = sockets;
        fdsExcept = sockets;

        // if still running, call 'select' which finds out, if there were some changes on file descriptors
        if (isRunning) {
            activity = select(FD_SETSIZE, &fdsRead, nullptr, &fdsExcept, nullptr);
        }

        // Server most of the time waits on select() above and when SIGINT signal comes,
        // it is caught by signalHandler() in main.cpp, which sets the isRunning variable to 0.
        // Then select() is released, so this condition breaks the loop, in order to
        // the server may shutdown properly
        if (isRunning == 0) {
            break;
        }

        // crash server after error on select
        if (activity < 0) {
            isRunning = 0;
            crash = 1;
            break;
        }

        // update clients -- accept, recv
        {
            const std::lock_guard<std::mutex> lock(this->mtx);
            this->updateClients(fdsRead, fdsExcept);
        }
    }

    // safely shutdown server
    {
        const std::lock_guard<std::mutex> lock(this->mtx);
        this->shutdown();
    }

    pingThread.join();

    if (crash) {
        throw std::runtime_error(std::string("select is negative.."));
    }
}


void Server::prStats() {
    logger->info("--- Printing statistics ---");
    logger->info("Clients connected: %d",    this->mngClient.getClientsConnected());
    logger->info("Clients disconnected: %d", this->mngClient.getClientsDisconnected());
    logger->info("Clients reconnected: %d",  this->mngClient.getClientsReconnected());
    logger->info("Bytes received: %d",       this->bytesRecv);
    logger->info("Bytes sent: %d",           this->mngClient.getBytesSend());
    logger->info("--- Printing statistics --- DONE");
}


// ----- GETTERS


char* Server::getIPaddress() {
    return this->ipAddress;
}

int Server::getPort() {
    return this->port;
}

int Server::getMaxClients() {
    return this->maxClients - 1;
}

int Server::getMaxRooms() {
    return this->maxRooms;
}
