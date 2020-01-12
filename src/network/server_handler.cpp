#include <cstring>
#include <memory>

#include "../system/Logger.hpp"
#include "../system/signal.hpp"
#include "server_handler.hpp"
#include "Server.hpp"


/******************************************************************************
 *
 * 	Tells server to stop running on CTRL+C.
 *
 */
void signalHandler(int signum) {
    logger->info("Interrupt signal received [%d].", signum);
    isRunning = 0;
    cv.notify_one();
}


/******************************************************************************
 *
 * Creates server instance as unique pointer.
 * Exits with error value in 'catch', if instance was not created.
 *
 * @return Moves unique pointer.
 *
 */
std::unique_ptr<Server> server_init(const Defaults& defs) {
    std::unique_ptr<Server> server = nullptr;

    logger->info("Initializing server.");

    try {
        // create server instance
        server = std::make_unique<Server>(defs.def_addr, defs.def_port, defs.def_clients, defs.def_rooms);
    }
    catch (const std::exception& ex) {
        // if server was not created, print exception and exit
        logger->fatal(ex.what());
        exit(EXIT_FAILURE);
    }

    logger->info("Server initialized.");
    logger->info("IP address: [%s]",      server->getIPaddress());
    logger->info("port: [%d]",            server->getPort());
    logger->info("max. clients: [%d]",    server->getMaxClients());
    logger->info("max. game rooms: [%d]", server->getMaxRooms());

    return std::move(server);
}


/******************************************************************************
 *
 * Runs server instance, which have been passed as argument.
 *
 * @param server Server instance.
 *
 */
void server_run(std::unique_ptr<Server> server) {
    logger->info("Running server.");

    try {
        // run server
        server->run();
    }
    catch (const std::exception& ex) {
        // if server crashed, log exception and exit
        logger->fatal("Server crashed [%s, %s].", ex.what(), std::strerror(errno));
        exit(EXIT_FAILURE);
    }

    logger->info("Server stopped successfully.");

    // print statistics after successful shutdown
    server->prStats();
}


/******************************************************************************
 *
 * Calls a function, that creates server instance.
 * Then calls a function, that runs the instance.
 *
 */
void server_setup(Defaults& defs) {
    // register signal SIGINT with signal handler function
    std::signal(SIGINT, signalHandler);

    // create server instance
    auto server = server_init(defs);
    // run the server
    server_run(std::move(server));
}
