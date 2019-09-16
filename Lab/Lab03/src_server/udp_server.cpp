#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <bits/stdc++.h>
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
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_out, sizeof(struct timeval));
}

void udp_server::run(){
    while(1){
        receive_header(sockfd, from);
        cout << "Successfully Receive header" << endl;
        cout << "Total_Frame: " << total_frame << endl;
        receive_file();
        // for (auto& t : receive_file_map){
        //     std::cout << t.first << " " << t.second <<  "\n";
        // }
        cout << "Receive number of packets: " << receive_file_map.size() << endl;
        cout << "Receive number of packets: " << receive_file_sequence.size() << endl;
        break;
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
        cout << receive_from_return_number << endl;
        if(receive_packet.packetSequence >= 0){
            string str(receive_packet.dataBuffer);
            receive_file_map[receive_packet.packetSequence] = str;
            receive_file_sequence.push_back(receive_packet.packetSequence);
        }
        cout << "Type: " << receive_packet.typePacket << endl;
        cout << "Sequence: " << receive_packet.packetSequence << endl;
        cout << "Size: " << receive_file_sequence.size() << endl;
        memset(&receive_packet, 0, sizeof(receive_packet));

        receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
    }
    cout << "Get First file done" << endl;

    // Receive Missing frame
    receive_missing_frame(sockfd,from);

    return;
}

void udp_server::receive_header(int sockfd, struct sockaddr_in from){
    int header;
    struct packet header_packet;

    memset(&header_packet, 0, sizeof(header_packet));
    int receive_header_number = 0;
    // Receive Header
    header = recvfrom(sockfd, &(header_packet), sizeof(header_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
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

void udp_server::receive_missing_frame(int sockfd, struct sockaddr_in from){
    struct packet receive_packet;
    int receive_from_return_number;
    memset(&receive_packet, 0, sizeof(receive_packet));

    while(is_missing_frame(receive_file_sequence, missing_frame)){
        request_to_resend_missing_frame(sockfd,from,missing_frame);
        receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
        while(receive_packet.typePacket != DONE_MISSING){
            if(receive_packet.packetSequence >= 0){
                string str(receive_packet.dataBuffer);
                receive_file_map[receive_packet.packetSequence] = str;
                receive_file_sequence.push_back(receive_packet.packetSequence);
            }

            memset(&receive_packet, 0, sizeof(receive_packet));

            receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
        }
        cout << "Check" << endl;

    }
    // send receive all frame signal
    send_packet(sockfd, from, "Receive ALL Frames", DONE_ACK, long(-3));
    cout << "Receive ALL Frames" << endl;

    return;
}

bool udp_server::is_missing_frame(vector<long> receive_file_sequence, vector<long> &missing_frame){
    cout << "Check is_missing_frame" << endl;
    missing_frame.clear();
    // sort vector
    sort(receive_file_sequence.begin(),receive_file_sequence.end());
    bool flag = false;
    long prev = 0;
    for(int i = 0; i < receive_file_sequence.size(); i++){
        if(i == 0 && receive_file_sequence[i] > prev){
            flag = true;
            for(int j = prev; j < receive_file_sequence[i]; j++){
                missing_frame.push_back(long(j));
            }
            prev = receive_file_sequence[i];
            continue;
        }
        if(prev == receive_file_sequence[i]) continue;
        if(prev + 1 < receive_file_sequence[i]){
            flag = true;
            for(int j = prev+1; j < receive_file_sequence[i]; j++){
                missing_frame.push_back(long(j));
            }
            prev = receive_file_sequence[i];
            continue;
        }
        if(prev + 1 == receive_file_sequence[i]){
            prev = receive_file_sequence[i];
        }
    }
    if(receive_file_sequence[receive_file_sequence.size()-1] < total_frame){
        for(int k = receive_file_sequence[receive_file_sequence.size()-1] + 1; k <= total_frame; k++){
            missing_frame.push_back(long(k));
        }
    }
    cout << "flag: " << flag << endl;
    return flag;
}

void udp_server::request_to_resend_missing_frame(int sockfd, struct sockaddr_in from, vector<long> missing_frame){
    string send_string = "";
    for(int i = 0; i < missing_frame.size(); i++){
        string current_missing = to_string(missing_frame[i]);
        if(send_string.length() + current_missing.length() <= DATABUFFER_SIZE - 1){
            send_string = send_string + current_missing + ",";
        }
        else{
            // send packet
            send_string.pop_back();
            send_packet(sockfd, from, send_string, MISSING, send_string.length());
            // set send_string to empty
            send_string = "";
            send_string = send_string + current_missing + ","; 
        }
    }
    // send last missing
    send_packet(sockfd, from, send_string, MISSING_SEND_DONE, send_string.length());

    return;
}

void udp_server::send_packet(int sockfd, struct sockaddr_in from, string send_data, packetType type, long sequence){
    struct packet packet_send;
    int send_to_return_number = 0;
    int timeOut_couter = 0;
    memset(&packet_send, 0, sizeof(packet_send));
    // set packet
    packet_send.dataSize = long(send_data.length());
    packet_send.packetSequence = sequence;
    packet_send.typePacket = type;
    strcpy(packet_send.dataBuffer,send_data.c_str());

    // send packet
    send_to_return_number = sendto(sockfd,&(packet_send),sizeof(packet_send),0,(const struct sockaddr *) &from, sizeof(from));
    cout << send_to_return_number << endl;
    while(send_to_return_number < 0){
        if(timeOut_couter > 20){
            error("Error in sending packet");
        }
        send_to_return_number = sendto(sockfd,&(packet_send),sizeof(packet_send),0,(const struct sockaddr *) &from, sizeof(from));
        timeOut_couter++;
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

void udp_server::printPacket(struct packet myPacket){
    cout << "File Sequence: " << myPacket.packetSequence << endl;
    cout << "File Type: " << myPacket.typePacket << endl;
    cout << "File DataSize: " << myPacket.dataSize << endl;
    cout << "File Data: " << string(myPacket.dataBuffer) << endl;
}
