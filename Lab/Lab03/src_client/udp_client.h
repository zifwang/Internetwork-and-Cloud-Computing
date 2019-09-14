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
#include "packet.h"

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

        void receive_file();

        void error(const char *msg);

        vector<string> readFile(string fileName, long fileSize, long totalFrame);

        /**
         * udp_client private variables
         */ 
        string udp_command = "";
        string file_name = "";
        string ip_address = "";
        int port_number = 0;
        int sockfd;
        int reading;
        struct sockaddr_in server, from;
        struct hostent *hp;
        char buffer[PACKET_SIZE];


        // file status
        long file_size = 0;
        long total_frame = 0;
};