#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <vector>

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

vector<string> readFile(string fileName, off_t fileSize, int totalFrame){
    
    FILE *file;
    vector<string> v;
    char *buffer;

    file = fopen(fileName.c_str(),"rb");

    while(fileSize > 0){
        int chunck = 0;
        if(fileSize <= DATABUFFER_SIZE - 1) chunck = fileSize;
        else chunck = DATABUFFER_SIZE - 1;
        buffer = new char[chunck+1];

        fread(buffer,1,chunck,file);
        buffer[chunck+1] = '\0';

        v.push_back(buffer);
        bzero(buffer,chunck);

        fileSize -= chunck;
    }

    return v;
}