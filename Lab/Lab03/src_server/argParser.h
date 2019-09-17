#pragma once

#include <string>

/**
 * ArgsOptions structure contains:
 *          1. portNumber: listening or sending port
 */
struct ArgsOptions{
    int portNumber;
};

/**
 * Prints information about how to use this program
 */
void print_usage();

/**
 * Parses all arguments provided by users 
 */ 
ArgsOptions parse_args(int argc, char **argv);