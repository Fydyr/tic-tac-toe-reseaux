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

void game(int socketDialogue, int socketDialogue2)
{
	int nb_left;					  /* nb de cases restantes */
	int winner_x = 0;					  /* si le joueur X à gagné */
	int winner_O = 0;					  /* si le joueur O à gagné */
	int run_game;
	char message[LG_MESSAGE];

	// Initialization of the grid
	char grid[GRID_CELL];
	set_empty_grid(grid);
	run_game = 1;

	while (run_game)
	{ 
		int bytesRead = read_message(socketDialogue, message, sizeof(message));
		
		if (message[0] - '0' < 1)
		{
			strcpy(message, "ERROR"); 
			send_message(socketDialogue, message);

			memset(&message, 0x00, 9);
			message[0] = '1'; 

			send_message(socketDialogue, message);
		}
		else if (message[0] - '0' > 9)
		{
			strcpy(message, "ERROR"); 
			send_message(socketDialogue, message);

			memset(&message, 0x00, 9);
			message[0] = '2'; 

			send_message(socketDialogue, message);
		}
		else
		{
			if ( bytesRead > 0)
			{

				update_grid(message[0] - '0', grid, message[1]);
				show_grid(grid);

				winner_x = is_winner('X', grid);
				nb_left = is_full(grid);

				if (nb_left == 0 || winner_x == 1)
				{
					if (winner_x == 1)
					{
						strcpy(message, "XWIN"); 
						send_message(socketDialogue, message);
						close(socketDialogue);
						run_game = 0;
					}
					else
					{
						strcpy(message, "XEND"); 
						send_message(socketDialogue, message);
						close(socketDialogue);
						run_game = 0;
					}
				}
				else
				{
					int random_nb = (rand() % (GRID_SIZE*GRID_SIZE)) + 1;
					while (is_occupied(grid, random_nb)){
						random_nb = (rand() % (GRID_SIZE*GRID_SIZE)) + 1;
					}

					update_grid(random_nb, grid, 'O');
					show_grid(grid);

					winner_O = is_winner('O', grid);
					nb_left = is_full(grid);

					if (nb_left == 0 || winner_O == 1)
					{
						if (winner_O == 1)
						{
							strcpy(message, "OWIN"); 
							send_message(socketDialogue, message);

							memset(&message, 0x00, 9);
							message[0] = random_nb + '0'; 
							message[1] = 'O';

							send_message(socketDialogue, message);
							close(socketDialogue);
							run_game = 0;
						}
						else
						{
							strcpy(message, "OEND"); 
							send_message(socketDialogue, message);

							memset(&message, 0x00, 9);
							message[0] = random_nb + '0'; 
							message[1] = 'O';

							send_message(socketDialogue, message);
							close(socketDialogue);
							run_game = 0;
						}
					}
					else
					{
						strcpy(message, "CONTINUE"); 
						send_message(socketDialogue, message);

						memset(&message, 0x00, 9);
						message[0] = random_nb + '0'; 
						message[1] = 'O';

						send_message(socketDialogue, message);
					}
				}
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

		read_message(socketDialogue, messageRecu, LG_MESSAGE * sizeof(char));
		read_message(socketDialogue2, messageRecu, LG_MESSAGE * sizeof(char));

		strcpy(buffer, "start");
		send_message(socketDialogue, buffer);
		send_message(socketDialogue2,buffer);

		game(socketDialogue, socketDialogue2);
	}

	sleep(10);

	close(socketEcoute);
	return 0;
}