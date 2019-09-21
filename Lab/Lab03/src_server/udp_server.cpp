#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <ctime>
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
    // Receive timeout
    // time_out.tv_sec = 2;			
	// time_out.tv_usec = 0;
    // setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_out, sizeof(struct timeval));
}

void udp_server::run(){
    while(1){
        if(receive_user_request(sockfd,from)){
            /**
            * Client upload file part
            */
            if(user_request == "upload"){
                std::cout << "Start geting upload from user" << endl;
                receive_header(sockfd, from);
                std::cout << "Successfully Receive File Status:" << endl;
                std::cout << "                                  File Name: " << file_name << endl;
                std::cout << "                                  Total Frame: " << total_frame << endl;
                receive_file();
                std::cout << "Receive " << receive_file_map.size() << " numbers of frames and total frames is: " << total_frame << endl; 
                writeFile(receive_file_map,file_name);
            }

            /**
            * Client download file part
            */
            if(user_request == "download"){
                std::cout << "Start transfering file to user" << endl;
                bool flag = receive_user_download_file_request(sockfd,&from);
                if(access(file_name.c_str(),F_OK) == 0){
                    // get file status
                    struct stat statics; 
                    stat(file_name.c_str(),&statics);
                    file_size = statics.st_size;
                    if((file_size % (DATABUFFER_SIZE-1)) != 0){
                        total_frame = (file_size/(DATABUFFER_SIZE-1)) + 1;
                    }else total_frame = (file_size/(DATABUFFER_SIZE-1));
                    std::cout << "File requested to download is: " << file_name << endl;
                    printf("File size --> %ld and Total number of packets ---> %d \n", file_size, (int)total_frame);

                    // send header and receive receiver confirmation
                    if(!send_header(sockfd, from, total_frame, file_name)) error("Error send header");

                    // send file
                    send_file();
                }
                else{
                    // Send file list to user
                    std::cout << "enter correct file name" << endl;
                }
            }
            if(user_request == "message"){
                continue;
            }
        }
    }
    return;
}



/**
 * Private functions sections
 */

void udp_server::send_file(){
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
        send_packet(sockfd,from,fileVector[i],SEND_PACKET,long(i));  //send the frame
    }
    // Send SEND_FILE_DONE signal to client
    send_packet(sockfd,from,"SEND_FILE_DONE",SEND_FILE_DONE,long(USELESS_LENGTH));
    send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    while(receive_packet.typePacket != SEND_FILE_DONE_ACK){
        send_packet(sockfd,from,"SEND_FILE_DONE",SEND_FILE_DONE,long(USELESS_LENGTH));
        send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
        if(resend_frame > MAX_SEND){
            cout << "Missing Confirmations of Receiving FILE from Server \n";
        }
        resend_frame++;
    }
    resend_frame = 0;

    // missing_frame_sequence
    bool outFlag = false;
    vector<long> missing_frame_seq;
    missing_frame_seq.clear();
    
    send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    while(receive_packet.typePacket != GET_ALL_FILE){
        missing_frame_packet_interpreter(receive_packet,missing_frame_seq);
        while(receive_packet.typePacket != SEND_MISSING_DONE){
            send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
            if(receive_packet.typePacket == GET_ALL_FILE){
                outFlag = true;
                break;
            }
            missing_frame_packet_interpreter(receive_packet,missing_frame_seq);
        }
        if(outFlag) break;
        // Send confirmations
        for(int i = 0; i < MAX_SEND; i++){
            send_packet(sockfd,from,"SEND_MISSING_DONE_ACK",SEND_MISSING_DONE_ACK,long(USELESS_LENGTH));
        }
        
        // Send missing
        for(int i = 0; i < missing_frame_seq.size(); i++){
            send_packet(sockfd,from,fileVector[int(missing_frame_seq[i])],SEND_PACKET,missing_frame_seq[i]);
        }

        // Send confirms
        send_packet(sockfd,from,"SEND_FILE_DONE",SEND_FILE_DONE,long(USELESS_LENGTH));
        send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
        if(receive_packet.typePacket == GET_ALL_FILE) break;
        while(receive_packet.typePacket != SEND_FILE_DONE_ACK){
            send_packet(sockfd,from,"SEND_FILE_DONE",SEND_FILE_DONE,long(USELESS_LENGTH));
            send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
            if(receive_packet.typePacket == GET_ALL_FILE){
                outFlag = true;
                break;
            }
            if(resend_frame > MAX_SEND){
                cout << "Can not confirms whether client get SEND_FILE_DONE signal. \n"; 
            }
            resend_frame++;
        }
        if(outFlag) break;
        resend_frame = 0;
        missing_frame_seq.clear();
        send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    }
    for(int i = 0; i < MAX_SEND; i++){
        send_packet(sockfd,from,"GET_ALL_FILE_ACK",GET_ALL_FILE_ACK,long(USELESS_LENGTH));
    }

    std::cout << "Send Entire File Done" << endl;

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
    // std::cout << send_to_return_number << endl;
    while(send_to_return_number < 0){
        if(timeOut_couter > 20){
            error("Error in sending packet");
        }
        send_to_return_number = sendto(sockfd,&(packet_send),sizeof(packet_send),0,(const struct sockaddr *) &from, sizeof(from));
        timeOut_couter++;
    }

    return;
}

bool udp_server::send_header(int sockfd, struct sockaddr_in from, long totalFrames, string fileName){
    int send_header_number;
    int resend_header = 0;
    // struct packet send_header_packet;
    struct packet receive_header_ack;
    // memset(&send_header_packet, 0, sizeof(send_header_packet));
    memset(&receive_header_ack, 0, sizeof(receive_header_ack));
    
    // set packet
    // send_header_packet.packetSequence = totalFrames;
    // send_header_packet.typePacket = (enum packetType)SEND_FILE_STATUS;
    // send_header_packet.dataSize = fileName.length();
    // strcpy(send_header_packet.dataBuffer,fileName.c_str());

    // udp send
    // send_header_number = sendto(sockfd,&(send_header_packet),sizeof(send_header_packet),0,(const struct sockaddr *) &from, sizeof(from));
    send_packet(sockfd,from,fileName,SEND_FILE_STATUS,totalFrames);
    // udp receive
    send_header_number = recvfrom(sockfd, &(receive_header_ack), sizeof(receive_header_ack), 0, (struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    // Resend
    while(receive_header_ack.typePacket != SEND_FILE_STATUS_ACK){
        // send_header_number = sendto(sockfd,&(send_header_packet),sizeof(send_header_packet),0,(const struct sockaddr *) &from, sizeof(from));
        send_packet(sockfd,from,fileName,SEND_FILE_STATUS,totalFrames);
        send_header_number = recvfrom(sockfd, &(receive_header_ack), sizeof(receive_header_ack), 0, (struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
        resend_header++;
        if(resend_header >= 20){
            error("Error send header");
        }
    }
    resend_header = 0;

    // Check equality
    if(totalFrames == receive_header_ack.packetSequence && fileName.length() == receive_header_ack.dataSize) return true;    

    return false;
}

void udp_server::missing_frame_packet_interpreter(struct packet receive_packet, vector<long>& missing_frame_seq){
    string missing_frame_string = "";
    if(receive_packet.typePacket == SEND_MISSING || receive_packet.typePacket == SEND_MISSING_DONE){
        for(int i = 0; i < (int)receive_packet.dataSize; i++){
            if(receive_packet.dataBuffer[i] == ','){
                missing_frame_seq.push_back(stoi(missing_frame_string));
                missing_frame_string = "";
            }else{
                missing_frame_string = missing_frame_string + receive_packet.dataBuffer[i];
            }
        }
        missing_frame_seq.push_back(stoi(missing_frame_string));
    }
    return;
}

void udp_server::receive_file(){
    // // Declare variables
    struct packet receive_packet;
    // struct packet send_packet;
    int receive_from_return_number;
    int resend_frame = 0;
    memset(&receive_packet, 0, sizeof(receive_packet));
    // memset(&send_packet, 0, sizeof(send_packet));
    receive_file_map.clear();
    receive_file_sequence.clear();
    // FILE *file;
    // fopen(file_name.c_str(),"wb");
    // int unreceive_counter = 0;
    // Set receive timeout
    time_out.tv_sec = 2;			
	time_out.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_out, sizeof(struct timeval));

    // Get file
    receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
    if(receive_packet.typePacket == SEND_PACKET){
        string str(receive_packet.dataBuffer);
        receive_file_map[receive_packet.packetSequence] = str;
        receive_file_sequence.push_back(receive_packet.packetSequence);
    }
    while(receive_packet.typePacket != SEND_FILE_DONE){
        // std::cout << receive_from_return_number << endl;
        memset(&receive_packet, 0, sizeof(receive_packet));
        receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
        if(receive_packet.typePacket == SEND_PACKET){
            string str(receive_packet.dataBuffer);
            receive_file_map[receive_packet.packetSequence] = str;
            receive_file_sequence.push_back(receive_packet.packetSequence);
        }
        if(receive_from_return_number == -1) break;
    }
    // Time check
    time_t now = time(0);
    char* dt = ctime(&now);
    cout << "Time get the last frame is: " << dt << endl;

    // Send SEND_FILE_DONE_ACK to client
    for(int i = 0; i < MAX_SEND; i++){
        send_packet(sockfd,from,"SEND_FILE_DONE_ACK",SEND_FILE_DONE_ACK,long(USELESS_LENGTH));
    }

    // Check missing frame
    set<long> missing_frame_sequence = missing_frame(receive_file_sequence);
    while(missing_frame_sequence.size() != 0){
        // Send missing frame to client
        request_to_resend_missing_frame(sockfd,from,missing_frame_sequence);
        receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
        while(receive_packet.typePacket != SEND_FILE_DONE){
            if(receive_packet.typePacket == SEND_PACKET){
                string str(receive_packet.dataBuffer);
                receive_file_map[receive_packet.packetSequence] = str;
                missing_frame_sequence.erase(receive_packet.packetSequence);
            }
            receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
            if(receive_from_return_number < 0) break;
        }
        // Send SEND_FILE_DONE_ACK confirmations to client
        for(int i = 0; i < MAX_SEND; i++){
            send_packet(sockfd,from,"SEND_FILE_DONE_ACK",SEND_FILE_DONE_ACK,long(USELESS_LENGTH));
        }
    }

    // Time check
    time_t now_a = time(0);
    char* dt_a = ctime(&now_a);
    cout << "Time get the last missing frame is: " << dt << endl;

    // Send confirms that receive all frames
    send_packet(sockfd,from,"GET_ALL_FILE",GET_ALL_FILE,long(USELESS_LENGTH));
    receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);    
    while(receive_packet.typePacket != GET_ALL_FILE_ACK){
        send_packet(sockfd,from,"GET_ALL_FILE",GET_ALL_FILE,long(USELESS_LENGTH));
        receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);        
        if(resend_frame > MAX_SEND){
            cout << "Can not confirms whether server get GET_ALL_FILE signal. \n"; 
            break;
        }
        resend_frame++;
    }
    resend_frame = 0;

    return;
}

void udp_server::receive_header(int sockfd, struct sockaddr_in from){
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

bool udp_server::receive_user_request(int sockfd, struct sockaddr_in from){
    struct packet receive_packet;
    int receive_from_return_number;
    memset(&receive_packet, 0, sizeof(receive_packet));

    // Get request
    receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);

    if(receive_packet.typePacket == DOWNLOAD_REQUEST || receive_packet.typePacket == UPLOAD_REQUEST || receive_packet.typePacket == MESSAGE_REQUEST){
        user_request = string(receive_packet.dataBuffer);
        std::cout << "user_request: " << user_request << endl;
        if(receive_packet.typePacket == DOWNLOAD_REQUEST){
            send_packet(sockfd, from, user_request, DOWNLOAD_REQUEST_ACK, long(-9));
        }
        if(receive_packet.typePacket == UPLOAD_REQUEST){
            send_packet(sockfd, from, user_request, UPLOAD_REQUEST_ACK, long(-9));
        }
        if(receive_packet.typePacket == MESSAGE_REQUEST){
            send_packet(sockfd, from, user_request, MESSAGE_REQUEST_ACK, long(-9));
        }
        return true;
    }

    return false;
}

bool udp_server::receive_user_download_file_request(int sockfd, struct sockaddr_in *from){
    struct packet receive_packet;
    int receive_from_return_number;
    memset(&receive_packet, 0, sizeof(receive_packet));
    // Get request
    receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (sockaddr*)from, (socklen_t *) & sockaddr_in_length);
    file_name = string(receive_packet.dataBuffer);
    while(receive_packet.typePacket != DOWNLOAD_FILE_REQUEST){
        memset(&receive_packet, 0, sizeof(receive_packet));
        receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (sockaddr*)from, (socklen_t *) & sockaddr_in_length);
        file_name = string(receive_packet.dataBuffer);
    }
    // Check file
    if(access(file_name.c_str(),F_OK) == 0) {
        send_packet(sockfd,*from,file_name,DOWNLOAD_FILE_REQUEST_ACK,long(-3));
        return true;
    }
    else{
        // Send file list to client
        // MOD Later
        cout << "File does not existed" << endl;
    }
    return false;
}

set<long> udp_server::missing_frame(vector<long> receive_file_sequence){
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

void udp_server::request_to_resend_missing_frame(int sockfd, struct sockaddr_in from, set<long> missing_frame_sequence){
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
            send_packet(sockfd, from, send_string, SEND_MISSING, send_string.length());
            send_string = "";
            send_string = send_string + current_missing + ","; 
        }
    }
    send_string.pop_back();
    send_packet(sockfd, from, send_string, SEND_MISSING_DONE, send_string.length());
    receive_num = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (sockaddr*)&from, (socklen_t *) & sockaddr_in_length);
    while(receive_packet.typePacket != SEND_MISSING_DONE_ACK){
        send_packet(sockfd, from, send_string, SEND_MISSING_DONE, send_string.length());
        receive_num = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (sockaddr*)&from, (socklen_t *) & sockaddr_in_length);
        if(timeout_counter > MAX_SEND){
            cout << "Can not confirm that whether client get missing frame sequence. \n";
            break;
        }
        timeout_counter++;
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
    char *buffer = new char[DATABUFFER_SIZE];

    file = fopen(fileName.c_str(),"rb");
    int framecounter = 0;

    while(fileSize > 0){
        int chunck = 0;
        if(fileSize <= DATABUFFER_SIZE - 1) chunck = fileSize;
        else chunck = DATABUFFER_SIZE - 1;

        fread(buffer,1,chunck,file);
        // cout << "Frame: " << totalFrame << endl;
        // for(int i = 0; i < strlen(buffer); i++){
        //     cout << buffer[i];
        // }
        // cout << endl;
        string myString = string(buffer);
        v.push_back(myString);
        // cout << "Frame: " << framecounter << endl;
        // cout << "Size: " << myString.size() << endl;
        // cout << myString << endl;

        bzero(buffer,DATABUFFER_SIZE);

        fileSize -= chunck;
        framecounter++;
    }
    fclose(file);

    return v;
}

void udp_server::printPacket(struct packet myPacket){
    std::cout << "File Sequence: " << myPacket.packetSequence << endl;
    std::cout << "File Type: " << myPacket.typePacket << endl;
    std::cout << "File DataSize: " << myPacket.dataSize << endl;
    std::cout << "File Data: " << string(myPacket.dataBuffer) << endl;
}

void udp_server::writeFile(map<long,string> file_map, string fileName){
    // std::map<long,string>::iterator it=file_map.begin();
    FILE *filetowrite;
	filetowrite=fopen(fileName.c_str(),"wb");
	// while(it != file_map.end()){
    //     fwrite(it->second.c_str(),sizeof(it->second),1,filetowrite);
    //     it++;
    // }
    for(auto t : file_map){
        // cout << "Frame: " << t.first << endl;
        // cout << "Size: " << t.second.length() << endl;
        // cout << t.second << endl;
        fwrite(t.second.c_str(),sizeof(char),t.second.length(),filetowrite);
    }
    fclose(filetowrite);
    std::cout << "Write file done" << endl;
}