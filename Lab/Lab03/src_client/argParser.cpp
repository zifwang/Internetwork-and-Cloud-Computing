#include "argParser.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <stdlib.h>                 /* atoi */
#include <getopt.h>                 /* get option */
#include "udp_client.h"             /* default port number */

using namespace std;

void print_usage(){
    /**
     * Print how to run the program and what arguments need to be provided
     * Argument: no
     * Return: no
     */ 
    cout << 
        "Usage: ./client -p port_number\n"
        "In order to run client: server's ip address must be set.\n"
        "\n"
        "Options:\n"
        "  -h, --help:\n"
        "      Print this help message and exit.\n"
        "  -i, --server's ip address <ip address>: \n"
        "      Server's ip address. Provide server's ip address to connect to server.\n"
        "  -p, --send on port number <port number>:\n"
        "      If set, the client will send packets through the given port.\n"
        "  -c, --user's commands options <options>:\n"
        "      If set, the client will follow users' commands. Valid values are:\n"
        "           upload: upload file to server\n"
        "           download: download file from server\n"
        "           message: send and receive short messages from server\n"
        "  -f, --filename <filename>:\n"
        "      If set, the client will upload file with filename to server if this file exists.\n";

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
        {"server's_ip_address", required_argument, 0, 'i'},
        {"sending_through_port_number", required_argument, 0, 'p'},
        {"user's_commands_option", required_argument, 0, 'c'},
        {"user'filename", required_argument, 0, 'f'},
        {"help",       no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int option_index;
    char args_char;

    // Parse args entered by the user.
    while (true) {
        args_char = getopt_long(argc, argv, "-hi:p:c:f:", args_options, &option_index);

        // Detect the end of the options.
        if (args_char == -1) {
            break;
        }

        switch (args_char) {
            case 'i':
                args.ipAddress = string(optarg);
                break;

            case 'p':
                args.portNumber = atoi(optarg);
                break;

            case 'c':
                args.command = string(optarg);
                break;

            case 'f':
                args.transferFileName = string(optarg);
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