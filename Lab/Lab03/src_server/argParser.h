#pragma once
#include <string>

/**
 * ArgsOptions structure contains:
 *          1. transferFileName: file needs to transfer
 *          2. ipAddress: address connect to server
 *          3. portNumber: listening or sending port
 */
struct ArgsOptions{
    int portNumber;
    std::string transferFileName;
    std::string ipAddress;
};

/**
 * Prints information about how to use this program
 */
void print_usage();

/**
 * Parses all arguments provided by users 
 */ 
ArgsOptions parse_args(int argc, char **argv);