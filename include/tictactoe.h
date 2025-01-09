#ifndef TICTACTOE_H
#define TICTACTOE_H

/**
 * The size of the grid
 */
#define GRID_SIZE 3
/**
 * The number of case in the grid
 */
#define GRID_CASE GRID_SIZE * GRID_SIZE

/**
 * Initializes an empty grid
 * @param grid the grid to be initialized
 */
void set_empty_grid(char grid[GRID_CASE]);

/**
 * Show the grid into the panel
 * @param grid the character table who represents the grid
 */
void show_grid(const char grid[GRID_CASE]);

/**
 * Update the grid with the index of the case to change
 * @param i the index of the case (must be between 1 and 9)
 * @param grid the character table who represents the grid
 * @param symbol the character to put in the grid
 * @exception 
 * - Out of range : If the index is lower than 1 or greater than 9
 */
void update_grid(const int i, char grid[GRID_CASE], const char symbol);

/**
 * Check if the grid is full or not
 * @param grid the character table who represents the grid
 * @return `1` if the grid is full else `0`
 */
int is_full(const char grid[GRID_CASE]);

/**
 * Check if the selected player is a winner
 * @param grid the character table who represents the grid
 * @param player the player to check
 * @return `1` if the player is a winner else `0` 
 */
int is_winner(char joueur, const char grid[GRID_CASE]);

#endif