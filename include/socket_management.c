#include "socket_management.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/**
 * Read a message from the socket
 * @param socket The socket to read
 * @param message The variable who must contain the message
 * @param size The size of the message
 * @return int The state of the read operation
 */
int read_message(int socket, char *message, int size) {
    int state;

    if (message == NULL) {
        fprintf(stderr, "Error : Message container is NULL.\n");
        close(socket);
        exit(EXIT_FAILURE);
    }

    // Get data from the socket
    state = recv(socket, message, size - 1, 0);

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
