#include <iostream>
#include "Logger.hpp"

#define DEBUG 1

void server_setup();
void client_setup();

int main(int argc, char const **argv) {
#if DEBUG
    logger->setLevel(Trace);
#endif

	std::cout << "Hello, Game!" << std::endl;

    server_setup();

//    client_init();

    /* TODO delete logger instance and close file it uses */
    logger->clearInstance();

    return EXIT_SUCCESS;
}