#ifndef _GAME_H_
#define _GAME_H_

#define FIELD_COLUMNS 7
#define FIELD_ROWS 6

#include "message.h"

typedef char GameField[FIELD_COLUMNS][FIELD_ROWS] ;

typedef enum {
    ComputerPlayer,
    HumanPlayer,
    NoPlayer
} PlayerType;

/** Initializes the game 
 * @type The type of the player, human or computer 
 */
void game_initGame(PlayerType type);
/** Processes the game, should be called periodically
 * @return 0 on success 1 on failure
 */
int  game_process();
/** Restarts the game, called by client 
 * @return 0 on success 1 on failure
 */
int game_restart();
/** Places a beacon, called by client
 * @column column where to place the beacon
 * @return 0 on success 1 on failure
 */
int game_place(int column);
/** Unplaces a beacon, called by client
 * @return 0 on success 1 on failure
 */
int game_unplace();
/** Cmmits a placement, called by client
 * @return 0 on success 1 on failure
 */
int game_commit();
/** Closes the game, called by client
 * @return 0 on success 1 on failure
 */
int game_close();

#endif
