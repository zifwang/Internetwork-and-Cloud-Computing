#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "packet.h"
#include "udp_server.h"

using namespace std;

udp_server::udp_server(int portNumber){
    port_number = portNumber;

    /*Clear all the data buffer and structure*/
    memset(&server, 0, sizeof(server));
	memset(&from, 0, sizeof(from));
    
    // init socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) error("Cannot create socket");
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(port_number);
    if(bind(sockfd, (struct sockaddr *) &server, sizeof(server)) == -1){
        error("Server Bind error");
    }
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval));
}

void udp_server::run(){
    while(1){
        receive_header(sockfd, from);
        // receive_file();
    }
}



/**
 * Private functions sections
 */

// void send_file(){}

void udp_server::receive_file(){
    // Declare variables
    struct packet receive_packet;
    struct packet send_packet;
    int receive_from_return_number;
    memset(&receive_packet, 0, sizeof(receive_packet));
    memset(&send_packet, 0, sizeof(send_packet));
    int unreceive_counter = 0;

    // Get file
    receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
    while(receive_packet.typePacket != DONE){
        string str(receive_packet.dataBuffer);
        receive_file_map[receive_packet.packetSequence] = str;
        memset(&receive_packet, 0, sizeof(receive_packet));

        receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
    }
}

void udp_server::receive_header(int sockfd, struct sockaddr_in from){
    int header;
    struct packet header_packet;

    memset(&header_packet, 0, sizeof(header_packet));
    int receive_header_number = 0;
    // Receive Header
    header = recvfrom(sockfd, &(header_packet), sizeof(header_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
    cout << header << endl;
    while(header_packet.typePacket != REQUEST){
        header = recvfrom(sockfd, &(header_packet), sizeof(header_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
        if(receive_header_number >= 20){
            error("Error receive header from sender");
        }
        receive_header_number++;
    }
    receive_header_number = 0;

    // Set file status
    string tmp(header_packet.dataBuffer);
    file_name = tmp;
    total_frame = header_packet.packetSequence;

    // Update typePacket of header_packet
    header_packet.typePacket = (enum packetType)REQUEST_ACK;


    // Send confirmation of receiving packet to sender
    header = sendto(sockfd, &(header_packet), sizeof(header_packet), 0, (struct sockaddr* ) &from, sizeof(from));
    while(header < 0){
        header = sendto(sockfd, &(header_packet), sizeof(header_packet), 0, (struct sockaddr* ) &from, sizeof(from));
        if(receive_header_number >= 20){
            error("Error receive header from sender");
        }
        receive_header_number++;
    }

    return;
}























void udp_server::error(const char *msg){
    perror(msg);
    exit(0);
}





vector<string> udp_server::readFile(string fileName, long fileSize, long totalFrame){
    
    FILE *file;
    vector<string> v;
    char *buffer;

    file = fopen(fileName.c_str(),"rb");

    while(fileSize > 0){
        int chunck = 0;
        if(fileSize <= DATABUFFER_SIZE - 1) chunck = fileSize;
        else chunck = DATABUFFER_SIZE - 1;
        buffer = new char[chunck+1];

        fread(buffer,1,chunck,file);
        buffer[chunck+1] = '\0';

        // for(int i = 0; i < strlen(buffer); i++){
        //     cout << buffer[i];
        // }
        // cout << endl;

        v.push_back(buffer);
        bzero(buffer,chunck);

        fileSize -= chunck;
    }
    fclose(file);

    return v;
}

