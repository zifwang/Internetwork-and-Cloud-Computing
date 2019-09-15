#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <map>
#include "packet.h"

using namespace std;

class udp_server{
    public:
        udp_server();

        udp_server(int portNumber);

        void run();

        int get_listen_port_number();

        string get_file_name();






    private:
        /**
         * udp_server private functions
         */
        void send_file();

        void receive_file();
        
        void receive_header(int sockfd, struct sockaddr_in fromr);

        
        void error(const char *msg);

        vector<string> readFile(string fileName, long fileSize, long totalFrame);

        /**
         * udp_server private client
         */
        // udp server variables
        int port_number = 0;
        int sockfd;
        struct sockaddr_in server;
        struct sockaddr_in from;
        socklen_t sockaddr_in_length = sizeof(struct sockaddr_in);
        char buffer[PACKET_SIZE];

        // file status
        string file_name = "";
        long file_size = 0;
        long total_frame = 0;
        map<long,string> receive_file_map;

};