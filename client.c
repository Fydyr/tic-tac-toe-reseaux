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

int request_cell(const char grid[GRID_CELL])
{
	int chosenCell;
	printf("Choose a cell: ");

	// While is not a number
	while (1)
	{
		printf("Please enter a number between 1 and 9: ");

		// Check if the input is valid
		if (scanf("%d", &chosenCell) != 1)
		{
			printf("Invalid input.\nPlease enter a valid number.\n");
			while (getchar() != '\n')
				; // Clear the buffer
			continue;
		}

		if (chosenCell < 1 || chosenCell > 9)
		{
			printf("Value too big.\nPlease enter a single number.\n");
			continue;
		}

		if (is_occupied(grid, chosenCell))
		{
			printf("Cell already occupied.\nPlease choose an empty cell.\n");
			continue;
		}
		// If everything is correct, exit the loop
		break;
	}
	return chosenCell;
}

/**
 * @return
 * - `-1` : erreur / `0` : continue / `1` : fin de jeu
 */
int result_process(const char player, int descriptorSocket)
{
	char message[10], winner_code[6], loser_code[6];
	int state;

	read_message(descriptorSocket, message, sizeof(message), 0);

	if(player == 'X' || player == 'O')
	{
		sprintf(winner_code, "%cWIN", player);
		sprintf(loser_code, "%cWIN", (player == 'X') ? 'O' : 'X');
	}
	
	if (strcasecmp(message, "XEND") == 0 || strcasecmp(message, "OEND") == 0)
	{
		printf("GAME OVER\n Nobody has won\n");
		state = 1;
	}
	else if (strcasecmp(message, winner_code) == 0)
	{
		printf("You have won\n");
		state = 1;
	}
	else if (strcasecmp(message, loser_code) == 0)
	{
		printf("You have lost\n");
		state = 1;
	}
	else if (strcasecmp(message, "XWIN") == 0 || strcasecmp(message, "OWIN") == 0)
	{
		printf("The player %c is winner\n",message[0]);
		state = 1;
	}
	else if (strcasecmp(message, "CONTINUE") == 0)
	{
		state = 0;
	}
	else if (strcmp(message, "ERROR") == 0)
	{
		printf("Invalid input: number is too large. Choose between 1 and 9.\n");
		state = -1;
	}
	else
	{
		printf("Unknown message received: %s\n", message);
		state = -1;
	}
	return state;
}

/**
 * If the client is a player
 * @param player the current player symbol
 * @param grid the character table who represents the grid
 * @param descriptorSocket the socket who is connected to the server
 */
void play(const char player, char grid[GRID_CELL], int descriptorSocket)
{
	char message[4];
	int chosenCell;

	printf("It is your turn !\n");
	chosenCell = request_cell(grid);

	message[0] = chosenCell + '0';
	message[1] = player;

	send_message(descriptorSocket, message);

	update_grid(chosenCell, grid, message[1]);
}

/**
 * If the player don't play, he spectates the game
 * @param player the current player symbol
 * @param grid the character table who represents the grid
 * @param descriptorSocket the socket who is connected to the server
 */
void spectate(const char player, char grid[GRID_CELL], int descriptorSocket)
{
	printf("It is the turn of Player %c !\n", (player == 'X') ? 'O' : 'X');
}

/**
 * The game loop, the game logic is applied here
 * @param player the current player symbol
 * @param grid the character table who represents the grid
 * @param descriptorSocket the socket who is connected to the server
 */
void game_loop(const char player, char grid[GRID_CELL], int descriptorSocket)
{
	int result, position;
	char message[4];

	position = (player == 'X') ? 1 : 2;
	position = (player == 'S') ? 0 : position;

	result = -1;

	while (result <= 0)
	{
		result = -1;
		while (result == -1)
		{
			if (position == 1) play(player, grid, descriptorSocket);
			else spectate(player, grid, descriptorSocket);

			result = result_process(player, descriptorSocket);
		}

		if(position == 1) position = 2;
		else if(position == 2) position = 1;

		memset(message, 0, sizeof(message));
		read_message(descriptorSocket, message, sizeof(message), 0);
		update_grid(message[0] - '0', grid, message[1]);
		show_grid(grid);
		memset(message, 0, sizeof(message));
	}
}

int main(int argc, char *argv[])
{
	int descriptorSocket;
	struct sockaddr_in sockaddrDistant;

	char buffer[] = "Demande de partie";
	char player;

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
	descriptorSocket = create_communication_socket(port_dest, ip_dest, &sockaddrDistant);

	send_message(descriptorSocket, buffer);

	read_message(descriptorSocket, buffer, LG_MESSAGE * sizeof(char), 0);

	printf("GAME START\n");
	read_message(descriptorSocket, buffer, LG_MESSAGE * sizeof(char), 0);

	printf("Player : %s\n\n", buffer);
	player = buffer[0];

	// Initialization of the grid
	char grid[GRID_CELL];

	set_empty_grid(grid);
	show_grid(grid);

	// Start the game
	game_loop(player, grid, descriptorSocket);

	sleep(10);
	close(descriptorSocket);
	return 0;
}