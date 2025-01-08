#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket_management.h"

#define LG_MESSAGE 256
#define GRID_SIZE 3
#define GRID_CASE GRID_SIZE *GRID_SIZE

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
	grid[i - 1] = symbol;
}

int main(int argc, char *argv[])
{
	int descripteurSocket;
	struct sockaddr_in sockaddrDistant;
	socklen_t longueurAdresse;

	char buffer[] = "Demande de partie"; // buffer stockant le message
	// int nb;							 /* nb d’octets écrits et lus */

	char ip_dest[16];
	int port_dest;

	// Pour pouvoir contacter le serveur, le client doit connaître son adresse IP et le port de comunication
	// Ces 2 informations sont passées sur la ligne de commande
	// Si le serveur et le client tournent sur la même machine alors l'IP locale fonctionne : 127.0.0.1
	// Le port d'écoute du serveur est 5000 dans cet exemple, donc en local utiliser la commande :
	// ./client_base_tcp 127.0.0.1 5000
	if (argc > 1)
	{ // si il y a au moins 2 arguments passés en ligne de commande, récupération ip et port
		strncpy(ip_dest, argv[1], 16);
		sscanf(argv[2], "%d", &port_dest);
	}
	else
	{
		printf("USAGE : %s ip port\n", argv[0]);
		exit(-1);
	}

	// Crée un socket de communication
	descripteurSocket = socket(AF_INET, SOCK_STREAM, 0);
	// Teste la valeur renvoyée par l’appel système socket()
	if (descripteurSocket < 0)
	{
		perror("Erreur en création de la socket..."); // Affiche le message d’erreur
		exit(-1);									  // On sort en indiquant un code erreur
	}
	printf("Socket créée! (%d)\n", descripteurSocket);

	// Remplissage de sockaddrDistant (structure sockaddr_in identifiant la machine distante)
	// Obtient la longueur en octets de la structure sockaddr_in
	longueurAdresse = sizeof(sockaddrDistant);
	// Initialise à 0 la structure sockaddr_in
	// memset sert à faire une copie d'un octet n fois à partir d'une adresse mémoire donnée
	// ici l'octet 0 est recopié longueurAdresse fois à partir de l'adresse &sockaddrDistant
	memset(&sockaddrDistant, 0x00, longueurAdresse);
	// Renseigne la structure sockaddr_in avec les informations du serveur distant
	sockaddrDistant.sin_family = AF_INET;
	// On choisit le numéro de port d’écoute du serveur
	sockaddrDistant.sin_port = htons(port_dest);
	// On choisit l’adresse IPv4 du serveur
	inet_aton(ip_dest, &sockaddrDistant.sin_addr);

	// Débute la connexion vers le processus serveur distant
	if ((connect(descripteurSocket, (struct sockaddr *)&sockaddrDistant, longueurAdresse)) == -1)
	{
		perror("Erreur de connection avec le serveur distant...");
		close(descripteurSocket);
		exit(-2); // On sort en indiquant un code erreur
	}
	printf("Connexion au serveur %s:%d réussie!\n", ip_dest, port_dest);

	send_message(descripteurSocket, buffer);

	read_message(descripteurSocket, buffer, LG_MESSAGE * sizeof(char));

	// Initialization of the grid
	char grid[GRID_CASE];
	for (int i = 0; i < GRID_CASE; i++)
	{
		grid[i] = ' ';
	}

	show_grid(grid);

	// Loop on the interaction between client and server
	while (1)
	{
		int chosenCase;
		printf("Choose a case: ");

		// While is not a number
		while (1) {
			printf("Please enter a number between 1 and 9: ");

			// Check if the input is valid
			if (scanf("%d", &chosenCase) != 1) {
				printf("Invalid input. Please enter a valid number.\n");
				while (getchar() != '\n'); // Clear the buffer
				continue;
			}

			if (chosenCase)
			{
				printf("Value too big. Please enter a single number.\n");
				continue;
			}
			// If everything is correct, exit the loop
			break;
    	}

		char message[10] = {chosenCase + '0', 'X'}; 

		send_message(descripteurSocket, message);

		update_grid(chosenCase, grid, message[1]);
		show_grid(grid);

		memset(message, 0, sizeof(message));

		read_message(descripteurSocket, message, sizeof(message));

		if (message[0] == 'X' || message[0] == 'O')
		{
			if (strcmp(message, "XWIN") == 0)
			{
				printf("The player has won !\n");
				close(descripteurSocket);
				return 0;
			}
			else if (strcmp(message, "XEND") == 0)
			{
				printf("Game over\nNo winner !\n");
				close(descripteurSocket);
				return 0;
			}
			else if (strcmp(message, "OWIN") == 0)
			{
				read_message(descripteurSocket, message, sizeof(message));
				update_grid(message[0] - '0', grid, message[1]);
				show_grid(grid);
				printf("The server has won !\n");
				close(descripteurSocket);
				return 0;
			}
			else if (strcmp(message, "OEND") == 0)
			{
				read_message(descripteurSocket, message, sizeof(message));
				update_grid(message[0] - '0', grid, message[1]);
				show_grid(grid);
				printf("Game over\nNo winner !\n");
				close(descripteurSocket);
				return 0;
			}
		}
		else if (strcmp(message, "CONTINUE") == 0)
		{
			read_message(descripteurSocket, message, sizeof(message));
			update_grid(message[0] - '0', grid, message[1]);
			show_grid(grid);
		}
		else if (strcmp(message, "ERROR") == 0)
		{
			printf("Erreur\n");
			read_message(descripteurSocket, message, sizeof(message));
			if (message[0] == '1')
			{
				printf("The number is inferior to what can be choosen\nThe number must be between 1 and 9. Try again.\n");
			}
			else if (message[0] == '2')
			{
				printf("The number is superior to what can be choosen\nThe number must be between 1 and 9. Try again.\n");
			}
        }
	}
}
