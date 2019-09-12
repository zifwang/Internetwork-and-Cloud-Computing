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
#include "argParser.h"

using namespace std;

int main(int argc, char *argv[]){
    ArgsOptions args = parse_args(argc,argv);
    cout << args.portNumber << endl;
    cout << args.ipAddress << endl;




    return 0;
}