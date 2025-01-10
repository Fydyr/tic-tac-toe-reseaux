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

#define PORT 5000 //(ports >= 5000 réservés pour usage explicite)
#define MAX_PORT 5005 //(Port maximal pouvant être utilisé)
#define LG_MESSAGE 256

struct Tuple {
    int outcome;
    char position[LG_MESSAGE];
};

struct Tuple player_turn(int socketDialogue, char player, char grid[GRID_CELL])
{
	int next = 0;					/* si la partie continue */
	int winner = 0;					  /* si le joueur actuelle à gagné */
	char message[LG_MESSAGE];
	struct Tuple result;
	int nb_left;					  /* nb de cases restantes */


	int bytesRead = read_message(socketDialogue, message, sizeof(message), 0);
	
	if (message[0] - '0' < 1)
	{
		next = -1;
	}
	else if (message[0] - '0' > 9)
	{
		next = -2;
	}
	else
	{
		if ( bytesRead > 0)
		{

			update_grid(message[0] - '0', grid, message[1]);
			show_grid(grid);

			winner = is_winner(player, grid);
			nb_left = is_full(grid);

			if (nb_left == 0 || winner == 1)
			{
				if (winner == 1)
				{
					next = 1;
				}
				else
				{
					next = 2;
				}
			}
		}
	}
	result.outcome = next;
	strcpy(result.position, message);
	return result;
}

void game(int socketDialogue, int socketDialogue2)
{
	struct Tuple result_turn;					  /* resultat du tour du joueur */
	int run_game;
	int first_turn = 1;
	char message[LG_MESSAGE];

	// Initialization of the grid
	char grid[GRID_CELL];
	set_empty_grid(grid);
	run_game = 1;

	while (run_game)
	{ 
		result_turn = player_turn(socketDialogue, 'X', grid);
		switch (result_turn.outcome)
		{
		case -2:
			strcpy(message, "ERROR"); 
			send_message(socketDialogue, message);

			memset(&message, 0x00, 9);
			message[0] = '2'; 

			send_message(socketDialogue, message);
			break;
		
		case -1:
			strcpy(message, "ERROR"); 
			send_message(socketDialogue, message);

			memset(&message, 0x00, 9);
			message[0] = '1'; 

			send_message(socketDialogue, message);
			break;

		case 0:
			if(first_turn == 1)
			{
				strcpy(message, "START");
				send_message(socketDialogue2, message);
				first_turn = 0;
			}
			strcpy(message, "CONTINUE"); 
			send_message(socketDialogue2, message);

			memset(&message, 0x00, 9);
			strcpy(message, result_turn.position);

			send_message(socketDialogue2, message);
			break;

		case 1:
			strcpy(message, "XWIN"); 
			send_message(socketDialogue, message);
			send_message(socketDialogue2, message);
			close(socketDialogue);

			memset(&message, 0x00, 9);
			strcpy(message, result_turn.position);

			send_message(socketDialogue2, message);
			close(socketDialogue2);
			run_game = 0;
			break;
		
		case 2:
			strcpy(message, "XEND"); 
			send_message(socketDialogue, message);
			send_message(socketDialogue2, message);
			close(socketDialogue);

			memset(&message, 0x00, 9);
			strcpy(message, result_turn.position);

			send_message(socketDialogue2, message);
			close(socketDialogue2);
			run_game = 0;
			break;

		default:
			break;
		}

		if (result_turn.outcome == 0){
			result_turn = player_turn(socketDialogue2, 'O', grid);
			switch (result_turn.outcome)
			{
			case -2:
				strcpy(message, "ERROR"); 
				send_message(socketDialogue2, message);

				memset(&message, 0x00, 9);
				message[0] = '2'; 

				send_message(socketDialogue2, message);
				break;
			
			case -1:
				strcpy(message, "ERROR"); 
				send_message(socketDialogue2, message);

				memset(&message, 0x00, 9);
				message[0] = '1'; 

				send_message(socketDialogue2, message);
				break;

			case 0:
				strcpy(message, "CONTINUE"); 
				send_message(socketDialogue, message);

				memset(&message, 0x00, 9);
				strcpy(message, result_turn.position);

				send_message(socketDialogue, message);
				break;

			case 1:
				strcpy(message, "OWIN"); 
				send_message(socketDialogue2, message);
				send_message(socketDialogue, message);
				close(socketDialogue2);

				memset(&message, 0x00, 9);
				strcpy(message, result_turn.position);

				send_message(socketDialogue, message);
				close(socketDialogue);
				run_game = 0;
				break;
			
			case 2:
				strcpy(message, "OEND"); 
				send_message(socketDialogue2, message);
				send_message(socketDialogue, message);
				close(socketDialogue2);

				memset(&message, 0x00, 9);
				strcpy(message, result_turn.position);

				send_message(socketDialogue, message);
				close(socketDialogue);
				run_game = 0;
				break;

			default:
				break;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	int socketDialogue,socketDialogue2;
	int socketEcoute;
	struct sockaddr_in pointDeRencontreLocal;
	socklen_t addrLength;

	struct sockaddr_in pointDeRencontreDistant,pointDeRencontreDistant2;
	char messageRecu[LG_MESSAGE]; /* le message de la couche Application ! */
	char buffer[LG_MESSAGE];	  // Buffer pour recevoir la réponse

	srand(time(NULL));

	socketEcoute = create_listen_socket(PORT,&pointDeRencontreLocal);
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
	}

	sleep(10);

	close(socketEcoute);
	return 0;
}