#include <iostream>
#include <regex>

#include "Logger.hpp"
#include "main.hpp"
#include "signal.hpp"
#include "../network/protocol.hpp"


void signalHandler(int signum) {
    logger->info("Interrupt signal received [%d].", signum);
    isRunning = 0;
}

int main(int argc, char const **argv) {
    std::cout << "Hello, Game!" << std::endl;

    #if DEBUG
    logger->setLevel(Trace);

//    logger->fatal   ("test fatal    msg: string %s, int %d, float %.5f", "Ab1yZ2", 42, 3.14159);
//    logger->error   ("test error    msg: string %s, int %d, float %.5f", "Ab1yZ2", 42, 3.14159);
//    logger->warning ("test warning  msg: string %s, int %d, float %.5f", "Ab1yZ2", 42, 3.14159);
//    logger->info    ("test info     msg: string %s, int %d, float %.5f", "Ab1yZ2", 42, 3.14159);
//    logger->debug   ("test debug    msg: string %s, int %d, float %.5f", "Ab1yZ2", 42, 3.14159);
//    logger->trace   ("test trace    msg: string %s, int %d, float %.5f", "Ab1yZ2", 42, 3.14159);
    #endif

    // register signal SIGINT with signal handler function
    std::signal(SIGINT, signalHandler);

    // setup server and start everything
    server_setup();

    // delete logger instance and close file it uses
    logger->clearInstance();

    return EXIT_SUCCESS;
}