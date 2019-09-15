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
    REQUEST,                        // sender requests receiver to receive file
    REQUEST_ACK,                    // receiver receives sending request from sender and confirm back to sender.
    MISSING,                        // receiver sends missing signal to sender to request resend
    MISSING_ACK,                    // sender receives resend request
    SEND,                           // sender starts to send packet
    SEND_ACK,                       // receuver gets packet
    DONE,                           // sender finishes sending 
    DONE_ACK,                       // receive confirms receiving finish
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
