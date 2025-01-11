/**
 * @author
 * Behani Julien
 * Mallevaey Adrien
 * Fournier Enzo
 * Alluin Edouard
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "include/socket_management.h"
#include "include/tictactoe.h"

#define LG_MESSAGE 256

int main(int argc, char *argv[])
{
	int descriptorSocket;
	struct sockaddr_in sockaddrDistant;

	char buffer[] = "Demande de partie";
	char message[10];
	char player;
	char player_turn;
	int first_turn_block = 1;
	char grid[GRID_CELL];

	char ip_dest[16];
	int port_dest;

	if (argc > 1)
	{ 
		strncpy(ip_dest, argv[1], 16);
		sscanf(argv[2], "%d", &port_dest);
	}
	else
	{
		printf("USAGE : %s ip port\n", argv[0]);
		exit(-1);
	}

	// Create a communication socket
	descriptorSocket = create_communication_socket(port_dest,ip_dest,&sockaddrDistant);

	send_message(descriptorSocket, buffer);

	read_message(descriptorSocket, buffer, LG_MESSAGE * sizeof(char), 0);

	if (strcmp(buffer,"start") == 0)
	{
		printf("GAME START\n");

		read_message(descriptorSocket, buffer, LG_MESSAGE * sizeof(char), 0);

		printf("Player : %s\n\n", buffer);
		player = buffer[0];
		player_turn = (player == 'X') ? 'O' : 'X';

		set_empty_grid(grid);
		show_grid(grid);
		
		// Loop on the interaction between client and server
		while (1)
		{
			if ((first_turn_block != 1 && player == 'X') || player == 'O')
			{
				printf("Player %c's turn\n\n", player_turn);
			}
			else
			{
				first_turn_block = 0;
			}
			// Wait if player is 'O' and it's the first turn
			if ((first_turn_block != 1 && player == 'O') || player == 'X')
			{
				
				int chosenCell;
				printf("Choose a cell: ");

				// While is not a number
				while (1) {
					printf("Please enter a number between 1 and 9: ");

					// Check if the input is valid
					if (scanf("%d", &chosenCell) != 1) {
						printf("Invalid input.\nPlease enter a valid number.\n");
						while (getchar() != '\n'); // Clear the buffer
						continue;
					}

					if (chosenCell < 1 || chosenCell > 9)
					{
						printf("Value too big.\nPlease enter a single number.\n");
						continue;
					}
					
					if (is_occupied(grid, chosenCell)){
						printf("Cell already occupied.\nPlease choose an empty cell.\n");
						continue;
					}
					// If everything is correct, exit the loop
					break;
				}

				message[0] = chosenCell + '0';
				message[1] = player;

				send_message(descriptorSocket, message);

				update_grid(chosenCell, grid, message[1]);
				show_grid(grid);
			}
			else
			{
				read_message(descriptorSocket, buffer, LG_MESSAGE * sizeof(char), 0);
				first_turn_block = 0;
			}

			memset(message, 0, sizeof(message));
			read_message(descriptorSocket, message, sizeof(message), 0);

			if (message[0] == 'X' || message[0] == 'O')
			{
				if (strcmp(message, "XWIN") == 0)
				{
					if	(player == 'O')
					{
						read_message(descriptorSocket, message, sizeof(message), 0);
						update_grid(message[0] - '0', grid, message[1]);
						show_grid(grid);
					}
					printf("The player X has won !\n");
					close(descriptorSocket);
					return 0;
				}
				else if (strcmp(message, "XEND") == 0)
				{
					if	(player == 'O')
					{
						read_message(descriptorSocket, message, sizeof(message), 0);
						update_grid(message[0] - '0', grid, message[1]);
						show_grid(grid);
					}
					printf("Game over\nNo winner !\n");
					close(descriptorSocket);
					return 0;
				}
				else if (strcmp(message, "OWIN") == 0)
				{
					if (player == 'X')
					{
						read_message(descriptorSocket, message, sizeof(message), 0);
						update_grid(message[0] - '0', grid, message[1]);
						show_grid(grid);
					}
					printf("The player O has won !\n");
					close(descriptorSocket);
					return 0;
				}
				else if (strcmp(message, "OEND") == 0)
				{
					if	(player == 'X')
					{
						read_message(descriptorSocket, message, sizeof(message), 0);
						update_grid(message[0] - '0', grid, message[1]);
						show_grid(grid);
					}
					printf("Game over\nNo winner !\n");
					close(descriptorSocket);
					return 0;
				}
			}
			else if (strcmp(message, "CONTINUE") == 0)
			{
				read_message(descriptorSocket, message, sizeof(message), 0);
				update_grid(message[0] - '0', grid, message[1]);
				show_grid(grid);
			}
			else if (strcmp(message, "ERROR") == 0)
			{
				printf("Erreur\n");
				read_message(descriptorSocket, message, sizeof(message), 0);
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
		
		close(descriptorSocket);
		return 0;
	}
	else
	{
		set_empty_grid(grid);
		show_grid(grid);

		while (1)
		{
			memset(message, 0, sizeof(message));
			read_message(descriptorSocket, message, sizeof(message), 0);
			printf("%s", message);
			
			if ((strcmp(message, "XWIN") == 0) || (strcmp(message, "OWIN") == 0))
			{
				memset(message, 0, sizeof(message));
				read_message(descriptorSocket, message, sizeof(message), 0);
				update_grid(message[0] - '0', grid, message[1]);
				show_grid(grid);
				printf("Player %c has won !!!\n", message[1]);
				close(descriptorSocket);
				return 0;
			}
			else if ((strcmp(message, "XEND") == 0) || (strcmp(message, "OEND") == 0))
			{
				memset(message, 0, sizeof(message));
				read_message(descriptorSocket, message, sizeof(message), 0);
				update_grid(message[0] - '0', grid, message[1]);
				show_grid(grid);
				printf("GAME OVER\n Nobody won !!!\n");
				close(descriptorSocket);
				return 0;
			}
			else if (strcmp(message, "CONTINUE") == 0)
			{
				memset(message, 0, sizeof(message));
				read_message(descriptorSocket, message, sizeof(message), 0);
				update_grid(message[0] - '0', grid, message[1]);
				show_grid(grid);
			}
		}
	}
}
