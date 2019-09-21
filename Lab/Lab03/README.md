# Assignment: EE542 Lab Assignment 03: Fast, Reliable File Transfer

# Group: Zifan Wang & Yuhan Qiu

# Codes:
    src_client (support upload and download): codes used in file transfer client side.
        1. argParser.h/.cpp: input arguments parser.
        2. packet.h/.cpp: UDP packet's structure. 
        3. udp_client.h/.cpp: udp file transfer client class. This class support upload file to server and accepting download from server.
        4. client.cpp: main function of the program.
    
    src_server (support upload and download): codes used in file transfer server side.
        1. argParser.h/.cpp: input arguments parser.
        2. packet.h/.cpp: UDP packet's structure. 
        3. udp_server.h/.cpp: udp file transfer server class. This class support accepting upload from client and doing download to client.
        4. server.cpp: main function of the program.

# Run Codes:
    In client side:
        1. make clean
        2. make
        3. ./client -h: to see how to run the program or
        3. ./client -i ipAddress -p portNumber -f fileName -c command
            1). -i: ip address flag
            2). ipAddress: server's ip address
            3). -p: port number
            4). portNumber: server's port number
            5). -f: file
            6). fileName: file user want to transfer
            7). -c: command
            8). command: (upload, download)
    In server side:
        1. make clean
        2. make
        3. ./server -h : to see how to run the program or
        3. ./server -p portNumber
            1). -p: port number
            2). portNumber: server's port number
        
# Limitation:
    Only support the file can be read as string