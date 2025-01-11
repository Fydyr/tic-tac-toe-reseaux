/**
 * @author
 * Behani Julien
 * Mallevaey Adrien
 * Fournier Enzo
 * Alluin Edouard
 */

#include "tictactoe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void set_empty_grid(char grid[GRID_CELL])
{
    memset(grid, ' ', GRID_CELL * sizeof(char));
}

void show_grid(const char grid[GRID_CELL])
{
    for (int i = 0; i < GRID_CELL; i++)
    {
        printf(" %c ", grid[i] ? grid[i] : ' ');
        if ((i + 1) % GRID_SIZE == 0)
        {
            printf("\n");
            if (i < GRID_CELL - GRID_SIZE)
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

void update_grid(const int i, char grid[GRID_CELL], const char symbol)
{
    if (i >= 1 && i <= GRID_CELL)
    {
        grid[i - 1] = symbol;
    }
    else 
    {
        fprintf(stderr, "Error: Out of range on grid update (index:%d,symbol:%c)\n", i, symbol);
        exit(EXIT_FAILURE);
    }
}

int is_full(const char grid[GRID_CELL]){
	int i, nb;
	nb = 0;

	for (i = 0; i < GRID_CELL; i++)
	{
		if (grid[i] == ' ')
		{
			nb = nb + 1;
		}
	}

	return nb;
}

int is_winner(char player, const char grid[GRID_CELL]){
	int i, result;

    result = 0;

    for (i = 0; i < GRID_CELL; i=i+GRID_SIZE) {
        if (grid[i] == player && grid[i+1] == player && grid[i+2] == player) {
            result = 1;
        }
    }
    // Check columns
    for (i = 0; i < GRID_SIZE; i++) {
        if (grid[i] == player && grid[i+GRID_SIZE] == player && grid[i+GRID_SIZE*2] == player) {
            result = 1;
        }
    }
    // Check diagonals
    if ((grid[0] == player && grid[4] == player && grid[8] == player) || (grid[2] == player && grid[4] == player && grid[6] == player)) {
            result = 1;
    }
    return result;
}

int is_occupied(const char grid[GRID_CELL], int cell) {
    return grid[cell - 1] != ' ';
}