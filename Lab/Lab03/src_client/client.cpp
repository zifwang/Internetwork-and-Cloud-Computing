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
    // ArgsOptions args = parse_args(argc,argv);
    string fileName = "Atlantic.txt";
    

    // vector<string> myVect = readFile(fileName,fileSize,totalFrame);




    // socket
    // int sock, n;
    // unsigned int length;
    // struct sockaddr_in server, from;
    // struct hostent *hp;
    // char data_buffer[274];

    // sock= socket(AF_INET, SOCK_DGRAM, 0);
    // if (sock < 0) error("socket");
    // server.sin_family = AF_INET;
    // hp = gethostbyname(argv[1]);
    // if (hp == 0) error("Unknown host");
    // bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
    // server.sin_port = htons(atoi(argv[2]));
    // length=sizeof(struct sockaddr_in);
    // bzero(data_buffer,274);


    return 0;
}