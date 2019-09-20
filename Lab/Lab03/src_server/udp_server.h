#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include "packet.h"

using namespace std;

#define DEFAULTPORT 50000                 // A Default port number
#define MAX_SEND 200                       // The maximum number times can use to send a packet
#define USELESS_LENGTH -1             // 


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

        void send_packet(int sockfd, struct sockaddr_in from, string send_data, packetType type, long sequence);

        bool send_header(int sockfd, struct sockaddr_in from, long totalFrames, string fileName);
    
        void missing_frame_packet_interpreter(struct packet receive_packet, vector<long>& missing_frame);
        
        void receive_file();
        
        void receive_header(int sockfd, struct sockaddr_in from);

        void receive_missing_frame(int sockfd, struct sockaddr_in from);

        bool receive_user_request(int sockfd, struct sockaddr_in from);

        bool receive_user_download_file_request(int sockfd, struct sockaddr_in *from);

        bool is_missing_frame(vector<long> receive_file_sequence, vector<long> &missing_frame);

        void request_to_resend_missing_frame(int sockfd, struct sockaddr_in from, vector<long> missing_frame);

        void error(const char *msg);

        void printPacket(struct packet myPacket);

        vector<string> readFile(string fileName, long fileSize, long totalFrame);

        void writeFile(map<long,string> file_map, string fileName);

        /**
         * udp_server private client
         */
        // udp server variables
        int port_number = 0;
        string user_request = "";
        int sockfd;
        struct sockaddr_in server;
        struct sockaddr_in from;
        socklen_t sockaddr_in_length = sizeof(struct sockaddr_in);
        struct timeval time_out = {0, 0};

        // file status
        string file_name = "";
        long file_size = 0;
        long total_frame = 0;
        map<long,string> receive_file_map;
        vector<long> receive_file_sequence;
        vector<long> missing_frame;
};