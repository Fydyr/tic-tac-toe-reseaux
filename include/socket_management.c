/**
 * @author
 * Behani Julien
 * Mallevaey Adrien
 * Fournier Enzo
 * Alluin Edouard
 */

#include "socket_management.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>


int create_listen_socket(const int port, struct sockaddr_in *localEncounterPoint) {
    
    // Creation of listening socket
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Initialize sockaddr_in structure
    socklen_t addrLength = sizeof(*localEncounterPoint);
    memset(localEncounterPoint, 0, addrLength);
    localEncounterPoint->sin_family = AF_INET;
    localEncounterPoint->sin_addr.s_addr = htonl(INADDR_ANY);
    localEncounterPoint->sin_port = htons(port);

    // Associate socket with specified port
    if (bind(listenSocket, (struct sockaddr *)localEncounterPoint, addrLength) < 0) {
        perror("bind");
        close(listenSocket);
        exit(EXIT_FAILURE);
    }

    return listenSocket;
}

int create_communication_socket(const int port, const char ip[16], struct sockaddr_in *distantAddrSocket)
{
    int descriptorSocket;
    socklen_t addrLength;

    descriptorSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (descriptorSocket < 0)
    {
        perror("Error occuring during the creation of the socket...");
        exit(EXIT_FAILURE);
    }
    printf("Socket created! (%d)\n", descriptorSocket);

    addrLength = sizeof(*distantAddrSocket);
    memset(distantAddrSocket, 0, addrLength);

    distantAddrSocket->sin_family = AF_INET;
    distantAddrSocket->sin_port = htons(port);
    inet_aton(ip, &distantAddrSocket->sin_addr);

    if (connect(descriptorSocket, (struct sockaddr *)distantAddrSocket, addrLength) == -1)
    {
        perror("Connection error with the distant server...");
        close(descriptorSocket);
        exit(-2);
    }
    printf("Connection to the server %s:%d is successfull !\n", ip, port);

    return descriptorSocket;
}

int read_message(int socket, char *message, int size, int read_only) {
    int state;

    if (message == NULL) {
        fprintf(stderr, "Error : Message container is NULL.\n");
        close(socket);
        exit(EXIT_FAILURE);
    }

    // Get data from the socket
    state = recv(socket, message, size - 1, (read_only == 1) ? MSG_PEEK : 0);

    switch (state) {
        case -1: // An error has occurred
            perror("recv");
            close(socket);
            exit(EXIT_FAILURE);

        case 0: // The socket is closed
            fprintf(stderr, "Socket has closed by the client.\n");
            close(socket);
            return 0;

        default: // Data have been recovered
            message[state] = '\0';
            printf("Recovered message : %s (%d bits)\n", message, state);
            return 1;
    }
}

int send_message(int socket, char *message)
{
	int state;
	switch(state = send(socket, message, strlen(message)+1, 0)){
			case -1 : /* an error ! */
				perror("Error in writing...");
				close(socket);
				exit(-3);
			case 0 : /* socket is closed */
				fprintf(stderr, "The socket has been closed !\n\n");
			    close(socket);
                return 0;
			default: /* envoi de n octets */
				printf("Sending responses : %s (%d bits)\n\n", message, state);
                return 1;
		}
}
