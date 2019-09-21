#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <bits/stdc++.h>
#include "packet.h"
#include "udp_client.h"

using namespace std;

udp_client::udp_client(string ipAddress, int portNumber, string command, string fileName){
    // set variables
    ip_address = ipAddress;
    port_number = portNumber;
    udp_command = command;
    file_name = fileName;

    /*Clear all the data buffer and structure*/
    std::memset(&server, 0, sizeof(server));
	std::memset(&from, 0, sizeof(from));

    // init socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) error("Cannot create socket");
    server.sin_family = AF_INET;
    server.sin_port = htons(port_number);
    hp = gethostbyname(ip_address.c_str());
    if (hp==0) error("Unknown host");
    bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
}

void udp_client::run(){
    // Upload file
    if(send_user_request(sockfd,server,from,udp_command)){
        if(udp_command == "upload"){
            if(access(file_name.c_str(),F_OK) == 0){
                // get file status
                struct stat statics; 
                stat(file_name.c_str(),&statics);
                file_size = statics.st_size;
                if((file_size % (DATABUFFER_SIZE-1)) != 0){
                    total_frame = (file_size/(DATABUFFER_SIZE-1)) + 1;
                }else total_frame = (file_size/(DATABUFFER_SIZE-1));
                printf("File size --> %ld and Total number of packets ---> %d \n", file_size, (int)total_frame);

                // send header and receive receiver confirmation
                if(!send_header(sockfd, server, from, total_frame, file_name)) error("Error send header");

                // send file
                send_file();
            }
            else{
                std::cout << "enter correct file name" << endl;
                exit(EXIT_FAILURE);
            }
        }
        if(udp_command == "download"){
            bool flg = request_to_download_file(sockfd,server,from);
            std::cout << flg;
            receive_header(sockfd, from, server);
            std::cout << " Successfully Receive File Status: " << endl;
            std::cout << "                                  File Name: " << file_name << endl;
            std::cout << "                                  Total Frames: " << total_frame << endl;
            receive_file();
            std::cout << "Receive number of packets: " << receive_file_sequence.size() << endl;
            // writeFile(receive_file_map,file_name);
        }
        if(udp_command == "message"){
            std::cout << "finish later" << endl;
            // come back later
        }
    }
}


string udp_client::get_ip_address(){
    return ip_address;
}
int udp_client::get_connection_port_number(){
    return port_number;
}
string udp_client::get_file_name(){
    return file_name;
}
string udp_client::get_command(){
    return udp_command;
}

/**
 * private member function
 */
void udp_client::send_file(){
    // Read file to a file vector
    vector<string> fileVector = readFile(file_name, file_size, total_frame);

    // Declare variables
    struct packet receive_packet;
    // struct packet send_my_packet;
    std::memset(&receive_packet, 0, sizeof(receive_packet));
    // std::memset(&send_my_packet, 0, sizeof(send_my_packet));
    int send_to_return_number;
    int resend_frame = 0;
    bool timeout_flag = false;

    for(size_t i = 0; i < fileVector.size(); i++){
        if(i == 0){
            time_t now = time(0);
            char* dt = ctime(&now);
            cout << "Time send the frist frame is: " << dt << endl;
        }
        send_packet(sockfd,server,fileVector[i],SEND_PACKET,long(i));  //send the frame
    }
    // Send SEND_FILE_DONE signal to client
    send_packet(sockfd,server,"SEND_FILE_DONE",SEND_FILE_DONE,long(USELESS_LENGTH));
    send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    while(receive_packet.typePacket != SEND_FILE_DONE_ACK){
        send_packet(sockfd,server,"SEND_FILE_DONE",SEND_FILE_DONE,long(USELESS_LENGTH));
        send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
        if(resend_frame > MAX_SEND){
            cout << "Missing Confirmations of Receiving FILE from Server \n";
        }
        resend_frame++;
    }
    resend_frame = 0;
    std::cout << "send first file done" << endl;

    // missing_frame_sequence
    vector<long> missing_frame_seq;
    missing_frame_seq.clear();

    send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    missing_frame_packet_interpreter(receive_packet,missing_frame_seq);
    while(receive_packet.typePacket != SEND_MISSING_DONE){
        send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
        missing_frame_packet_interpreter(receive_packet,missing_frame_seq);
    }
    cout << "Missing Frame: " << endl; 
    for(int i = 0; i < missing_frame_seq.size(); i++){
        cout << missing_frame_seq[i] << endl;
    }

    // send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    // cout << send_to_return_number << endl;
    // while(receive_packet.typePacket != DONE_UPLOAD_ACK){
    //     missing_frame.clear();
    //     while(receive_packet.typePacket != UPLOAD_MISSING_SEND_DONE){
    //         missing_frame_packet_interpreter(receive_packet,missing_frame);
    //         memset(&receive_packet, 0, sizeof(receive_packet));
    //         send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    //     }
    //     cout << "get missing done" << endl;
    //     // Send missing
    //     for(int i = 0; i < missing_frame.size(); i++){
    //         // send_to_return_number = send_frame(sockfd,server,missing_frame[i],fileVector[int(missing_frame[i])]);
    //         send_packet(sockfd,server,fileVector[int(missing_frame[i])],UPLOAD,missing_frame[i]);
    //     }

    //     send_packet(sockfd,server,"DONE_UPLOAD",DONE_UPLOAD_MISSING,long(-1));
    //     cout << "send missing done" << endl;
    //     // Get new receive
    //     send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    // }
    // std::cout << "Send all frames done" << endl;
    // cout << "Type: " << receive_packet.typePacket << endl;
     

    /**
     * Version 1
    // Start sending file to receiver
    for(size_t i = 0; i < fileVector.size(); i++){
        // cout << i << endl;
        // send_to_return_number = send_frame(sockfd,server,(long)i,fileVector[i]);
        // send_packet(sockfd,server,fileVector[i],UPLOAD,(long)i);
        // std::memset(&send_my_packet, 0, sizeof(send_my_packet));
        ack_seq = 0;
        if(i == 0){
            time_t now = time(0);
            char* dt = ctime(&now);
            cout << "Time send the frist frame is: " << dt << endl;
        }
        // send_my_packet.packetSequence = long(i);
        // send_my_packet.typePacket = UPLOAD;
        // send_my_packet.dataSize = fileVector[i].length();
        // strcpy(send_my_packet.dataBuffer,fileVector[i].c_str());

        // sendto(sockfd, &(send_my_packet), sizeof(send_my_packet), 0, (struct sockaddr *) &server, sizeof(server));
        send_packet(sockfd,server,fileVector[i],UPLOAD,long(i));  //send the frame
        recvfrom(sockfd, &(ack_seq), sizeof(ack_seq), 1, (struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);	//Recieve the acknowledgement

        while(ack_seq != i){
            // sendto(sockfd, &(send_my_packet), sizeof(send_my_packet), 0, (struct sockaddr *) &server, sizeof(server));  //send the frame
            send_packet(sockfd,server,fileVector[i],UPLOAD,long(i));  // resend the frame
            recvfrom(sockfd, &(ack_seq), sizeof(ack_seq), 1, (struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);	//Recieve the acknowledgement
            std::cout << "Frame: " << i << "droped";
            resend_frame++;
            if (resend_frame == 200) {
				timeout_flag = true;
				break;
				}
        }
        resend_frame = 0;

        if (timeout_flag) {
			std::cout << "File not send" << endl;
			break;
 		}
        
        if(total_frame == ack_seq) std::cout << "File send" << endl;
    }
    std::cout << "Send all frames done" << endl
     */
    return;
}

bool udp_client::send_header(int sockfd, struct sockaddr_in server, struct sockaddr_in from, long totalFrames, string fileName){
    int send_header_number;
    int resend_header = 0;
    struct packet send_header_packet;
    struct packet receive_header_ack;
    memset(&send_header_packet, 0, sizeof(send_header_packet));
    memset(&receive_header_ack, 0, sizeof(receive_header_ack));
    
    // set packet
    send_header_packet.packetSequence = totalFrames;
    send_header_packet.typePacket = (enum packetType)SEND_FILE_STATUS;
    send_header_packet.dataSize = fileName.length();
    strcpy(send_header_packet.dataBuffer,fileName.c_str());

    // udp send
    send_header_number = sendto(sockfd,&(send_header_packet),sizeof(send_header_packet),0,(const struct sockaddr *) &server, sizeof(server));
    // udp receive
    send_header_number = recvfrom(sockfd, &(receive_header_ack), sizeof(receive_header_ack), 0, (struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    // Resend
    while(receive_header_ack.typePacket != SEND_FILE_STATUS_ACK){
        send_header_number = sendto(sockfd,&(send_header_packet),sizeof(send_header_packet),0,(const struct sockaddr *) &server, sizeof(server));
        send_header_number = recvfrom(sockfd, &(receive_header_ack), sizeof(receive_header_ack), 0, (struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
        resend_header++;
        if(resend_header >= 20){
            error("Error send header");
        }
    }
    resend_header = 0;

    string receive_file_name(receive_header_ack.dataBuffer);

    // Check equality
    if(send_header_packet.packetSequence == receive_header_ack.packetSequence && send_header_packet.dataSize == receive_header_ack.dataSize) return true;    

    return false;
}

void udp_client::send_packet(int sockfd, struct sockaddr_in server, string send_data, packetType type, long sequence){
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
    send_to_return_number = sendto(sockfd,&(packet_send),sizeof(packet_send),0,(const struct sockaddr *) &server, sizeof(server));
    // std::cout << "Send to type: " << type << ", sendtoreturn number: " << send_to_return_number << endl;
    while(send_to_return_number < 0){
        if(timeOut_couter > 20){
            error("Error in sending packet");
        }
        send_to_return_number = sendto(sockfd,&(packet_send),sizeof(packet_send),0,(const struct sockaddr *) &server, sizeof(server));
        timeOut_couter++;
    }

    return;
}

bool udp_client::send_user_request(int sockfd, struct sockaddr_in server, struct sockaddr_in from, string command){
    enum packetType tmp; 

    if(command == "upload"){
        tmp = UPLOAD_REQUEST;
        send_packet(sockfd,server,"upload",UPLOAD_REQUEST,long(-10));
    }
    else if(command == "download"){
        tmp = DOWNLOAD_REQUEST;
        send_packet(sockfd,server,"download",DOWNLOAD_REQUEST,long(-10));
    }
    else if(command == "message"){
        tmp = MESSAGE_REQUEST; 
        send_packet(sockfd,server,"message",MESSAGE_REQUEST,long(-10));
    }
    else{
        std::cout << "Command not found. Please use following provided commands." << endl;
        // A print usage function here
        return false;
    }

    int request = 0;
    int timeout = 0;
    struct packet receive_packet;
    memset(&receive_packet, 0, sizeof(receive_packet));

    request = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);

    bool flag = receive_packet.typePacket != UPLOAD_REQUEST_ACK || receive_packet.typePacket != DOWNLOAD_REQUEST_ACK || receive_packet.typePacket != MESSAGE_REQUEST_ACK;

    //  || receive_packet.typePacket != DOWNLOAD_REQUEST_ACK || receive_packet.typePacket != MESSAGE_ACK
    while(!flag){
        send_packet(sockfd,server,command,tmp,long(-10));
        memset(&receive_packet, 0, sizeof(receive_packet));
        request = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
        if(timeout > 20){
            // Change
            std::cout << "Request is not successfully accepted by server." << endl;
            // Print useage
            return false; 
        }
    }

    std::cout << "Request is successfully received by server" << endl;

    return true;
}
        
void udp_client::missing_frame_packet_interpreter(struct packet receive_packet, vector<long>& missing_frame_seq){
    string missing_frame_string = "";
    for(int i = 0; i < (int)receive_packet.dataSize; i++){
        if(receive_packet.dataBuffer[i] == ','){
            missing_frame_seq.push_back(stoi(missing_frame_string));
            missing_frame_string = "";
        }else{
            missing_frame_string = missing_frame_string + receive_packet.dataBuffer[i];
        }
    }
    missing_frame_seq.push_back(stoi(missing_frame_string));

    return;
}

void udp_client::receive_file(){
    // Declare variables
    struct packet receive_packet;
    // struct packet send_packet;
    int receive_from_return_number;
    memset(&receive_packet, 0, sizeof(receive_packet));
    int resend_frame = 0;
    // FILE *file;
    // file = fopen(file_name.c_str(), "wb");
    // memset(&send_packet, 0, sizeof(send_packet));
    // Set receive timeout
    time_out.tv_sec = 2;			
	time_out.tv_usec = 0;
    int num = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_out, sizeof(struct timeval));
    if(num < 0) error("Error setting timeout");
    
     // Get file
    receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
    if(receive_packet.typePacket == SEND_PACKET){
        string str(receive_packet.dataBuffer);
        receive_file_map[receive_packet.packetSequence] = str;
        receive_file_sequence.push_back(receive_packet.packetSequence);
    }
    while(receive_packet.typePacket != SEND_FILE_DONE){
        // std::cout << receive_from_return_number << endl;
        memset(&receive_packet, 0, sizeof(receive_packet));
        receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
        if(receive_packet.typePacket == SEND_PACKET){
            string str(receive_packet.dataBuffer);
            receive_file_map[receive_packet.packetSequence] = str;
            receive_file_sequence.push_back(receive_packet.packetSequence);
        }
        if(receive_from_return_number == -1) break;
    }
    time_t now = time(0);
    char* dt = ctime(&now);
    cout << "Time get the last frame is: " << dt << endl;
    for(int i = 0; i < MAX_SEND; i++){
        send_packet(sockfd,server,"SEND_FILE_DONE_ACK",SEND_FILE_DONE_ACK,long(USELESS_LENGTH));
    }
    std::cout << "Get First file done" << endl;

    // Check missing frame
    set<long> missing_frame_sequence = missing_frame(receive_file_sequence);
    cout << "1: "<< missing_frame_sequence.size() << endl;
    int counter = 2;
    while(missing_frame_sequence.size() != 0){
        // Send missing frame to client
        request_to_resend_missing_frame(sockfd,server,from,missing_frame_sequence);

        receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
        while(receive_packet.typePacket != SEND_FILE_DONE){
            if(receive_packet.typePacket == SEND_PACKET){
                string str(receive_packet.dataBuffer);
                receive_file_map[receive_packet.packetSequence] = str;
                missing_frame_sequence.erase(receive_packet.packetSequence);
            }
            receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
        }
        cout << "Send SEND_FILE_DONE_ACK" << endl;
        for(int i = 0; i < MAX_SEND; i++){
            send_packet(sockfd,server,"SEND_FILE_DONE_ACK",SEND_FILE_DONE_ACK,long(USELESS_LENGTH));
        }
        cout << counter <<"'s missing_frame numbers: "<< missing_frame_sequence.size() << endl;
        counter++;
    }
    cout << receive_file_map.size() << endl;
    send_packet(sockfd,server,"GET_ALL_FILE",GET_ALL_FILE,long(USELESS_LENGTH));
    receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);    
    while(receive_packet.typePacket != GET_ALL_FILE_ACK){
        send_packet(sockfd,server,"GET_ALL_FILE",GET_ALL_FILE,long(USELESS_LENGTH));
        receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);        
        cout << receive_from_return_number << endl;
        if(resend_frame > MAX_SEND){
            cout << "Can not confirms whether server get GET_ALL_FILE signal. \n"; 
            break;
        }
        resend_frame++;
    }
    resend_frame = 0;
    // // Receive Missing frame
    // receive_missing_frame(sockfd,from,server);

    /**
     * Version 1
    for(long i = 0; i < total_frame; i++){
        if(i == total_frame){
            cout << "Download all file" << endl;
            break;
        }
        memset(&receive_packet, 0, sizeof(receive_packet));
        recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);  
		sendto(sockfd, &(receive_packet.packetSequence), sizeof(receive_packet.packetSequence), 0, (struct sockaddr *) &server, sizeof(server));	

		if ((receive_packet.packetSequence < i) || (receive_packet.packetSequence > i)) i--;
		else{
            if(i == total_frame - 1){
                time_t now = time(0);
                char* dt = ctime(&now);
                cout << "Time get the last frame is: " << dt << endl;
            }
            string str(receive_packet.dataBuffer);
            receive_file_map[receive_packet.packetSequence] = str;
            // receive_file_sequence.push_back(receive_packet.packetSequence);
            // fwrite(receive_packet.dataBuffer, 1, receive_packet.dataSize, file); 
        }
    }
    // fclose(file);
    */

    return;
}

void udp_client::receive_header(int sockfd, struct sockaddr_in from, struct sockaddr_in server){
    int header;
    struct packet header_packet;
    memset(&header_packet, 0, sizeof(header_packet));
    int receive_header_number = 0;

    // Receive Header
    header = recvfrom(sockfd, &(header_packet), sizeof(header_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
    while(header_packet.typePacket != SEND_FILE_STATUS){
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
    header_packet.typePacket = (enum packetType)SEND_FILE_STATUS_ACK;

    header = sendto(sockfd, &(header_packet), sizeof(header_packet), 0, (struct sockaddr* ) &server, sizeof(server));
    while(header < 0){
        header = sendto(sockfd, &(header_packet), sizeof(header_packet), 0, (struct sockaddr* ) &server, sizeof(server));
        if(receive_header_number >= 20){
            error("Error receive header from sender");
        }
        receive_header_number++;
    }

    return;
}

// void udp_client::receive_missing_frame(int sockfd, struct sockaddr_in from, struct sockaddr_in server){
//     struct packet receive_packet;
//     int receive_from_return_number;
//     memset(&receive_packet, 0, sizeof(receive_packet));

//     while(is_missing_frame(receive_file_sequence, missing_frame)){
//         request_to_resend_missing_frame(sockfd,server,from,missing_frame);
//         receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
//         while(receive_packet.typePacket != DONE_DOWNLOAD_MISSING){
//             if(receive_packet.packetSequence >= 0){
//                 string str(receive_packet.dataBuffer);
//                 receive_file_map[receive_packet.packetSequence] = str;
//                 receive_file_sequence.push_back(receive_packet.packetSequence);
//             }

//             memset(&receive_packet, 0, sizeof(receive_packet));

//             receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
//         }
//         send_packet(sockfd,server,"DONE_DOWNLOAD_MISSING_ACK",DONE_DOWNLOAD_MISSING_ACK,long(-10));
//         std::cout << "Check" << endl;

//     }
//     // send receive all frame signal
//     send_packet(sockfd, server, "Receive ALL Frames", DOWNLOAD_FINISH, long(-3));
//     std::cout << "Receive ALL Frames" << endl;

//     return;
// }

set<long> udp_client::missing_frame(vector<long> receive_file_sequence){
    std::cout << "Check missing_frame" << endl;
    std::set<long> missing_frame_set;
    // sort vector
    sort(receive_file_sequence.begin(),receive_file_sequence.end());
    long prev = 0;
    for(int i = 0; i < receive_file_sequence.size(); i++){
        if(i == 0 && receive_file_sequence[i] > prev){
            for(int j = prev; j < receive_file_sequence[i]; j++){
                missing_frame_set.insert(long(j));
            }
            prev = receive_file_sequence[i];
            continue;
        }
        if(prev == receive_file_sequence[i]) continue;
        if(prev + 1 < receive_file_sequence[i]){
            for(int j = prev+1; j < receive_file_sequence[i]; j++){
                missing_frame_set.insert(long(j));
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
            missing_frame_set.insert(long(k));
        }
    }

    
    return missing_frame_set;
}

void udp_client::request_to_resend_missing_frame(int sockfd, struct sockaddr_in server, struct sockaddr_in from, set<long> missing_frame_sequence){
    struct packet receive_packet;
    string send_string = "";
    int receive_num = 0;
    int timeout_counter = 0;
    std::set<long>::iterator it;

    for(it = missing_frame_sequence.begin(); it != missing_frame_sequence.end(); ++it){
        string current_missing = to_string(*it);
        if(send_string.length()+current_missing.length() <= DATABUFFER_SIZE - 1) {
            send_string = send_string + current_missing + ",";
        }
        else{
            send_string.pop_back();
            send_packet(sockfd, server, send_string, SEND_MISSING, send_string.length());
            send_string = "";
            send_string = send_string + current_missing + ","; 
        }
    }
    send_string.pop_back();
    send_packet(sockfd, server, send_string, SEND_MISSING_DONE, send_string.length());
    receive_num = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (sockaddr*)&from, (socklen_t *) & sockaddr_in_length);
    while(receive_packet.typePacket != SEND_MISSING_DONE_ACK){
        send_packet(sockfd, server, send_string, SEND_MISSING_DONE, send_string.length());
        receive_num = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (sockaddr*)&from, (socklen_t *) & sockaddr_in_length);
        if(timeout_counter > MAX_SEND){
            cout << "Can not confirm that whether client get missing frame sequence. \n";
            break;
        }
        timeout_counter++;
    }

    return;
}

bool udp_client::request_to_download_file(int sockfd, struct sockaddr_in server, struct sockaddr_in from){
    send_packet(sockfd,server,file_name,DOWNLOAD_FILE_REQUEST,long(USELESS_LENGTH));
    int downloadFile;
    int timeout = 0;
    struct packet DOWNLOAD_FILE_REQUEST_packet;
    memset(&DOWNLOAD_FILE_REQUEST_packet, 0, sizeof(DOWNLOAD_FILE_REQUEST_packet));
    cout << file_name << endl;
    send_packet(sockfd,server,file_name,DOWNLOAD_FILE_REQUEST,long(USELESS_LENGTH));
    // Receive Header
    downloadFile = recvfrom(sockfd, &(DOWNLOAD_FILE_REQUEST_packet), sizeof(DOWNLOAD_FILE_REQUEST_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
    while(DOWNLOAD_FILE_REQUEST_packet.typePacket != DOWNLOAD_FILE_REQUEST_ACK){
        send_packet(sockfd,server,file_name,DOWNLOAD_FILE_REQUEST,long(USELESS_LENGTH));
        downloadFile = recvfrom(sockfd, &(DOWNLOAD_FILE_REQUEST_packet), sizeof(DOWNLOAD_FILE_REQUEST_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
        if(timeout >= MAX_SEND){
            error("Error receive DOWNLOAD_FILE_REQUEST_packet from server");
        }
        timeout++;
    }
    timeout = 0;

    return true;
}

void udp_client::error(const char *msg){
    perror(msg);
    exit(0);
}

vector<string> udp_client::readFile(string fileName, long fileSize, long totalFrame){
    
    FILE *file;
    std::vector<string> v;
    char *buffer;
    long file_size = fileSize;

    file = fopen(fileName.c_str(),"rb");

    while(file_size > 0){
        int chunck = 0;
        if(file_size <= DATABUFFER_SIZE - 1) chunck = file_size;
        else chunck = DATABUFFER_SIZE - 1;
        buffer = new char[chunck+1];

        fread(buffer,1,chunck,file);
        buffer[chunck+1] = '\0';

        // for(int i = 0; i < strlen(buffer); i++){
        //     cout << buffer[i];
        // }
        // cout << endl;

        v.push_back(buffer);
        bzero(buffer,chunck+1);

        file_size -= chunck;
    }
    fclose(file);

    return v;
}

void udp_client::writeFile(map<long,string> file_map, string fileName){
    // std::map<long,string>::iterator it=file_map.begin();
    FILE *filetowrite;
	filetowrite=fopen(fileName.c_str(),"wb");
	// while(it != file_map.end()){
    //     fwrite(it->second.c_str(),sizeof(it->second),1,filetowrite);
    //     it++;
    // }
    for(auto t : file_map){
        cout << "Frame: " << t.first << endl;
        cout << "Size: " << t.second.length() << endl;
        cout << t.second << endl;
        fwrite(t.second.c_str(),sizeof(char),t.second.length(),filetowrite);
    }
    fclose(filetowrite);
    std::cout << "Write file done" << endl;
}