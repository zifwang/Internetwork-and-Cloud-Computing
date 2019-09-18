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
            // for (auto& t : receive_file_map){
            //     std::cout << t.first << " " << t.second <<  "\n";
            // }
            // std::cout << "Receive number of packets: " << receive_file_map.size() << endl;
            // std::cout << "Receive number of packets: " << receive_file_sequence.size() << endl;
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
    long ack_seq = 0;
    bool timeout_flag = false;

    // Start sending file to receiver
    for(size_t i = 0; i < fileVector.size(); i++){
        // cout << i << endl;
        // send_to_return_number = send_frame(sockfd,server,(long)i,fileVector[i]);
        // send_packet(sockfd,server,fileVector[i],UPLOAD,(long)i);
        // std::memset(&send_my_packet, 0, sizeof(send_my_packet));
        ack_seq = 0;
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
    std::cout << "Send all frames done" << endl;
    // send_packet(sockfd,server,"DONE_UPLOAD",DONE_UPLOAD,long(-1));
    // cout << "send first file done" << endl;

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
    send_header_packet.typePacket = (enum packetType)UPLOAD_HEADER_REQUEST;
    send_header_packet.dataSize = fileName.length();
    strcpy(send_header_packet.dataBuffer,fileName.c_str());

    // udp send
    send_header_number = sendto(sockfd,&(send_header_packet),sizeof(send_header_packet),0,(const struct sockaddr *) &server, sizeof(server));
    // udp receive
    send_header_number = recvfrom(sockfd, &(receive_header_ack), sizeof(receive_header_ack), 0, (struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    // Resend
    while(receive_header_ack.typePacket != UPLOAD_HEADER_REQUEST_ACK){
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
        tmp = MESSAGE; 
        send_packet(sockfd,server,"message",MESSAGE,long(-10));
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

    bool flag = receive_packet.typePacket != UPLOAD_REQUEST_ACK || receive_packet.typePacket != DOWNLOAD_REQUEST_ACK || receive_packet.typePacket != MESSAGE_ACK;

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
        
void udp_client::missing_frame_packet_interpreter(struct packet receive_packet, vector<long>& missing_frame){
    string missing_frame_string = "";
    for(int i = 0; i < (int)receive_packet.dataSize; i++){
        if(receive_packet.dataBuffer[i] == ','){
            missing_frame.push_back(stoi(missing_frame_string));
            missing_frame_string = "";
        }else{
            missing_frame_string = missing_frame_string + receive_packet.dataBuffer[i];
        }
    }

    return;
}

void udp_client::receive_file(){
    // Declare variables
    struct packet receive_packet;
    // struct packet send_packet;
    int receive_from_return_number;
    memset(&receive_packet, 0, sizeof(receive_packet));
    // FILE *file;
    // file = fopen(file_name.c_str(), "wb");
    // memset(&send_packet, 0, sizeof(send_packet));
    
    for(long i = 0; i < total_frame; i++){
        if(i == total_frame){
            cout << "Download all file" << endl;
        }
        memset(&receive_packet, 0, sizeof(receive_packet));
        recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);  
		sendto(sockfd, &(receive_packet.packetSequence), sizeof(receive_packet.packetSequence), 0, (struct sockaddr *) &server, sizeof(server));	

		if ((receive_packet.packetSequence < i) || (receive_packet.packetSequence > i)) i--;
		else{
            string str(receive_packet.dataBuffer);
            receive_file_map[receive_packet.packetSequence] = str;
            // receive_file_sequence.push_back(receive_packet.packetSequence);
            // fwrite(receive_packet.dataBuffer, 1, receive_packet.dataSize, file); 
        }
    }
    // fclose(file);


    // // Get file
    // receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
    // while(receive_packet.typePacket != DONE_DOWNLOAD){
    //     std::cout << receive_from_return_number << endl;
    //     if(receive_packet.packetSequence >= 0){
    //         string str(receive_packet.dataBuffer);
    //         receive_file_map[receive_packet.packetSequence] = str;
    //         receive_file_sequence.push_back(receive_packet.packetSequence);
    //     }
    //     std::cout << "Type: " << receive_packet.typePacket << endl;
    //     std::cout << "Sequence: " << receive_packet.packetSequence << endl;
    //     std::cout << "Size: " << receive_file_sequence.size() << endl;

    //     memset(&receive_packet, 0, sizeof(receive_packet));
    //     receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
    // }
    // send_packet(sockfd,server,"DONE_DOWNLOAD_ACK",DONE_DOWNLOAD_ACK,long(-10));

    // std::cout << "Get First file done" << endl;

    // // Receive Missing frame
    // receive_missing_frame(sockfd,from,server);

    return;
}

void udp_client::receive_header(int sockfd, struct sockaddr_in from, struct sockaddr_in server){
    int header;
    struct packet header_packet;
    memset(&header_packet, 0, sizeof(header_packet));
    int receive_header_number = 0;

    // Receive Header
    header = recvfrom(sockfd, &(header_packet), sizeof(header_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
    while(header_packet.typePacket != DOWNLOAD_HEADER_REQUEST){
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
    header_packet.typePacket = (enum packetType)DOWNLOAD_HEADER_REQUEST_ACK;

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

void udp_client::receive_missing_frame(int sockfd, struct sockaddr_in from, struct sockaddr_in server){
    struct packet receive_packet;
    int receive_from_return_number;
    memset(&receive_packet, 0, sizeof(receive_packet));

    while(is_missing_frame(receive_file_sequence, missing_frame)){
        request_to_resend_missing_frame(sockfd,server,from,missing_frame);
        receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
        while(receive_packet.typePacket != DONE_DOWNLOAD_MISSING){
            if(receive_packet.packetSequence >= 0){
                string str(receive_packet.dataBuffer);
                receive_file_map[receive_packet.packetSequence] = str;
                receive_file_sequence.push_back(receive_packet.packetSequence);
            }

            memset(&receive_packet, 0, sizeof(receive_packet));

            receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr*) &from, (socklen_t *) & sockaddr_in_length);
        }
        send_packet(sockfd,server,"DONE_DOWNLOAD_MISSING_ACK",DONE_DOWNLOAD_MISSING_ACK,long(-10));
        std::cout << "Check" << endl;

    }
    // send receive all frame signal
    send_packet(sockfd, server, "Receive ALL Frames", DOWNLOAD_FINISH, long(-3));
    std::cout << "Receive ALL Frames" << endl;

    return;
}

bool udp_client::is_missing_frame(vector<long> receive_file_sequence, vector<long> &missing_frame){
    std::cout << "Check is_missing_frame" << endl;
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
    std::cout << "flag: " << flag << endl;
    return flag;
}

void udp_client::request_to_resend_missing_frame(int sockfd, struct sockaddr_in server, struct sockaddr_in from, vector<long> missing_frame){
    string send_string = "";
    struct packet receive_packet;
    memset(&receive_packet,0,sizeof(receive_packet));
    int send_to_return_number = 0;
    int timeout = 0;

    for(int i = 0; i < missing_frame.size(); i++){
        string current_missing = to_string(missing_frame[i]);
        if(send_string.length() + current_missing.length() <= DATABUFFER_SIZE - 1){
            send_string = send_string + current_missing + ",";
        }
        else{
            // send packet
            send_string.pop_back();
            send_packet(sockfd, server, send_string, DOWNLOAD_MISSING, send_string.length());
            // set send_string to empty
            send_string = "";
            send_string = send_string + current_missing + ","; 
        }
    }
    // send last missing
    send_packet(sockfd, server, send_string, DOWNLOAD_MISSING, send_string.length());

    send_packet(sockfd, server, "DOWNLOAD_MISSING_SEND_DONE", DOWNLOAD_MISSING_SEND_DONE, send_string.length());
    send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    while(receive_packet.typePacket != DOWNLOAD_MISSING_ACK){
        send_packet(sockfd, server, "DOWNLOAD_MISSING_SEND_DONE", DOWNLOAD_MISSING_SEND_DONE, send_string.length());
        send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
        if(timeout > 20){
            error("timeout");
        }
        timeout++;
    }

    return;
}

bool udp_client::request_to_download_file(int sockfd, struct sockaddr_in server, struct sockaddr_in from){
    send_packet(sockfd,server,file_name,DOWNLOAD_FILE_REQUEST,long(-3));
    int downloadFile;
    int timeout = 0;
    struct packet DOWNLOAD_FILE_REQUEST_packet;
    memset(&DOWNLOAD_FILE_REQUEST_packet, 0, sizeof(DOWNLOAD_FILE_REQUEST_packet));
    // Receive Header
    downloadFile = recvfrom(sockfd, &(DOWNLOAD_FILE_REQUEST_packet), sizeof(DOWNLOAD_FILE_REQUEST_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
    while(DOWNLOAD_FILE_REQUEST_packet.typePacket != DOWNLOAD_FILE_REQUEST_ACK){
        send_packet(sockfd,server,file_name,DOWNLOAD_FILE_REQUEST,long(-3));
        downloadFile = recvfrom(sockfd, &(DOWNLOAD_FILE_REQUEST_packet), sizeof(DOWNLOAD_FILE_REQUEST_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
        if(timeout >= 20){
            error("Error receive DOWNLOAD_FILE_REQUEST_packet from server");
        }
        timeout++;
    }
    timeout = 0;

    send_packet(sockfd,server,file_name,DOWNLOAD_FILE_REQUEST_ACK_CLIENT,long(-3));
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

void udp_client::writeFile(map<long,string> receive_file_map, string file_name){
    std::map<long,string>::iterator it=receive_file_map.begin();
    FILE *filetowrite;
    file_name = file_name + "useFunc";
	filetowrite=fopen(file_name.c_str(),"w");
	while(it !=receive_file_map.end()){
		fwrite(it->second.c_str(),sizeof(it->second),1,filetowrite);
		it++;
	}
    fclose(filetowrite);
    cout << "Write file Done" << endl;
}