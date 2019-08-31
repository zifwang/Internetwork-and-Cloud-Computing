#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> // defines the structure hostent

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;  // return value for the read() and write() calles; i.e. it contains the number of characters read or written
    struct sockaddr_in serv_addr; // The variable serv_addr will contain the address of the server to which we want to connect. It is of type struct sockaddr_in.
    /**
    * The variable server is a pointer to a structure of type hostent. This structure is defined in the header file netdb.h as follows:
    * struct  hostent {
    * char    *h_name;        // official name of host
    * char    **h_aliases;    // alias list 
    * int     h_addrtype;     // host address type 
    * int     h_length;       // length of address 
    * char    **h_addr_list;  // list of addresses from name server 
    * #define h_addr  h_addr_list[0]  // address, for backward compatiblity 
    * };
    * h_name       Official name of the host.
    * h_aliases    A zero  terminated  array  of  alternate names for the host.
    * h_addrtype   The  type  of  address  being  returned; currently always AF_INET.
    * h_length     The length, in bytes, of the address.
    * h_addr_list  A pointer to a list of network addresses for the named host. Host addresses are returned in network byte order.
    */
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    // Create client sockert
    portno = atoi(argv[2]);     // port number: stores the port number on which the server accepts connections
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    // server 
    server = gethostbyname(argv[1]);  // The variable argv[1] contains the name of a host on the Internet, e.g. cs.rpi.edu. Takes such a name as an argument and returns a pointer to a hostent containing information about that host. The field char *h_addr contains the IP address. If this structure is NULL, the system could not locate a host with this name.
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // copy from server->h_addr to serv_addr.sin_addr.s_addr (pass by reference)
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    serv_addr.sin_port = htons(portno);
    // The connect function is called by the client to establish a connection to the server. It takes three arguments, the socket file descriptor, the address of the host to which it wants to connect (including the port number), and the size of this address. This function returns 0 on success and -1 if it fails.
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}