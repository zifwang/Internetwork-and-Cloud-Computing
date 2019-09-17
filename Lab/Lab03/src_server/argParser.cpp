#include "argParser.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <getopt.h>             /* get option */
#include <stdlib.h>             /* atoi */
#include "udp_server.h"         /* default port number */

using namespace std;

void print_usage(){
    /**
     * Print how to run the program and what arguments need to be provided
     * Argument: no
     * Return: no
     */ 
    std::cout << 
        "Usage: ./server -p port_number\n"
        "\n"
        "Options:\n"
        "  -h, --help:\n"
        "      Print this help message and exit.\n"
        "  -p, --listening port number <port number>:\n"
        "      If set, the server will listen on the given port.\n";

    return;
}

ArgsOptions parse_args(int argc, char **argv){
    /**
     * Function to parse input argument
     * Argument: argc (int type): how many arguments provided by user
     *           argv (pointer array): what args provided by user
     * Return:
     *          args: (ArgsOptions type): return a ArgsOptions struct
     */
    // Init. return
    ArgsOptions args;
    
    // set port number to default
    args.portNumber = DEFAULTPORT;

    // Command-line args accepted by this program.
    static struct option args_options[] = {
        {"listening_port_number", required_argument, 0, 'p'},
        {"help",       no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int option_index;
    char args_char;

    // Parse args entered by the user.
    while (true) {
        args_char = getopt_long(argc, argv, "-p:h", args_options, &option_index);

        // Detect the end of the options.
        if (args_char == -1) {
            break;
        }

        switch (args_char) {
            case 'p':
                args.portNumber = atoi(optarg);
                break;

            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
                break;

            default:
                print_usage();
                exit(EXIT_FAILURE);
        }
    }

    return args;
}