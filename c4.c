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
#include "interface_dl.h"
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
int readUserInput(int fd);

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
    interface_closeLibrary();
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

int readUserInput(int fd)
{
    char readChar;
    static char buffer[100];
    static int bufferPos = 0;
    
    while (read(fd, &readChar, 1u) > 0)
    {
        if (readChar != '\n')
        {
            buffer[bufferPos] = readChar;
            bufferPos += 1;
            
            if (bufferPos == 100)
            {
                bufferPos = 0;
            }
        }
        else
        {
            buffer[bufferPos] = '\0';
            if (handleUserInput(buffer) != 0)
            {
                return -1;
            }
            bufferPos = 0;
        }
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    pid_t pid;
    int stdinPipe[2];
    int opt;
    char identifier[MAX_ID_SIZE];
    char transport[5];
    char libname[30];
    int server = 0;
    PlayerType playerType = HumanPlayer;
    
    strncpy(transport, "fifo", 5u);
    strncpy(identifier, "game", MAX_ID_SIZE);
    
    while ((opt = getopt(argc, argv, "i:t:hsc")) != -1) {
        switch (opt) {
        case 'i':
             (void)strncpy(identifier, optarg, MAX_ID_SIZE);
            break;
        case 't':
            (void)strncpy(transport, optarg, 5u);
            break;
        case 's':
            server = 1;
            break;
        case 'c':
            playerType = ComputerPlayer;
        case 'h':
        default: /* '?' */
            (void)fprintf(stderr, "Usage: %s [-i identifier] [-h] [-t msg|fifo] [-s]\n"
                                  "The identifier should be a string with a maximum length of 30 characters\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    
    snprintf(libname, 30u, "./libinterface_%s.so", transport);
    
    if (pipe(stdinPipe) == -1)
    {
        ERROR("failed to create pipe");
        exit(EXIT_FAILURE);
    }
    if ((fcntl(stdinPipe[0], F_SETFL, O_NONBLOCK) == -1)
        || (fcntl(stdinPipe[1], F_SETFL, O_NONBLOCK) == -1))
    {
        ERROR("failed to set non-blocking option");
        exit(EXIT_FAILURE);
    }
    
    pid = fork();   // fork to create a process for reading stdin
    
    if (pid < 0)
    {
        ERROR("failed to fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)  // child, stdin reader
    {
        char readChar;
        
        while (read(STDIN_FILENO, &readChar, 1u) > 0)
        {
            DEBUG(3, "reading stdin");
            if (write(stdinPipe[1], &readChar, 1u) == -1)
            {
                ERROR("error writing pipe");
                exit(EXIT_FAILURE);
            }
        }
        
        return 0;
    }
    else // parent, doing as usual
    {  
        Player playerName;
        
        interface_openLibrary(libname);
        registerSignalhandler();
        interface_init(identifier);
        
        if (server) 
        {
            INFO("waiting for client to connect");
            playerName = PlayerTwo;
            
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
                    if (readUserInput(stdinPipe[0]) != 0)
                    {
                        visualize_clearScreen();
                        DEBUG(1, "game disconnected");
                        INFO("user input broken");
                        interface_closeServer();
                        connectionState = Disconnected_State;
                    }
                }
                
                struct timespec tim;
                tim.tv_sec = 0;
                tim.tv_nsec = 20000000L; // 200ms
                (void)nanosleep(&tim, NULL);
            }
        }
        else
        {
            INFO("waiting for server to appear...");
            playerName = PlayerOne;
            
            while (1)
            {
                if (connectionState == Disconnected_State)
                {
                    DEBUG(1, "open client");
                    if (interface_openClient() == 0)
                    {
                        DEBUG(1, "client connected");
                        connectionState = Connected_State;
                        game_initGame(playerName, playerType);
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
                    if (readUserInput(stdinPipe[0]) != 0)
                    {
                        visualize_clearScreen();
                        DEBUG(1, "game disconnected");
                        INFO("user input broken");
                        interface_closeClient();
                        connectionState = Disconnected_State;
                    }
                }
                
                struct timespec tim;
                tim.tv_sec = 0;
                tim.tv_nsec = 20000000L; // 200ms
                (void)nanosleep(&tim, NULL);
            }
        }
    }
    
    return 0;
}
