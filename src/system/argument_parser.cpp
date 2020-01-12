#include <cstring>
#include <iostream>
#include <regex>

#include "../network/server_handler.hpp"


/******************************************************************************
 *
 * 	Returns true, if given string is valid IPv4 Address.
 *
 */
bool is_ipv4_address(const char* str) {
    return std::regex_match(str, std::regex(R"(^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(?:\.(?!$)|$)){4}$)"));
}


/******************************************************************************
 *
 * 	Returns successfully parsed value, -1 otherwise.
 *
 */
int parse_number(const char* num_str) {
    int num = -1;

    try {
        num = std::stoi(num_str);
    }
    catch (std::invalid_argument const &e) {
        std::cout << "Bad input: not a number [" << num_str << "]" << '\n';
    }
    catch (std::out_of_range const &e) {
        std::cout << "Bad input: number not in range [" << num_str << "]" << '\n';
    }

    return num;
}


/******************************************************************************
 *
 * 	Handles string value of parsed flag.
 *
 */
void handle_flag_string(const char* str, char* attribute, int& rv) {
    if (is_ipv4_address(str)) {
        strcpy(attribute, str);
    }
    else {
        std::cout << "Invalid argument: " << str << std::endl;
        rv = -1;
    }
}


/******************************************************************************
 *
 * 	Handles int value of parsed flag.
 *
 */
void handle_flag_int(const char* str, int& attribute, const int& bottom, const int& top, int& rv) {
    int num = parse_number(str);

    if (num >= bottom && num <= top) {
        attribute = num;
    }
    else {
        std::cout << "Invalid argument: " << str << std::endl;
        rv = -1;
    }
}


/******************************************************************************
 *
 * 	Parses arguments given on startup.
 * 	Returns 0 if everything were successfully parsed,
 * 	1 if user wants to print help, and -1 otherwise.
 *
 */
int parse_arguments(const int& argc, char const **argv, Defaults& defs) {
    // return value
    int rv = 0;

    // just print help, after return
    if (argc > 1 && strcmp(argv[1], "-h") == 0) {
        rv = 1;
    }
    // parse all argument for server instance, go like (shift;shift;)
    else {
        for (int i = 1; i < argc; i+=2) {
            // valid flag starting with '-'
            if ((char) argv[i][0] == '-' && strlen(argv[i]) == 2) {
                if (i+1 < argc) {
                    switch ((char) argv[i][1]) {
                        case 'a':
                            // valid IPv4 address
                            handle_flag_string(argv[i+1], defs.def_addr, rv);
                            break;

                        case 'p':
                            // valid port number
                            handle_flag_int(argv[i+1], defs.def_port, 1024, 49151, rv);
                            break;

                        case 'c':
                            // valid client count number
                            handle_flag_int(argv[i+1], defs.def_clients, 2, 20, rv);
                            break;

                        case 'r':
                            // valid game rooms number
                            handle_flag_int(argv[i+1], defs.def_rooms, 1, 10, rv);
                            break;

                        default:
                            std::cout << "Invalid flag: " << argv[i] << std::endl;
                            rv = -1;
                    }
                }
                else {
                    std::cout << "No value given for flag: " << argv[i] << std::endl;
                    rv = -1;
                }
            }
            // something not valid
            else {
                std::cout << "Not a flag: " << argv[i] << std::endl;
                rv = -1;
            }

            // break after bad argument
            if (rv == -1) {
                break;
            }
        }
    }

    return rv;
}
