#include <iostream>
#include <map>

#include "Logger.hpp"
#include "ServerHandler.hpp"


int main(int argc, char const **argv) {
#if DEBUG
    logger->setLevel(Trace);

//    logger->fatal   ("test fatal    msg: string %s, int %d, float %.5f", "Ab1yZ2", 42, 3.14159);
//    logger->error   ("test error    msg: string %s, int %d, float %.5f", "Ab1yZ2", 42, 3.14159);
//    logger->warning ("test warning  msg: string %s, int %d, float %.5f", "Ab1yZ2", 42, 3.14159);
//    logger->info    ("test info     msg: string %s, int %d, float %.5f", "Ab1yZ2", 42, 3.14159);
//    logger->debug   ("test debug    msg: string %s, int %d, float %.5f", "Ab1yZ2", 42, 3.14159);
//    logger->trace   ("test trace    msg: string %s, int %d, float %.5f", "Ab1yZ2", 42, 3.14159);
#endif

    std::cout << "Hello, Game!" << std::endl;

    server_setup();

    /* delete logger instance and close file it uses */
    logger->clearInstance();

    return EXIT_SUCCESS;
}