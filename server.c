/**
 * @author
 * Behani Julien
 * Mallevaey Adrien
 * Fournier Enzo
 * Alluin Edouard
 */

#include <stdio.h>
#include <stdlib.h> /* pour exit */
#include <unistd.h> /* pour read, write, close, sleep */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>		/* pour memset */
#include <netinet/in.h> /* pour struct sockaddr_in */
#include <arpa/inet.h>	/* pour htons et inet_aton */
#include <signal.h>
#include <time.h>
#include <threads.h>	/* pour créer des threads*/
#include <stdatomic.h>	/* pour générer une mémoire sécurisée entre threads*/
#include <omp.h>	/* pour le parrallelisme du code */
#include "include/socket_management.h"
#include "include/tictactoe.h"

#define MIN_PORT 5000 //(ports >= 5000 réservés pour usage explicite)
#define MAX_PORT 5005 //(Port maximal pouvant être utilisé)
#define LG_MESSAGE 256

#define QUEUE_SIZE 10 // Taille de la liste de demande de spectateurs
#define SPECTATOR_SIZE 30 // Taille de la liste de spectateurs

typedef struct {
    int sockets[QUEUE_SIZE];
	int spectators[SPECTATOR_SIZE];
    int front;
    int rear;
    int count;
	int unused_count;
    mtx_t mutex;
    cnd_t cond;
} SocketQueue;

// Global queue for the threads
SocketQueue queue;

// Shared flag to indicate cancellation
atomic_int cancel_flag = 0;

// Initialize the queue
void queue_init(SocketQueue *queue)
{
    queue->front = queue->rear = queue->count = 0;
    queue->unused_count = SPECTATOR_SIZE;
	for(int i=0; i<SPECTATOR_SIZE; i++)
	{
		queue->spectators[i] = 0;
	}
    mtx_init(&queue->mutex, mtx_plain);
    cnd_init(&queue->cond);
}

// Destroy the queue
void queue_destroy(SocketQueue *queue)
{
	for (int i=0; i<SPECTATOR_SIZE; i++)
	{
		if (queue->spectators[i] != 0)
		{
			close(queue->spectators[i]);
		}
	}
    mtx_destroy(&queue->mutex);
    cnd_destroy(&queue->cond);
}

// Enqueue a client socket (Producer)
void queue_push(SocketQueue *queue, int client_socket)
{
    mtx_lock(&queue->mutex);

    // Wait if the queue is full
    while (queue->count == QUEUE_SIZE) {
        cnd_wait(&queue->cond, &queue->mutex);
    }

    queue->sockets[queue->rear] = client_socket;
    queue->rear = (queue->rear + 1) % QUEUE_SIZE;
    queue->count++;

    cnd_signal(&queue->cond); // Notify the consumer
    mtx_unlock(&queue->mutex);
}

int queue_pop(SocketQueue *queue)
{
    mtx_lock(&queue->mutex);

    // Wait if the queue is empty
    while (queue->count == 0) {
        cnd_wait(&queue->cond, &queue->mutex);
    }

    int client_socket = queue->sockets[queue->front];
    queue->front = (queue->front + 1) % QUEUE_SIZE;
    queue->count--;

    cnd_signal(&queue->cond); // Notify the producer
    mtx_unlock(&queue->mutex);

    return client_socket;
}

// Dequeue a client socket (Consumer)
void update_spectator_list(SocketQueue *queue, int *socket_server)
{
	int error = 0;
	int spectator_socket;
	int count = queue->count;
	int new_socket = queue_pop(queue);
	char message[LG_MESSAGE];
    socklen_t len = sizeof(error);

    mtx_lock(&queue->mutex);
	queue->unused_count = SPECTATOR_SIZE;
    for (int i = 0; i < SPECTATOR_SIZE; i++)
	{
		spectator_socket = queue->spectators[i];
		if (getsockopt(spectator_socket, SOL_SOCKET, SO_ERROR, &error, &len) < 0 && count != 0){
			queue->spectators[i] = new_socket;
			count--;
			queue->unused_count--;
		}
		else if (getsockopt(spectator_socket, SOL_SOCKET, SO_ERROR, &error, &len) < 0 && count == 0){
			queue->spectators[i] = 0;
		}
		else if (queue->count != 0 && queue->spectators[i] == 0)
		{
			queue->spectators[i] = new_socket;
			count--;
			queue->unused_count--;
		}
		else
		{
			queue->unused_count--;
		}
	}

	read_message(new_socket, message, LG_MESSAGE * sizeof(char), 0);
	strcpy(message, "S");
	send_message(new_socket, message);

    mtx_unlock(&queue->mutex);
}

// Background thread function to accept connections
int accept_connections(void *arg)
{
    int server_socket = *(int *)arg;
	
    while (!cancel_flag)
	{
		//
		fd_set read_fds;
		FD_ZERO(&read_fds);
    	FD_SET(server_socket, &read_fds);
		struct timeval timeout = {0, 500000};
		int client_socket = 0;

        int ready = select(server_socket + 1, &read_fds, NULL, NULL, &timeout);

        if (ready == -1) {
            perror("select");
            break;
        } else if (ready == 0) {
            client_socket = 0;
        } else {
            if (FD_ISSET(server_socket, &read_fds))
			{
				// Accept a new client connection
				struct sockaddr_in client_addr;
				socklen_t client_len = sizeof(client_addr);
				client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
			}
		}

		if (client_socket > 0)
		{
			printf("Accepted new spectator: socket %d\n", client_socket);
			send_message(client_socket, "S");
			
			// Push the client socket to the queue
			queue_push(&queue, client_socket);
			
			printf("add spectator");

			if (queue.unused_count > 0 && queue.count > 0)
			{
				//update spectator list
				update_spectator_list(&queue, &server_socket);
			}
		}
    }
	return 0;
}

/**
 * Sending a message to all spectators
 */
int notify_spectators(SocketQueue *queue, char *message)
{
	int spectator_count = SPECTATOR_SIZE - queue->unused_count;
	
	if (spectator_count > 0)
	{
		// Using parrallel threading to send to all spectators simultaneously
		omp_set_num_threads(spectator_count);
		#pragma omp parallel
		{
			send_message(queue->spectators[omp_get_thread_num()], message);
		}
	}

	return 0;
}

/* Game Functions */
/**
 * Function to let a player choose an unused cell for their turn
 */
int player_turn(int socketDialogue, int socketDialogue2, char player, char grid[GRID_CELL])
{
	int next = -1;	/* si la partie continue */
	int winner = 0; /* si le joueur actuelle à gagné */
	char message[10], position[4];
	int nb_left; /* nb de cases restantes */

	memset(message, 0, sizeof(message));
	memset(position, 0, sizeof(position));
	int bytesRead = read_message(socketDialogue, position, sizeof(position), 0);
	
	if (bytesRead > 0)
	{
		if (position[0] - '0' < 1 || position[0] - '0' > 9 || is_occupied(grid, position[0] - '0'))
		{
			next = -1;
		}
		else
		{
			update_grid(position[0] - '0', grid, position[1]);
			winner = is_winner(player, grid);

			nb_left = is_full(grid);

			if (winner == 1) next = 1;
			if (nb_left == 0) next = 2;
			if (next == -1) next = 0;
		}
    
		switch (next)
		{
		case 0:
			strcpy(message, "CONTINUE");
			break;
		case 1:
			sprintf(message, "%cWIN", player);
			break;
		case 2:
			sprintf(message, "%cEND", player);
			break;
		default:
			strcpy(message, "ERROR");
			break;
		}

		if (strcasecmp(message, "ERROR") == 1)
		{
			send_message(socketDialogue, message);
		}
		else
		{
			send_message(socketDialogue, message);
			send_message(socketDialogue2, message);
			notify_spectators(&queue, message);

			memset(message, 0, sizeof(message));
			strcpy(message, position);

			send_message(socketDialogue, message);
			send_message(socketDialogue2, message);
			notify_spectators(&queue, message);
		}
	}

	return next;
}

/**
 * Function to start a new game
 */
void game(int socketDialogue, int socketDialogue2)
{
	int run_game, result;
	char grid[GRID_CELL];

	set_empty_grid(grid);

	run_game = 1;
	result = -1;
	while (run_game == 1)
	{
		result = -1;
		while (result == -1) result = player_turn(socketDialogue, socketDialogue2, 'X', grid);

		if (result == 0)
		{
			result = -1;
			while (result == -1) result = player_turn(socketDialogue2, socketDialogue, 'O', grid);
			if (result > 0) run_game = 0;
		}
		else run_game = 0;
	}
}



int main(int argc, char *argv[])
{
	int socketDialogue,socketDialogue2;
	int socketEcoute;
	int port;
	struct sockaddr_in pointDeRencontreLocal;
	socklen_t addrLength;

	struct sockaddr_in pointDeRencontreDistant,pointDeRencontreDistant2;
	char messageRecu[LG_MESSAGE]; /* le message de la couche Application ! */
	char buffer[LG_MESSAGE];	  // Buffer pour recevoir la réponse

	int created_thread = 0;
	thrd_t accept_thread;
	int* res = 0;

	srand(time(NULL));

    // Looking for an open port
	port = MIN_PORT;
	while(1){
		// Associate socket with currently selected port
		if (port > MAX_PORT)
		{
			perror("bind");
            exit(EXIT_FAILURE);
		}
		else if ((socketEcoute = create_listen_socket(port, &pointDeRencontreLocal)) < 0)
		{
            port++;
		}
		else
		{
			printf("Current Port : %d\n", port);
			break;
		}
	}

	addrLength = sizeof(pointDeRencontreLocal);

	// On fixe la taille de la file d’attente à 5 (pour les demandes de connexion non encore traitées)
	if (listen(socketEcoute, 5) < 0)
	{
		perror("listen");
		exit(-3);
	}
	printf("Socket placée en écoute passive ...\n");

	// boucle d’atttente de connexion : en théorie, un serveur attend indéfiniment !
	while (1)
	{
		memset(messageRecu, 'a', LG_MESSAGE*sizeof(char));
		printf("Attente d’une demande de connexion (quitter avec Ctrl-C)\n\n");

		// c’est un appel bloquant
		socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &addrLength);
		if (socketDialogue < 0)
		{
			perror("accept");
			close(socketDialogue);
			close(socketEcoute);
			exit(-4);
		}

		socketDialogue2 = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant2, &addrLength);
		if (socketDialogue2 < 0)
		{
			perror("accept");
			close(socketDialogue2);
			close(socketEcoute);
			exit(-4);
		}

		queue_init(&queue);

		if (thrd_create(&accept_thread, accept_connections, &socketEcoute) != thrd_success) {
			fprintf(stderr, "Failed to create spectator accept thread\n");
		}
		else
		{
			created_thread = 1;
		}

		read_message(socketDialogue, messageRecu, LG_MESSAGE * sizeof(char), 0);
		read_message(socketDialogue2, messageRecu, LG_MESSAGE * sizeof(char), 0);

		strcpy(buffer, "start");
		send_message(socketDialogue, buffer);
		send_message(socketDialogue2,buffer);

		memset(&buffer, 0x00, 9);
		strcpy(buffer, "X");
		send_message(socketDialogue, buffer);

		memset(&buffer, 0x00, 9);
		strcpy(buffer, "O");
		send_message(socketDialogue2, buffer);

		game(socketDialogue, socketDialogue2);
		
		close(socketDialogue);
		close(socketDialogue2);

		if(created_thread == 1){
			cancel_flag = 1;
			thrd_join(accept_thread, res);
			created_thread = 0;
			cancel_flag = 0;
		}
		queue_destroy(&queue);
	}

	sleep(10);

	close(socketEcoute);
	return 0;
}