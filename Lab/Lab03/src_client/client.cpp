#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <vector>
#include "argParser.h"
#include "packet.h"
#include "udp_client.h"

using namespace std;

int main(int argc, char *argv[]){
    // argument parser
    ArgsOptions args = parse_args(argc,argv);

    // socket
    udp_client myClient = udp_client(args.ipAddress, args.portNumber, args.command, args.transferFileName);
    myClient.run();


    return 0;
}