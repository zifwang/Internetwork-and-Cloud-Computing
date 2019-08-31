# Assignment: EE542 Lab Assignment 01

# Group: Zifan Wang & Yuhan Qiu

# Codes:
    1. client_udp.c:    UDP client 
    2. server_udp.c:    UDP server (The simple Internet version: run forever)
    3. client.c:        TCP client
    4. server.c:        TCP server (The simple Internet version: run forever)
    5. server_single.c: TCP server (Only run once)

# Steps:
    1. Connect two computers or laptops into the same network (Wired Ethernet or WiFi). 
    2. Find IP addresses for both computers in the local network by using following command in Linux terminal: 
                        ip address. 
    The IP address will look like 192.168.xxx.xxx or 10.0.xxx.xxx. 
    3. Assume the first computer's IP address is IP1 and the second computer's IP address is IP2.
    4. Set one of the computer to be the server and run following command:
                        g++ -o outputFileName serverFileName
       ouputFileName can be varied
       serverFileName must in: server.c server_single.c server_udp.c
       Then, run the following command:
                        ./ouputFileName serverPortNumber
       serverPortNumber: opening port for listening
       e.g. g++ -o server server.c
            ./server 51717
    5. Set another computer to be the client and run following command:
                        g++ -o outputFileName clientFileName
       ouputFileName can be varied
       clientFileName must in: client.c client_udp.c
       Then, run the following command:
                        ./ouputFileName IP1(server's IP address, see step 3) portNumber
       serverPortNumber: server's listening port
       e.g. g++ -o client client.c
            ./client 192.169.0.210 51717
    6. The client will prompt you to enter a message. 
       If everything works correctly, the server will display your message on stdout, send an acknowledgement message to the client and terminate.  
       The client will print the acknowledgement message from the server and then terminate.
       