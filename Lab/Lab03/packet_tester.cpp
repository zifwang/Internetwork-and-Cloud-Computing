#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "packet.h"

using namespace std;

int main(){


    packet inputPacket;
    packet outputPacket;
    char outputBuffer[274];
    char data[256];
    for(int i = 0; i < 254; i = i+2){
        data[i] = 'a';
        data[i+1] = ',';
    }
    data[254] = '\0';


    strcpy(inputPacket.dataBuffer,data);
    inputPacket.packetSequence = 100000;
    inputPacket.typePacket = REQUEST;
    inputPacket.dataSize = 254;

    socket_buffer_parser(inputPacket,outputBuffer);

    outputPacket = packet_parser(outputBuffer);

    cout << outputPacket.typePacket << endl;
    cout << outputPacket.packetSequence << endl;
    cout << outputPacket.dataSize << endl;
    for(int i = 0; i < outputPacket.dataSize; i++){
        cout << outputPacket.dataBuffer[i];
    }
    cout << endl;

    for(int i = 0; i < 250; i = i+2){
        data[i] = 'b';
        data[i+1] = ',';
    }
    data[250] = '\0';
    strcpy(inputPacket.dataBuffer,data);
    inputPacket.packetSequence = 100001;
    inputPacket.typePacket = REQUEST;
    inputPacket.dataSize = 250;

    socket_buffer_parser(inputPacket,outputBuffer);

    outputPacket = packet_parser(outputBuffer);


    cout << outputPacket.typePacket << endl;
    cout << outputPacket.packetSequence << endl;
    cout << outputPacket.dataSize << endl;
    for(int i = 0; i < outputPacket.dataSize; i++){
        cout << outputPacket.dataBuffer[i];
    }
    cout << endl;


    return 0;
}