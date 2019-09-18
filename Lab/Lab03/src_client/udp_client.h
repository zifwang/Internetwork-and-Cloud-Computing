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
#include <iterator>
#include "packet.h"

#define DEFAULTPORT 50000                 // A Default port number

using namespace std;

class udp_client{
    public:
        udp_client(string ipAddress, int portNumber, string command, string fileName);

        void run();

        string get_ip_address();
        int get_connection_port_number();
        string get_file_name();
        string get_command();


    private:
        /**
         * udp_client private functions
         */
        void send_file();

        // int send_frame(int sockfd, struct sockaddr_in server, long frame, string send_data);

        // int send_done(int sockfd, struct sockaddr_in server);

        bool send_header(int sockfd, struct sockaddr_in server, struct sockaddr_in from, long totalFrames, string fileName);
        
        void send_packet(int sockfd, struct sockaddr_in from, string send_data, packetType type, long sequence);

        bool send_user_request(int sockfd, struct sockaddr_in server, struct sockaddr_in from, string command);
        
        void missing_frame_packet_interpreter(struct packet receive_packet, vector<long>& missing_frame);

        void receive_file();

        void receive_header(int sockfd, struct sockaddr_in from, struct sockaddr_in server);
        
        void receive_missing_frame(int sockfd, struct sockaddr_in from, struct sockaddr_in server);
        
        bool is_missing_frame(vector<long> receive_file_sequence, vector<long> &missing_frame);

        void request_to_resend_missing_frame(int sockfd, struct sockaddr_in server, struct sockaddr_in from, vector<long> missing_frame);

        bool request_to_download_file(int sockfd, struct sockaddr_in server, struct sockaddr_in from);

        void error(const char *msg);

        vector<string> readFile(string fileName, long fileSize, long totalFrame);

        void writeFile(map<long,string> receive_file_map, string file_name);

        /**
         * udp_client private variables
         */ 
        string udp_command = "";
        string ip_address = "";
        int port_number = 0;
        int sockfd;
        int reading;
        struct sockaddr_in server, from;
        socklen_t sockaddr_in_length = sizeof(struct sockaddr_in);
        struct hostent *hp;
        // char buffer[PACKET_SIZE];


        // file status
        string file_name = "";
        long file_size = 0;
        long total_frame = 0;
        map<long,string> receive_file_map;
        vector<long> receive_file_sequence;
        vector<long> missing_frame;

};

