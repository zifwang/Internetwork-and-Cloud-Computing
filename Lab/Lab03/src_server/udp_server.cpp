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
#include "udp_server.h"

using namespace std;

udp_server::udp_server(){
    // set variables

}

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
    if (bind(sockfd,(struct sockaddr *)&server,sizeof(server))<0) error("binding");
}


/**
 * Private functions sections
 */

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

