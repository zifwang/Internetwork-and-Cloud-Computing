#pragma once

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DATABUFFER_SIZE 256             // max size of data buffer in the packet
#define PACKET_ELEMENTS 4               // define how many elements in the packet struct
#define PACKET_SIZE 274                 // max size of packet

// Define type of message
enum packetType{
    MESSAGE,                                 // client sends message to server
    MESSAGE_ACK,                             // server confirms MESSAGE request from client
    DOWNLOAD_REQUEST,                        // client sends download request to server to download file
    DOWNLOAD_REQUEST_ACK,                    // server receives download request from client and confirms it
    DOWNLOAD_HEADER_REQUEST,                 // server sends header request to client
    DOWNLOAD_HEADER_REQUEST_ACK,             // client receives DOWNLOAD_HEADER_REQUEST and confirms to server
    DOWNLOAD_FILE_REQUEST,                   // client send DOWNLOAD_FILE_REQUEST to server with provided filename
    DOWNLOAD_FILE_REQUEST_ACK,               // server receives DOWNLOAD_FILE_REQUEST and confirm it
    DOWNLOAD_FILE_REQUEST_ACK_CLIENT,        // client confirm DOWNLOAD_FILE_REQUEST_ACK
    UPLOAD_REQUEST,                          // client send upload request to server to upload file
    UPLOAD_REQUEST_ACK,                      // server receives upload request from client and confirms it
    UPLOAD_HEADER_REQUEST,                   // client sends request to upload header to server
    UPLOAD_HEADER_REQUEST_ACK,               // server receives UPLOAD_HEADER_REQUEST and confirms to client
    DOWNLOAD_MISSING,                        // client sends missing signal to server to request resend
    DOWNLOAD_MISSING_SEND_DONE,              // client sends missing done signal to server 
    DOWNLOAD_MISSING_ACK,                    // server receives resend request
    UPLOAD_MISSING,                          // server sends missing signal to server to request client
    UPLOAD_MISSING_SEND_DONE,                // server sends missing done signal to client 
    UPLOAD_MISSING_ACK,                      // client receives resend request
    UPLOAD,                                  // client starts to uploading packet
    DOWNLOAD,                                // server starts to send packet and client downloading it
    DONE_UPLOAD,                             // client finishes send packets 
    DONE_DOWNLOAD,                           // server finishes send packets
    DONE_UPLOAD_ACK,                         // server confirms receiving client finishes send packets 
    DONE_DOWNLOAD_ACK,                       // client confirms receiving server finishes send packets 
    DONE_UPLOAD_MISSING,                     // client finishes sending missing packets
    DONE_DOWNLOAD_MISSING,                   // server finishes sending missing packets
    DONE_UPLOAD_MISSING_ACK,                 // client finishes sending missing packets and server confirms receive
    DONE_DOWNLOAD_MISSING_ACK,               // server finishes sending missing packets and client confirms receive
    UPLOAD_FINISH,                           // server confirms receive the entire file
    DOWNLOAD_FINISH,                         // client confirms receive the entire file
    UPLOAD_FINISH_ACK,                       // client confirms receive finish signal
    DOWNLOAD_FINISH_ACK                      // server confirms receive finish signal
};

// Define packet 
struct packet
{
    /* data */
    char dataBuffer[DATABUFFER_SIZE];
    long dataSize;
    enum packetType typePacket;
    long packetSequence;
};

void socket_buffer_parser(struct packet inputPacket, char *outputBuffer);

packet packet_parser(char *inputPackett);
