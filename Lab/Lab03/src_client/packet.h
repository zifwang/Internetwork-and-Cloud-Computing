#pragma once

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DATABUFFER_SIZE 1025             // max size of data buffer in the packet
#define PACKET_ELEMENTS 4               // define how many elements in the packet struct
#define PACKET_SIZE 274                 // max size of packet

// Define type of message
enum packetType{
    MESSAGE_REQUEST,                 // client requests to transfer messages with server
    MESSAGE_REQUEST_ACK,             // Server gets MESSAGE_REQUEST from client and confirms it by sending MESSAGE_REQUEST_ACK to client
    DOWNLOAD_REQUEST,                // client requests to download file from server
    DOWNLOAD_REQUEST_ACK,            // Server gets DOWNLOAD_REQUEST from client and confirms it by sending DOWNLOAD_REQUEST_ACK to client
    DOWNLOAD_FILE_REQUEST,           // Client requests to download file with given file name
    DOWNLOAD_FILE_REQUEST_ACK,       // Server gets DOWNLOAD_FILE_REQUEST from client and confirms back by using DOWNLOAD_FILE_REQUEST_ACK signal
    UPLOAD_REQUEST,                  // client requests to upload file to server
    UPLOAD_REQUEST_ACK,              // Server gets UPLOAD_REQUEST from client and confirms it by sending UPLOAD_REQUEST_ACK to client
    SEND_FILE_STATUS,                // Sender (server: download case, client: upload case) sends file status to receiver (file name, total frame)
    SEND_FILE_STATUS_ACK,            // Receiver (client: download case, server: upload case) gets SEND_FILE_STATUS from sender and confirms back
    SEND_PACKET,                     // Sender sends packet to receive
    SEND_FILE_DONE,                  // Sender finishs sending all file.
    SEND_FILE_DONE_ACK               // Receiver confirms that it gets SEND_FILE_DONE signal from Sender.
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

// void socket_buffer_parser(struct packet inputPacket, char *outputBuffer);

// packet packet_parser(char *inputPackett);
