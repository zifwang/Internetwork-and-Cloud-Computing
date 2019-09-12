#include "argParser.h"
#include <iostream>
#include <cstdlib>

using namespace std;

void print_usage(){
    // Come back later
    std::cout << 
        "To run this program, must follow the following: \n"
        "Server side: \n"
        "Arguments Explains: \n"
        " 0.program_name: \n"
        " 1.help: \n "
        "        Print this help message and exit. \n"
        " 1.portNumber: \n"
        " 2.transferFileName: \n";
}

ArgsOptions parse_args(int argc, char **argv){
    ArgsOptions flags;
    
    if(argc < 2){
        cout << "Syntax Error - Incorrect Parameter Usage:" << endl;
        print_usage();
        exit(EXIT_FAILURE);
    }
    else if(argc == 2){
        if(argv[1] == "help"){
            print_usage();
            exit(EXIT_SUCCESS);
        }else{
            flags.portNumber = atoi(argv[1]);
        }
    }
    else if(argc == 3){
        flags.transferFileName = argv[1];
    }
    else{
        print_usage();
        exit(EXIT_FAILURE);
    }
}