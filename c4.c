/* Compile: gcc -std=c99 -o write_benchmark write_benchmark.c
 * Run:     ./write_benchmark
 */
//for nanosleep
#define _POSIX_C_SOURCE 199309L

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <getopt.h>
#include <poll.h>
#include "interface.h"
#include "message.h"
#include "game.h"
#include "visualize.h"
#include "debug.h"

typedef enum {
    Disconnected_State,
    Connected_State
} ClientState;

ClientState connectionState = Disconnected_State;

/** Handles SIGINT signals */
void sigintHandler(int s);
/** Handles SIGPIPE signals */
void sigpipeHandler(int s);
/** Registers signal handlers */
void registerSignalhandler();
/** Cleans up all used functions */
void cleanup();
/** Handles the user input
 * @input input string
 * @return 0 on success 1 on failure
 */
int handleUserInput(char *input);
/** Reads user input char per char
 * @fd pipe discriptor where to get the data
 * @return 0 on success 1 on failure
 */
int readUserInput();

void sigintHandler(int s) {
    (void)s;
    (void)printf("ok, ok I exit\n");
    cleanup();
    exit(EXIT_FAILURE);
}

void sigpipeHandler(int s) {
    (void)s;
    (void)printf("got a sigpipe\n");
    cleanup();
    exit(EXIT_FAILURE);
}

void registerSignalhandler() {
   signal(SIGINT, sigintHandler);
   signal(SIGPIPE, sigpipeHandler);
}

void cleanup()
{
    interface_closeClient();
    interface_cleanup();
}

int handleUserInput(char *input)
{
    DEBUG(1, "got user input");
    DEBUG(1, input);

    if (strcmp(input, "q") == 0)
    {
        int result;
        DEBUG(1, "closing game");
        result = game_close();
        if (result == 0)
        {
            INFO("exiting game");
            cleanup();
            exit(EXIT_SUCCESS);
        }
        else
        {
            return 1;
        }
    }
    else if (strcmp(input, "r") == 0)
    {
        DEBUG(1, "restart game");
        return game_restart();
    }
    else if ((input[0] == 'p') && (strlen(input) == 2))
    {
        DEBUG(1, "place column");
        return game_place(atoi(&input[1]));
    }
    else if (strcmp(input, "c") == 0)
    {
        DEBUG(1, "commit place");
        return game_commit();
    }
    else if (strcmp(input, "u") == 0)
    {
        DEBUG(1, "take back");
        return game_unplace();
    }
    else
    {
        INFO("wrong input");
        // ignore
    }

    return 0;
}

int readUserInput()
{
    static char buffer[5] = {0};
    int ret;
    struct pollfd stdinPoll = { .fd = STDIN_FILENO, .events = POLLIN };

    ret = poll(&stdinPoll, (nfds_t) 1, 200); // Blocking I/0 only for 200ms

    if (!(ret == POLLIN))
    {
        return 0;
    }

    if (fgets(buffer, 5, stdin) != NULL)
    {
        buffer[strlen(buffer)-1] = 0;
        handleUserInput(buffer);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int opt;
    char identifier[MAX_ID_SIZE];
    int server = 0;
    PlayerType playerType = HumanPlayer;
    Player playerNumber;

    strncpy(identifier, "irda", MAX_ID_SIZE);

    while ((opt = getopt(argc, argv, "i:hsc")) != -1) {
        switch (opt) {
        case 'i':
             (void)strncpy(identifier, optarg, MAX_ID_SIZE);
            break;
        case 's':
            server = 1;
            break;
        case 'c':
            playerType = ComputerPlayer;
            break;
        case 'h':
        default: /* '?' */
            (void)fprintf(stderr, "Usage: %s [-i identifier] [-s] [-c] [-h]\n"
                                  "   i - The identifier should be a string with a maximum length of 30 characters\n"
                                  "   s - Starts in server mode.\n"
                                  "   c - Starts a computer player.\n"
                                  "   h - Shows this help.\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    registerSignalhandler();
    interface_init(identifier);

    if (server)
    {
        INFO("waiting for client to connect");
        playerNumber = PlayerTwo;

        while (1)
        {
            if (connectionState == Disconnected_State)
            {
                DEBUG(1, "open server");
                if (interface_openServer() == 0)
                {
                    DEBUG(1, "server connected");
                    INFO("client connected");
                    connectionState = Connected_State;
                    game_initGame(PlayerTwo, playerType);
                }
            }

            if (connectionState == Connected_State)
            {
                DEBUG(1, "process game");
                if (game_process() != 0)
                {
                    DEBUG(1, "game disconnected");
                    INFO("client disconnected");
                    interface_closeServer();
                    connectionState = Disconnected_State;
                }

                DEBUG(1, "reading user input");
                if (readUserInput() != 0)
                {
                    visualize_clearScreen();
                    DEBUG(1, "game disconnected");
                    INFO("user input broken");
                    interface_closeServer();
                    connectionState = Disconnected_State;
                }
            }
        }
    }
    else
    {
        INFO("waiting for server to appear...");
        playerNumber = PlayerOne;

        while (1)
        {
            if (connectionState == Disconnected_State)
            {
                DEBUG(1, "open client");
                if (interface_openClient() == 0)
                {
                    DEBUG(1, "client connected");
                    connectionState = Connected_State;
                    game_initGame(playerNumber, playerType);
                }
            }

            if (connectionState == Connected_State)
            {
                DEBUG(1, "process game");
                if (game_process() != 0)
                {
                    visualize_clearScreen();
                    DEBUG(1, "game disconnected");
                    INFO("server disconnected");
                    interface_closeClient();
                    connectionState = Disconnected_State;
                }

                DEBUG(1, "reading user input");
                if (readUserInput() != 0)
                {
                    visualize_clearScreen();
                    DEBUG(1, "game disconnected");
                    INFO("user input broken");
                    interface_closeClient();
                    connectionState = Disconnected_State;
                }
            }
        }
    }

    return 0;
}
