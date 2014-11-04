#ifndef _VISUALIZE_H_
#define _VISUALIZE_H_

#include "game.h"
/** Clears the terminal screen */
void visualize_clearScreen();
/** Displays game field and message 
 * @gameField pointer to the game field
 * @gameMessage message of the game
 */
void visualize_game(GameField* gameField, char* gameMessage);

#endif
