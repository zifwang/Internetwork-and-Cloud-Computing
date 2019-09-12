#include "argParser.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <stdlib.h>     /* atoi */

using namespace std;

void print_usage(){
    // Come back later
    std::cout << 
        "To run this program, must follow the following: \n"
        "Client side: \n"
        "Arguments Explains: \n"
        " 0.program_name: \n"
        " 1.help: \n "
        "        Print this help message and exit. \n"
        " 1.ipAddress: \n"
        " 2.portNumber: \n"
        " 3.transferFileName: \n";
}

ArgsOptions parse_args(int argc, char **argv){
    ArgsOptions args;
    
    if(argc < 2){
        cout << "Syntax Error - Incorrect Parameter Usage:" << endl;
        print_usage();
        exit(EXIT_FAILURE);
    }
    else{
        string str1(argv[1]);
        if(str1.compare("help") == 0){
            print_usage();
            exit(EXIT_SUCCESS);
        }
        else{
            if(argc < 3){
                cout << "Syntax Error - Incorrect Parameter Usage:" << endl;
                print_usage();
                exit(EXIT_FAILURE);
            }
            else if(argc == 3){
                args.ipAddress = argv[1];
                args.portNumber = atoi(argv[2]);
            }
            else if(argc == 4){
                args.ipAddress = argv[1];
                args.portNumber = atoi(argv[2]);
                args.transferFileName = argv[3];
            }
            else{
                cout << "Error, too many arguments: " << endl;
                print_usage();
                exit(EXIT_FAILURE);
            }
        }
    }

    return args;
}