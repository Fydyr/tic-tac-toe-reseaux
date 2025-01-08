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

#define PORT 5001 //(ports >= 5000 réservés pour usage explicite)
#define LG_MESSAGE 256
#define GRID_SIZE 3
#define GRID_CASE GRID_SIZE *GRID_SIZE

int socketDialogue;
int socketEcoute;

void show_grid(const char grid[GRID_CASE])
{
	for (int i = 0; i < GRID_CASE; i++)
	{
		printf(" %c ", grid[i] ? grid[i] : ' ');
		if ((i + 1) % GRID_SIZE == 0)
		{
			printf("\n");
			if (i < GRID_CASE - GRID_SIZE)
			{
				for (int j = 0; j < GRID_SIZE - 1; j++)
				{
					printf("---+");
				}
				printf("---\n");
			}
		}
		else
		{
			printf("|");
		}
	}
	printf("\n");
}

void update_grid(const int i, char grid[GRID_CASE], const char symbol)
{
	grid[i-1] = symbol;
}

int check_full(const char *grid){
	int i, nb;
	nb = 0;

	for (i = 0; i < GRID_CASE; i++)
	{
		if (grid[i] == ' ')
		{
			nb = nb + 1;
		}
	}

	return nb;
}

int gagnant(char joueur, const char *grid){
	int i, result;

    for (i = 0; i < GRID_CASE; i=i+3) {
        if (grid[i] == joueur && grid[i+1] == joueur && grid[i+2] == joueur) {
            result = 1;
        }
    }

    // Vérifier les colonnes
    for (i = 0; i < GRID_CASE; i++) {
        if (grid[i] == joueur && grid[i+3] == joueur && grid[i+6] == joueur) {
            result = 1;
        }
    }

    // Vérifier les diagonales
    if ((grid[0] == joueur && grid[4] == joueur && grid[8] == joueur) || (grid[2] == joueur && grid[4] == joueur && grid[6] == joueur)) {
            result = 1;
    }

    return result;
}

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
	pointDeRencontreLocal.sin_port = htons(PORT);			   // = 5000 ou plus

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
		for (int i = 0; i < GRID_CASE; i++)
		{
			grid[i] = ' ';
		}

		while (1)
		{
			char message[4]; 

			int bytesRead = read_message(socketDialogue, message, sizeof(message));

			if ( bytesRead > 0)
			{
				update_grid(message[0] - '0', grid, message[1]);
				show_grid(grid);

				nb_left = check_full(grid);
				printf("%d\n", nb_left);

				if (nb_left == 0)
				{
					printf("");
				}
				else
				{
					int random_nb = (rand() % GRID_CASE) + 1;

					update_grid(random_nb, grid, 'O');
					show_grid(grid);

					message[0] = random_nb + '0'; 
					message[1] = 'O';

					send_message(socketDialogue, message);
				}
			}
		}
	}

	close(socketEcoute);
	return 0;
}