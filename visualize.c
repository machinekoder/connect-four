#include <stdio.h>
#include "visualize.h"
#include "game.h"

void visualize_clearScreen()
{
    int n;
    for (n = 0; n < 10; n++)
    {
        (void)printf( "\n\n\n\n\n\n\n\n\n\n" );
    }
}

void visualize_game(GameField* gameField, char* gameMessage)
{
    int c;
    int r;
    
    visualize_clearScreen();
    
    for (r = 0; r < FIELD_ROWS; ++r)
    {
        (void)printf("   ");
        for (c = 0; c < FIELD_COLUMNS; ++c)
        {
            (void)printf("+—");
        }
        (void)printf("+\n   ");
        for (c = 0; c < FIELD_COLUMNS; ++c)
        {
            (void)printf("|%c", (*gameField)[c][r]);
        }
        (void)printf("|\n");
    }
    
    (void)printf("   ");
    for (c = 0; c < FIELD_COLUMNS; ++c)
    {
        (void)printf("+—");
    }
    (void)printf("+\n\n");
    
    printf("   %s\n\n   input:\n", gameMessage);
}
