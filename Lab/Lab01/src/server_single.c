/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>     // header file contains declarations used in most input and output
#include <stdlib.h>    
#include <string.h>    // String header
#include <unistd.h>    // header file provides access to the POSIX OS API (System call wrapper functions: fork, pipe and I/O primitives) 
#include <sys/types.h> // header file contains definitions of a number of data types used in system calls
#include <sys/socket.h>// header file socket.h includes a number of definitions of structures needed for sockets
#include <netinet/in.h>// header file in.h contains constants and structures needed for interent domain addresses

void error(const char *msg); // This function is called when a system call fails. 

int main(int argc, char *argv[]){

     int sockfd;        // file descriptor. Store the values returned by the socket system and the accept system call.
     int newsockfd;     // file descriptor. Store the values returned by the socket system and the accept system call.
     int portno;        // stores the port number on which the server accepts connections
     socklen_t clilen;  // stores the size of the address of the client. This is needed for the accept system call.
     char buffer[256];  // the server reads characters from the socket connection into this buffer.
     
     /**
     * A sockaddr_in is a structure containing an interent address. This structure is defined in netinet/in.h.
     * struct sockaddr_in{
     *  short   sin_family;         // must be AF_INET
     *  u_short sin_port;
     *  struct  in_addr sin_addr;
     *  char    sin_zero[8];        //Not used, must be zero
     *   };
     * in_addr structure, defined in the same header file, contains only one field, a unsigned long called s_addr.
     */
     struct sockaddr_in serv_addr, cli_addr;  // serv_addr will contain the address of the server, and cli_addr will contain the address of the client which connects to the server.

     int n;             // return value for the read() and write() calles; i.e. it contains the number of characters read or written
     
     // User need to pass in the port number on which the server will accept connections as an argument.
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     
     /**
     * The socket() system call creates a new socket. It takes three arguments. 
     * The first is the address domain of the socket: 
     *         There are two possible address domains, the unix domain for two processes which share a common file system, 
     *          and the Internet domain for any two hosts on the Internet. The symbol constant AF_UNIX is used for the unix domain, and AF_INET for the the Internet domain. 
     * The second argument is the type of socket: 
     *          1. stream socket (SOCK_STREAM) in which characters are read in a continuous stream as if from a file or pipe
     *          2. datagram socket (SOCK_DGRAM) in which messages are read in chunks.
     * The third argument is the protocol. If this argument is zero (and it always should be except for unusual circumstances), the operating system will choose the most appropriate protocol. It will choose TCP for stream sockets and UDP for datagram sockets.
     * The socket system call returns an entry into the file descriptor table (i.e. a small integer). This value is used for all subsequent references to this socket. If the socket call fails, it returns -1.
     * Reference: http://www.linuxhowtos.org/data/6/socket.txt
     */
     sockfd = socket(AF_INET, SOCK_STREAM, 0);  
     if (sockfd < 0) 
        error("ERROR opening socket");

     // The function bzero() sets all values in a buffer to zero. It takes two arguments, the first is a pointer to the buffer and the second is the size of the buffer. Thus, this line initializes serv_addr to zeros.
     bzero((char *) &serv_addr, sizeof(serv_addr));
     
     // The port number on which the server will listen for connections is passed in as an argument, and this statement uses the atoi() function to convert this from a string of digits to an integer.
     portno = atoi(argv[1]);  // provided by user

     // The first field is short sin_family, which contains a code for the address family. It should always be set to the symbolic constant AF_INET.
     serv_addr.sin_family = AF_INET;
     // This field contains the IP address of the host. For server code, this will always be the IP address of the machine on which the server is running, and there is a symbolic constant INADDR_ANY which gets this address.
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     // The second field of serv_addr is unsigned short sin_port, which contain the port number. However, instead of simply copying the port number to this field, it is necessary to convert this to network byte order using the function htons() which converts a port number in host byte order to a port number in network byte order.
     serv_addr.sin_port = htons(portno);

     /**
     * The bind() system call binds a socket to an address, in this case the address of the current host and port number on which the server will run.
     * It takes three arguments, the socket file descriptor, the address to which is bound, and the size of the address to which it is bound.
     * The second argument is a pointer to a structure of type sockaddr, but what is passed in is a structure of type sockaddr_in, and so this must be cast to the correct type. This can fail for a number of reasons, the most obvious being that this socket is already in use on this machine.
     * Return: 0: success. -1 : failure
     * Reference: http://www.linuxhowtos.org/data/6/bind.txt
     */
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding");
     
     /*
     * The listen system call allows the process to listen on the socket for connections. 
     * The first argument is the socket file descriptor, 
     * and the second is the size of the backlog queue, i.e., the number of connections that can be waiting while the process is handling a particular connection. This should be set to 5, the maximum size permitted by most systems. 
     * Return: 0 on success and -1 on failure
     * Reference: http://www.linuxhowtos.org/data/6/listen.txt
     */
     listen(sockfd,5);

     /*
     * The accept() system call causes the process to block until a client connects to the server. 
     * Thus, it wakes up the process when a connection from a client has been successfully established. 
     * It returns a new file descriptor, and all communication on this connection should be done using the new file descriptor. 
     * the second argument is a reference pointer to the address of the client on the other end of the connection,
     * and the third argument is the size of this structure.
     * Reference: http://www.linuxhowtos.org/data/6/accept.txt
     */ 
     clilen = sizeof(cli_addr);
     // return 0 success and -1 failure
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");

     // set buffer with size 256 to 0
     bzero(buffer,256);
     // reads from the socket. Note that the read call uses the new file descriptor, the one returned by accept(), not the original file descriptor returned by socket(). 
     // Note also that the read() will block until there is something for it to read in the socket, i.e. after the client has executed a write().
     n = read(newsockfd,buffer,255); // reference http://www.linuxhowtos.org/data/6/read.txt
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);

     // Once a connection has been established, both ends can both read and write to the connection. Naturally, everything written by the client will be read by the server, and everything written by the server will be read by the client. This code simply writes a short message to the client. The last argument of write is the size of the message.
     n = write(newsockfd,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");
     close(newsockfd);
     close(sockfd);
     return 0; 
}

void error(const char *msg){
    // This function is called when a system call fails. 
    // It displays a message about the error on stderr.
    // perror() produces a short  error  message  on  the  standard error describing the last error encountered during a call to a system or library function.
    // Reference http://www.linuxhowtos.org/data/6/perror.txt
    perror(msg); // perror, errno - system error messgaes

    // Aborts the program
    exit(1);
}

