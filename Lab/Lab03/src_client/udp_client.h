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

        int send_frame(int sockfd, struct sockaddr_in server, long frame, string send_data);

        int send_done(int sockfd, struct sockaddr_in server);

        bool send_header(int sockfd, struct sockaddr_in server, struct sockaddr_in from, long totalFrames, string fileName);
        
        void missing_frame_packet_interpreter(struct packet receive_packet, vector<long>& missing_frame);

        void receive_file();

        int receive_header(int sockfd, struct sockaddr_in from, struct sockaddr_in server, struct packet &header_packet);
        // int receive_frame();

        void error(const char *msg);

        vector<string> readFile(string fileName, long fileSize, long totalFrame);

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
        char buffer[PACKET_SIZE];


        // file status
        string file_name = "";
        long file_size = 0;
        long total_frame = 0;
        map<long,string> receive_file_map;
        vector<long> missing_frame;

};

