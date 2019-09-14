#pragma once

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

void error(const char*);

vector<string> readFile(string fileName, off_t fileSize, int totalFrame);