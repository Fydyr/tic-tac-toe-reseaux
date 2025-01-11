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
#include <string.h>		/* pour memset */
#include <netinet/in.h> /* pour struct sockaddr_in */
#include <arpa/inet.h>	/* pour htons et inet_aton */
#include <signal.h>
#include <time.h>
#include "include/socket_management.h"
#include "include/tictactoe.h"

#define PORT 5000

#define LG_MESSAGE 256

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

			memset(message, 0, sizeof(message));
			strcpy(message, position);

			send_message(socketDialogue, message);
			send_message(socketDialogue2, message);
		}
	}

	return next;
}

void game(int socketDialogue, int socketDialogue2)
{
	int run_game, result;
	char grid[GRID_CELL];

	// Initialization of the grid
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
	int socketDialogue, socketDialogue2;
	int socketEcoute;
	struct sockaddr_in pointDeRencontreLocal;
	socklen_t addrLength;

	struct sockaddr_in pointDeRencontreDistant, pointDeRencontreDistant2;
	char messageRecu[LG_MESSAGE]; /* le message de la couche Application ! */
	char buffer[LG_MESSAGE];	  // Buffer pour recevoir la réponse

	srand(time(NULL));

	socketEcoute = create_listen_socket(PORT, &pointDeRencontreLocal);
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
		memset(messageRecu, 0, LG_MESSAGE * sizeof(char));
		printf("Attente d’une demande de connexion (quitter avec Ctrl-C)\n\n");

		// c’est un appel bloquant
		socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &addrLength);
		if (socketDialogue < 0)
		{
			perror("accept");
			close(socketDialogue);
			exit(-4);
		}

		socketDialogue2 = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant2, &addrLength);
		if (socketDialogue2 < 0)
		{
			perror("accept");
			close(socketDialogue2);
			exit(-4);
		}

		read_message(socketDialogue, messageRecu, LG_MESSAGE * sizeof(char), 0);
		read_message(socketDialogue2, messageRecu, LG_MESSAGE * sizeof(char), 0);

		strcpy(buffer, "start");
		send_message(socketDialogue, buffer);
		send_message(socketDialogue2, buffer);

		memset(&buffer, 0x00, 9);
		strcpy(buffer, "X");
		send_message(socketDialogue, buffer);

		memset(&buffer, 0x00, 9);
		strcpy(buffer, "O");
		send_message(socketDialogue2, buffer);

		game(socketDialogue, socketDialogue2);
	}

	sleep(10);

	close(socketEcoute);
	return 0;
}