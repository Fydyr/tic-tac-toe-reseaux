#ifndef SOCKET_MANAGEMENT_H
#define SOCKET_MANAGEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int create_listen_socket(const int port, struct sockaddr_in *localEncounterPoint);
int create_communication_socket(const int port,const char ip[16], struct sockaddr_in *distantAddrSocket);
int read_message(int socket, char *message, int size);
int send_message(int socket, char *message);

#endif
