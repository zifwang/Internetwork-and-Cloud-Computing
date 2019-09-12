#include <sys/types.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include "argParser.h"

using namespace std;

int main(int argc, char **argv){
    // Arguments Parse
    ArgsOptions flags = parse_args(argc, argv);
    cout << flags.portNumber << endl;
    cout << flags.transferFileName << endl;


    return 0;
}
