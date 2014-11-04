#include <string.h> 
#include <stdlib.h> 
#include "game.h"
#include "visualize.h"
#include "interface_dl.h"
#include "debug.h"

typedef enum {
    Init,
    Running,
    ComputerTurn,
    PlayerTurn,
    PlayerPlaced,
    ComputerWon,
    PlayerWon,
    Remis
} GameState;

typedef struct {
    int column;
} PlaceMessageContainer;

typedef struct {
    GameField gameField;
} FieldMessageContainer;

typedef struct {
    char message[FIELD_COLUMNS*FIELD_ROWS];
} MessageMessageContainer;

MessageContainer container;
GameField gameField;
GameState gameState = Init;
PlayerType playerType;
char gameMessage[100];

/** Inits the game field with empty beacons */
void initGameField();
/** Updates the current game field with new data
 * @field pointer to the new game field data
 */
void updateGameField(GameField *field);
/** Updates the current game message with a new one
 * @message the new message
 */
void updateMessage(char *message);
/** Checks for 4-connects
 * @return ' ' if no conect was found, 'X' if game field is full, 'O' for computer, '@' for player
 */
char checkConnects();
/** Checks the status of the game
 * @return 0 if game is running, 1 if game has ended
 */
int checkGame();
/** Send the messages for end of the game
 * @return 0 on success, 1 on failue
 */
int endGame();
/** Place a beacon in a column, no logic
 * @type type of player, human or computer
 * @column the column to place the beacon in
 * @return 0 on success, -1 on failure
 */
int placeColumn(PlayerType type, int column);
/** Unplaces a beacon from a column
 * @column column where the beacon should be removed
 * @return 0 on success, -1 on failure
 */
int unplaceColumn(int column);
/** Place a beacon in a column, uses logic for computer and removing of beacons
 * @type type of player, human or computer, none for removing beacon
 * @column the column to place the beacon in
 * @return 0 on success, -1 on failure
 */
int placeBeacon(PlayerType type, int column);
/** Processes a received message 
 * @message pointer to the received message
 * @return 0 on success, 1 on failure
 */
int processMessage(MessageContainer* message);
/** Send game field to client
 * @field pointer to the data
 * @return 0 on success, 1 on failure
 */
int sendField(GameField *field);
/** Send game message to client
 * message pointer to the message
 * @return 0 on success, 1 on failure
 */
int sendMessage(char *message);
/** Send close event to client
 * @return 0 on success, 1 on failure
 */
int sendServerClose();
/** Send restart message to server
 * @return 0 on success, 1 on failure
 */
int sendRestart();
/** Send place message to server
 * @column column whre to place the beacon
 * @return 0 on success, 1 on failure
 */
int sendPlace(int column);
/** Send unplace message to server
 * @return 0 on success, 1 on failure
 */
int sendUnplace();
/** Send commit message to server
 * @return 0 on success, 1 on failure
 */
int sendCommit();
/** Send close message to server
 * @return 0 on success, 1 on failure
 */
int sendClientClose();

void initGameField()
{
    int c;
    int r;
    
    for (c = 0; c < FIELD_COLUMNS; ++c)
    {
        for (r = 0; r < FIELD_ROWS; ++r)
        {
            gameField[c][r] = ' ';
        }
    }
}

void updateGameField(GameField* field)
{
    memcpy(&gameField, field, sizeof(GameField));
}

void updateMessage(char* message)
{
    strncpy(gameMessage, message, 100u);
}

char checkConnects()
{
    int c;
    int r;
    int x;
    char matchChar;
    
    DEBUG(3, "check connects");
    
    // vertical
    for (c = 0; c < FIELD_COLUMNS; ++c)
    {
        for (r = 0; r < (FIELD_ROWS-4); ++r)
        {
            matchChar = gameField[c][r];
            if (matchChar != ' ')
            {
                for (x = 1; x < 4; ++x)
                {
                    if (matchChar != gameField[c][r+x])
                    {
                        break;
                    }
                }
                if (x == 4) // all matched
                {
                    DEBUG(3, "vertical match");
                    return matchChar;
                }
            }
        }
    }
    
    // horizontal
    for (c = 0; c < (FIELD_COLUMNS-4); ++c)
    {
        for (r = 0; r < FIELD_ROWS; ++r)
        {
            matchChar = gameField[c][r];
            if (matchChar != ' ')
            {
                for (x = 1; x < 4; ++x)
                {
                    if (matchChar != gameField[c+x][r])
                    {
                        break;
                    }
                }
                if (x == 4) // all matched
                {
                    DEBUG(3, "horizontal match");
                    return matchChar;
                }
            }
        }
    }
    
    // diagonal down
    for (c = 0; c < (FIELD_COLUMNS-4); ++c)
    {
        for (r = 0; r < (FIELD_ROWS-4); ++r)
        {
            matchChar = gameField[c][r];
            if (matchChar != ' ')
            {
                for (x = 1; x < 4; ++x)
                {
                    if (matchChar != gameField[c+x][r+x])
                    {
                        break;
                    }
                }
                if (x == 4) // all matched
                {
                    DEBUG(3, "diagonal down match");
                    return matchChar;
                }
            }
        }
    }
    
    // diagonal up
    for (c = 0; c < (FIELD_COLUMNS-4); ++c)
    {
        for (r = (FIELD_ROWS-4); r < FIELD_ROWS; ++r)
        {
            matchChar = gameField[c][r];
            if (matchChar != ' ')
            {
                for (x = 1; x < 4; ++x)
                {
                    if (matchChar != gameField[c+x][r-x])
                    {
                        break;
                    }
                }
                if (x == 4) // all matched
                {
                    DEBUG(3, "diagonal up match");
                    return matchChar;
                }
            }
        }
    }
    
    // full
    for (c = 0; c < FIELD_COLUMNS; ++c)
    {
        for (r = 0; r < FIELD_ROWS; ++r)
        {
            if (gameField[c][r] == ' ')
            {
                DEBUG(3, "game field not full");
                return ' ';
            }
        }
    }
    
    DEBUG(3, "game field full");
    return 'X'; // full field
}

int checkGame()
{
    char result;
    result = checkConnects();
    
    switch (result)
    {
        case ' ': 
            return 0;
        case 'O':
            gameState = ComputerWon;
            return 1;
        case '@':
            gameState = PlayerWon;
            return 1;
        case 'X':
            gameState = Remis;
            return 1;
        default:
            return 0;
    }
}

int placeColumn(PlayerType type, int column)
{
    int i;
    
    if ((column < 0) || (column > 6))
    {
        return -1;
    }
    
    for (i = (FIELD_ROWS-1); i >= 0; i--)
    {
        if (gameField[column][i] == ' ')
        {
            gameField[column][i] = (type == ComputerPlayer) ? 'O' : '@';
            return 0;
        }
    }
    
    return -1;
}

int unplaceColumn(int column)
{
    int i;
    
    if ((column < 0) || (column > 6))
    {
        return -1;
    }
    
    for (i = 0; i < FIELD_ROWS; ++i)
    {
        if (gameField[column][i] != ' ')
        {
           gameField[column][i] = ' ';
           return 0;
        }
    }
    
    return 0;
}

int placeBeacon(PlayerType type, int column)
{
    static int lastColumn = 0;
    
    if (type == HumanPlayer)
    {
        DEBUG(3, "placing human beacon");
        
        lastColumn = column;
        return placeColumn(type, column);
    }
    else if (type == ComputerPlayer)
    {
        DEBUG(3, "placing computer beacon");
        
        int checked = 0;
        
        do {
            if (!checked && ((rand() % 3) == 0))    // last column with 1/3 possibility
            {
                column = lastColumn;
            }
            else
            {
                column = (rand() % 7);  // 0 to 6
            }
            checked = 1;
        }
        while (placeColumn(type, column) == -1);
    }
    else // remove that last turn
    {
        return unplaceColumn(lastColumn);
    }
    
    return 0;
}

int endGame()
{
    if (gameState == PlayerWon)
    {
        if (sendMessage("Congratulations, you have won the game!") != 0)
        {
            return 1;
        }
    }
    else if (gameState == ComputerWon)
    {
        if (sendMessage("Haha, you lost!") != 0)
        {
            return 1;
        }
    }
    else
    {
        if (sendMessage("Oh, we have remis here") != 0)
        {
            return 1;
        }
    }
    return sendField(&gameField);
}

int processMessage(MessageContainer *message)
{
    if (playerType == ComputerPlayer)
    {
        if (message->type == Close_MessageType)
        {
            INFO("received exit from client");
            interface_closeServer();
            exit(EXIT_SUCCESS);
        }
        else if (message->type == Restart_MessageType)
        {
            DEBUG(2, "received restart");
            game_initGame(ComputerPlayer);
        }
        else if ((gameState == PlayerTurn) && (message->type == Place_MessageType))
        {
            int result;
            
            DEBUG(2, "received placing message");
            PlaceMessageContainer *placeContainer;
            placeContainer = (PlaceMessageContainer*)&(message->payload);
            result = placeBeacon(HumanPlayer, placeContainer->column);
            if (result == -1)
            {
                DEBUG(2, "not playable");
                return sendMessage("Not placeable, try again");
            }
            else
            {
                DEBUG(2, "placed");
                gameState = PlayerPlaced;
                return (sendMessage("Ready to commit") || sendField(&gameField));
            }
        }
        else if ((gameState == PlayerPlaced) && (message->type == Commit_MessageType))
        {
            if (checkGame() == 0)
            {
                DEBUG(2, "commited");
                gameState = ComputerTurn;
                return sendMessage("Computers turn, please stay calm");
            }
            else 
            {
                DEBUG(2, "the game has ended");
                return endGame(); // the game has ended
            }
        }
        else if ((gameState == PlayerPlaced) && (message->type == Unplace_MessageType))
        {
            DEBUG(2, "unplaced");
            gameState = PlayerTurn;
            (void)placeBeacon(NoPlayer, 0); // return value can be ignored
            return (sendMessage("Try again") || sendField(&gameField));
        }
        else
        {
            DEBUG(2, "wrong message received");
            return sendMessage("You are doing something wrong");
        }
    }
    else
    {
        if (message->type == Field_MessageType)
        {
            DEBUG(2, "received field message");
            FieldMessageContainer *fieldContainer;
            fieldContainer = (FieldMessageContainer*)&(message->payload);
            updateGameField(&(fieldContainer->gameField));
            visualize_game(&gameField, gameMessage);
        }
        else if (message->type == Message_MessageType)
        {
            DEBUG(2, "received string message");
            MessageMessageContainer *messageContainer;
            messageContainer = (MessageMessageContainer*)&(message->payload);
            updateMessage(messageContainer->message);
            visualize_game(&gameField, gameMessage);
        }
        else if (message->type == Close_MessageType)
        {
            INFO("received exit from server");
            interface_closeClient();
            exit(EXIT_SUCCESS);
        }
        else
        {
            DEBUG(2, "wrong message received");
            return 0;
            // ignore
        }
    }
    
    return 0;
}

int sendField(GameField *field)
{
    FieldMessageContainer *fieldContainer;
    
    container.type = Field_MessageType;
    fieldContainer = (FieldMessageContainer*)&(container.payload);
    memcpy(&(fieldContainer->gameField), field, sizeof(GameField));
    DEBUG(1, "writing field to pub");
    if (interface_writeServer(&container) == -1)
    {
        ERROR("error while writing");
        return 1;
    }
    
    return 0;
}

int sendMessage(char *message)
{
    MessageMessageContainer *messageContainer;
    
    container.type = Message_MessageType;
    messageContainer = (MessageMessageContainer*)&(container.payload);
    strcpy((messageContainer->message), message);
    DEBUG(1, "writing message to pub");
    if (interface_writeServer(&container) == -1)
    {
        ERROR("error while writing");
        return 1;
    }
    
    return 0;
}

int sendServerClose()
{
    container.type = Close_MessageType;
    DEBUG(1, "writing close to pub");
    if (interface_writeServer(&container) == -1)
    {
        ERROR("error while writing");
        return 1;
    }
    
    return 0;
}

int sendRestart()
{
    container.type = Restart_MessageType;
    DEBUG(1, "writing restart to cmd");
    if (interface_writeClient(&container) == -1)
    {
        ERROR("error while writing");
        return 1;
    }
    
    return 0;
}

int sendPlace(int column)
{
    PlaceMessageContainer *placeContainer;
    
    container.type = Place_MessageType;
    placeContainer = (PlaceMessageContainer*)&(container.payload);
    placeContainer->column = column;
    
    DEBUG(1, "writing place to cmd");
    if (interface_writeClient(&container) == -1)
    {
        ERROR("error while writing");
        return 1;
    }
    
    return 0;
}

int sendUnplace()
{
    container.type = Unplace_MessageType;
    DEBUG(1, "writing unplace to cmd");
    if (interface_writeClient(&container) == -1)
    {
        ERROR("error while writing");
        return 1;
    }
    
    return 0;
}

int sendCommit()
{
    container.type = Commit_MessageType;
    DEBUG(1, "writing commit to cmd");
    if (interface_writeClient(&container) == -1)
    {
        ERROR("error while writing");
        return 1;
    }
    
    return 0;
}
int sendClientClose()
{
    container.type = Close_MessageType;
    DEBUG(1, "writing close to cmd");
    if (interface_writeClient(&container) == -1)
    {
        ERROR("error while writing");
        return 1;
    }
    
    return 0;
}

void game_initGame(PlayerType type)
{
    initGameField();
    playerType = type;
    strcpy(gameMessage, "");
    
    if (playerType == HumanPlayer)
    {
        gameState = Running;
        visualize_game(&gameField, gameMessage);
    }
    else
    {
        gameState = PlayerTurn;
        sendField(&gameField);
        sendMessage("Started a new game, its your turn");
        sendField(&gameField);
    }
}

int game_process()
{
    if (playerType == HumanPlayer)
    {
        DEBUG(1,"reading messages");
        while (interface_readClient(&container) > 0)
        {
            DEBUG(1,"got message");
            if (processMessage(&container) != 0)
            {
                return 1;
            }
        }
    }
    else
    {
        DEBUG(1,"reading messages");
        while (interface_readServer(&container) > 0)
        {
            DEBUG(1,"got message");
            if (processMessage(&container) != 0)
            {
                return 1;
            }
        }
        
        if (gameState == ComputerTurn)
        {
            DEBUG(1,"computers turn");
            (void)placeBeacon(ComputerPlayer, 0);   // return always 0 for ComputerPlayer
            if (checkGame() == 1)
            {
                return endGame();  // someone has won the game
            }
            else 
            {
                gameState = PlayerTurn;
                return (sendMessage("Its your turn") || sendField(&gameField));
            }
            
        }
    }
    
    return 0;
}

int game_restart()
{
    return sendRestart();
}

int game_place(int column)
{
    return sendPlace(column);
}

int game_unplace()
{
    return sendUnplace();
}

int game_commit()
{
    return sendCommit();
}

int game_close()
{
    return sendClientClose();
}
