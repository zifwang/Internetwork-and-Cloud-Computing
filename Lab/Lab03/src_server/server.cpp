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
    ArgsOptions args = parse_args(argc, argv);



    return 0;
}

