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
#include "udp_server.h"

using namespace std;

int main(int argc, char **argv){
    // Arguments Parse
    ArgsOptions args = parse_args(argc, argv);

    udp_server myServer = udp_server(args.portNumber);
    myServer.run();



    return 0;
}

