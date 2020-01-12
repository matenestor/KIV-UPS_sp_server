#include <iostream>

#include "../network/server_handler.hpp"
#include "Logger.hpp"


extern int parse_arguments(const int&, char const **, Defaults&);


/******************************************************************************
 *
 * 	Help for user.
 *
 */
void pr_help() {
    std::cout <<
    "\nGame server of Hnefatafl is runned by thy commands, which follows:\n\n"
    "Usage: hnefserver [options]\n"
    "Options:\n"
    "  -a    IPv4 address                   default: 127.0.0.0\n\n"
    "  -p    Port                           default: 10000\n"
    "                                       range: <1024;49151>\n"
    "  -c    Max count of connected clients default: 10\n"
    "                                       range: <2;20>\n"
    "  -r    Max count of game rooms        default: 5\n"
    "                                       range: <1;10>\n\n"
    "Created by matenestor for KIV/UPS. Skål!\n"
    << std::endl;
}


/******************************************************************************
 *
 * 	Parses arguments -- if ok, then start server,
 * 	else print help or advice to print help.
 *
 */
int main(int argc, char const **argv) {
    // set logger level
    logger->setLevel(Trace);

    // default server parameters
    Defaults defs{"127.0.0.1", 10000, 10, 5};

    // parse terminal arguments
    int rv = parse_arguments(argc, argv, defs);


    if (rv == 0) {
        std::cout << "+-------------------+" << std::endl;
        std::cout << "| Hello, Hnefatafl! |" << std::endl;
        std::cout << "+-------------------+" << std::endl;

        // setup server and start everything
        server_setup(defs);
    }
    else if (rv == 1) {
        pr_help();
    }
    else {
        std::cout << "Run \"hnefserver -h\" to print help." << std::endl;
    }


    // delete logger instance and close file it uses
    logger->clearInstance();

    return EXIT_SUCCESS;
}
