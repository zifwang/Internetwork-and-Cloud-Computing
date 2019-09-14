#include <sys/types.h>
#include <sys/socket.h>
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
    memset(&server, 0, sizeof(send_addr));
	memset(&from, 0, sizeof(from_addr));

    // init socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) error("Cannot create socket");
    server.sin_family = AF_INET;
    server.sin_port = htons(port_number);
    hp = gethostbyname(ip_address);
    if (hp==0) error("Unknown host");
    bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
}

void udp_client::send_file(){
    // check whether file is existed



    // Get file status: file_size, total frame
    struct stat statics; 
    stat(file_name.c_str(),&statics);
    file_size = statics.st_size;
    if((file_size % DATABUFFER_SIZE) != 0){
        total_frame = (file_size/DATABUFFER_SIZE) + 1;
    }else total_frame = (file_size/DATABUFFER_SIZE);
    
    // Read file to a file vector
    vector<string> fileVector = readFile(file_name, file_size, total_frame);

    // Start sending file to receiver

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
void udp_client::error(const char *msg){
    perror(msg);
    exit(0);
}

vector<string> udp_client::readFile(string fileName, off_t fileSize, int totalFrame){
    
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

    return v;
}
