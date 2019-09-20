#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "packet.h"

using namespace std;

// void socket_buffer_parser(struct packet inputPacket, char *outputBuffer){
//     char packet_info[DATABUFFER_SIZE];
//     sprintf(packet_info, "%d,%d,%d,%s", (int)inputPacket.typePacket,(int)inputPacket.packetSequence,(int)inputPacket.dataSize,inputPacket.dataBuffer);
    
//     strcpy(outputBuffer,packet_info);
// }

// packet packet_parser(char *inputPacket){
//     packet outPacket;
//     int separatorCounter = 0;
//     string packet_type = "";
//     string packet_sequence = "";
//     string packet_data_size = "";
//     string packet_data = "";
//     int i = 0;

//     while(inputPacket[i] != '\0'){
//         if(inputPacket[i] == ','){
//             if(separatorCounter > 2){
//                 packet_data += inputPacket[i];
//             }
//             separatorCounter++;
//         }else{
//             if(separatorCounter == 0){
//                 packet_type += inputPacket[i];
//             }
//             else if(separatorCounter == 1){
//                 packet_sequence += inputPacket[i];
//             }
//             else if(separatorCounter == 2){
//                 packet_data_size += inputPacket[i];
//             }
//             else{
//                 packet_data += inputPacket[i];
//             }
//         }
//         i++;
//     }

//     outPacket.dataSize = stoi(packet_data_size);
//     outPacket.packetSequence = stoi(packet_sequence);
//     outPacket.typePacket = (enum packetType)(stoi(packet_type));
//     strcpy(outPacket.dataBuffer,packet_data.c_str());

//     return outPacket;
// }
