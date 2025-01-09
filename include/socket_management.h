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

/**
 * Create a socket that listens on a specified port.
 * @param port the port to set
 * @param localEncounterPoint structure to store the address and port information
 * @return The listen socket descriptor
 */
int create_listen_socket(const int port, struct sockaddr_in *localEncounterPoint);
/**
 * Create a socket and connect to a distant server using a given port and IP address.
 * @param port the port to connect to
 * @param ip the IP address of the distant server
 * @param distantAddrSocket structure to store the address and port information
 * @return The connected socket descriptor
 */
int create_communication_socket(const int port,const char ip[16], struct sockaddr_in *distantAddrSocket);
/**
 * Read a message from the socket
 * @param socket The socket to read
 * @param message The variable who must contain the message
 * @param size The size of the message
 * @return int The state of the read operation
 */
int read_message(int socket, char *message, int size, int read_only);

/**
 * Send a message through a socket
 * @param socket The socket through which the message will be sent
 * @param message The message to send
 * @return int The status of the send operation
 */
int send_message(int socket, char *message);

#endif
