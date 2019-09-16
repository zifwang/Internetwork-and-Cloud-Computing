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
    memset(&server, 0, sizeof(server));
	memset(&from, 0, sizeof(from));

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
    // send file
    if(access(file_name.c_str(),F_OK) == 0){
        // get file status
        struct stat statics; 
        stat(file_name.c_str(),&statics);
        file_size = statics.st_size;
        if((file_size % DATABUFFER_SIZE) != 0){
            total_frame = (file_size/DATABUFFER_SIZE) + 1;
        }else total_frame = (file_size/DATABUFFER_SIZE);
        printf("File size --> %ld and Total number of packets ---> %d \n", file_size, (int)total_frame);

        // send header and receive receiver confirmation
        if(!send_header(sockfd, server, from, total_frame, file_name)) error("Error send header");
        cout << "Success send header" << endl;

        // send file
        send_file();
    }
    else{
        cout << "enter correct file name" << endl;
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
    memset(&receive_packet, 0, sizeof(receive_packet));
    int send_to_return_number;
    socklen_t length = sizeof(struct sockaddr_in);
    int resend_frame = 0;

    // Start sending file to receiver
    for(int i = 0; i < fileVector.size(); i++){
        send_to_return_number = send_frame(sockfd,server,(long)i,fileVector[i]);
    }

    // Send Done file transmission done signal to receiver and this Done must be send
    send_to_return_number = send_done(sockfd,server);
    while(send_to_return_number < 0){
        cout << "Error send done and start resending" << endl;
        send_to_return_number = send_done(sockfd,server);
        if(resend_frame >= 200){
            resend_frame = 0;
            error("Error sending to receiver");
        }
        resend_frame++;
    }
    resend_frame = 0;

    send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    while(receive_packet.typePacket != DONE_ACK){
        missing_frame.clear();
        while(receive_packet.typePacket != MISSING_SEND_DONE){
            missing_frame_packet_interpreter(receive_packet,missing_frame);
            memset(&receive_packet, 0, sizeof(receive_packet));
            send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
        }
        // Send missing
        for(int i = 0; i < missing_frame.size(); i++){
            send_to_return_number = send_frame(sockfd,server,missing_frame[i],fileVector[int(missing_frame[i])]);
        }
        // Send Done file transmission done signal to receiver and this Done must be send
        send_to_return_number = send_done(sockfd,server);
        while(send_to_return_number < 0){
            cout << "Error send done and start resending" << endl;
            send_to_return_number = send_done(sockfd,server);
            if(resend_frame >= 200){
                resend_frame = 0;
                error("Error sending to receiver");
            }
            resend_frame++;
        }
        resend_frame = 0;
        // Get new receive
        send_to_return_number = recvfrom(sockfd,&receive_packet,sizeof(receive_packet),0,(struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    }
     
    return;
}

int udp_client::send_frame(int sockfd, struct sockaddr_in server, long frame, string send_data){
    int send_to_return_number;
    struct packet send_packet;
    // set packet 
    memset(&send_packet, 0, sizeof(send_packet));
    send_packet.packetSequence = frame;
    send_packet.typePacket = (enum packetType)SEND;
    send_packet.dataSize = send_data.length();
    strcpy(send_packet.dataBuffer,send_data.c_str());

    // udp send
    send_to_return_number = sendto(sockfd,&(send_packet),sizeof(send_packet),0,(const struct sockaddr *) &server, sizeof(server));

    return send_to_return_number;
}

int udp_client::send_done(int sockfd, struct sockaddr_in server){
    int send_to_return_number;
    struct packet send_done;
    string done = "DONE";
    // set packet
    memset(&send_done, 0, sizeof(send_done));
    send_done.packetSequence = long(-1);
    send_done.typePacket = (enum packetType)DONE;
    send_done.dataSize = 0;
    strcpy(send_done.dataBuffer,done.c_str());

    // udp send
    send_to_return_number = sendto(sockfd,&(send_done),sizeof(send_done),0,(const struct sockaddr *) &server, sizeof(server));

    return send_to_return_number;
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
    send_header_packet.typePacket = (enum packetType)REQUEST;
    send_header_packet.dataSize = fileName.length();
    strcpy(send_header_packet.dataBuffer,fileName.c_str());

    // udp send
    send_header_number = sendto(sockfd,&(send_header_packet),sizeof(send_header_packet),0,(const struct sockaddr *) &server, sizeof(server));
    // udp receive
    send_header_number = recvfrom(sockfd, &(receive_header_ack), sizeof(receive_header_ack), 0, (struct sockaddr *) &from, (socklen_t *) &sockaddr_in_length);
    // Resend
    while(receive_header_ack.typePacket != REQUEST_ACK){
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
    struct packet send_packet;
    int receive_from_return_number;
    memset(&receive_packet, 0, sizeof(receive_packet));
    memset(&send_packet, 0, sizeof(send_packet));
    receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
    while(receive_packet.typePacket != DONE){
        string str(receive_packet.dataBuffer);
        receive_file_map[receive_packet.packetSequence] = str;

        memset(&receive_packet, 0, sizeof(receive_packet));
        receive_from_return_number = recvfrom(sockfd, &(receive_packet), sizeof(receive_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
    }
}

int udp_client::receive_header(int sockfd, struct sockaddr_in from, struct sockaddr_in server, struct packet &header_packet){
    int header;
    header = recvfrom(sockfd, &(header_packet), sizeof(header_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
    int receive_header_number;
    while(header < 0){
        header = recvfrom(sockfd, &(header_packet), sizeof(header_packet), 0, (struct sockaddr* ) &from, (socklen_t *) &sockaddr_in_length);
        if(receive_header_number >= 20){
            error("Error receive header from sender");
        }
        receive_header_number++;
    }
    receive_header_number = 0;
    file_name = header_packet.dataBuffer;
    total_frame = header_packet.packetSequence;
    header_packet.typePacket = (enum packetType)REQUEST_ACK;

    header = sendto(sockfd, &(header_packet), sizeof(header_packet), 0, (struct sockaddr* ) &server, sizeof(server));
    while(header < 0){
        header = sendto(sockfd, &(header_packet), sizeof(header_packet), 0, (struct sockaddr* ) &server, sizeof(server));
        if(receive_header_number >= 20){
            error("Error receive header from sender");
        }
        receive_header_number++;
    }

    return header;
}

// int udp_client::receive_frame(){
//     int receive_from_frame;





//     return receive_from_frame;
// }

void udp_client::error(const char *msg){
    perror(msg);
    exit(0);
}

vector<string> udp_client::readFile(string fileName, long fileSize, long totalFrame){
    
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

