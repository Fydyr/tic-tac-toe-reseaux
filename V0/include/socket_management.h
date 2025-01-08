#ifndef SOCKET_MANAGEMENT_H
#define SOCKET_MANAGEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

int read_message(int socket, char *message, int size);
int send_message(int socket, char *message);

#endif
