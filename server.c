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
#include "socket_management.h"
#include "tictactoe.h"

#define PORT 5000 //(ports >= 5000 réservés pour usage explicite)
#define LG_MESSAGE 256

int socketDialogue;
int socketEcoute;


void handle_signal(int sig)
{
	printf("\nSignal reçu (%d). Fermeture du serveur...\n", sig);

	if (socketDialogue >= 0)
	{
		close(socketDialogue);
		printf("Socket de dialogue fermée.\n");
	}
	if (socketEcoute >= 0)
	{
		close(socketEcoute);
		printf("Socket d'écoute fermée.\n");
	}

	sleep(2);

	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{

	struct sockaddr_in pointDeRencontreLocal;
	socklen_t longueurAdresse;

	struct sockaddr_in pointDeRencontreDistant;
	char messageRecu[LG_MESSAGE]; /* le message de la couche Application ! */
	char buffer[LG_MESSAGE];	  // Buffer pour recevoir la réponse
	int nb_left;					  /* nb de cases restantes */
	int winner_x = 0;					  /* si le joueur X à gagné */
	int winner_O = 0;					  /* si le joueur O à gagné */

	srand(time(NULL));

	// Check if the program is CTRL-C
	// if (signal(SIGINT, handle_signal) == SIG_ERR) {
	//    perror("signal");
	//    exit(EXIT_FAILURE);
	//}
	// Check if the program window is close
	if (signal(SIGHUP, handle_signal) == SIG_ERR)
	{
		perror("signal");
		exit(EXIT_FAILURE);
	}

	// Crée un socket de communication
	socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
	// Teste la valeur renvoyée par l’appel système socket()
	if (socketEcoute < 0)
	{
		perror("socket"); // Affiche le message d’erreur
		exit(-1);		  // On sort en indiquant un code erreur
	}
	printf("Socket créée avec succès ! (%d)\n", socketEcoute); // On prépare l’adresse d’attachement locale

	// Remplissage de sockaddrDistant (structure sockaddr_in identifiant le point d'écoute local)
	longueurAdresse = sizeof(pointDeRencontreLocal);
	// memset sert à faire une copie d'un octet n fois à partir d'une adresse mémoire donnée
	// ici l'octet 0 est recopié longueurAdresse fois à partir de l'adresse &pointDeRencontreLocal
	memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
	pointDeRencontreLocal.sin_family = PF_INET;
	pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY); // attaché à toutes les interfaces locales disponibles
	pointDeRencontreLocal.sin_port = htons(PORT);			   // = 5001 ou plus

	// On demande l’attachement local de la socket
	if ((bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse)) < 0)
	{
		perror("bind");
		exit(-2);
	}
	printf("Socket attachée avec succès !\n");

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
		printf("Attente d’une demande de connexion (quitter avec Ctrl-C)\n\n");

		// c’est un appel bloquant
		socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &longueurAdresse);
		if (socketDialogue < 0)
		{
			perror("accept");
			close(socketDialogue);
			close(socketEcoute);
			exit(-4);
		}

		read_message(socketDialogue, messageRecu, LG_MESSAGE * sizeof(char));

		strcpy(buffer, "start");
		send_message(socketDialogue, buffer);

		// Initialization of the grid
		char grid[GRID_CASE];
		set_empty_grid(grid);

		while (1)
		{
			char message[10]; 

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
						}
						else
						{
							strcpy(message, "XEND"); 
							send_message(socketDialogue, message);
						}
					}
					else
					{
						int random_nb = (rand() % GRID_CASE) + 1;

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
							}
							else
							{
								strcpy(message, "OEND"); 
								send_message(socketDialogue, message);

								memset(&message, 0x00, 9);
								message[0] = random_nb + '0'; 
								message[1] = 'O';

								send_message(socketDialogue, message);
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

	sleep(10);

	close(socketEcoute);
	return 0;
}