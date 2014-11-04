/* Compile: gcc -std=c99 -o write_benchmark write_benchmark.c
 * Run:     ./write_benchmark
 */
//for nanosleep
#define _POSIX_C_SOURCE 199309L

#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h> 
#include <signal.h>
#include <getopt.h>
#include "interface_dl.h"
#include "game.h"
#include "debug.h"

typedef enum {
    Disconnected_ServerState,
    Connected_ServerState
} ServerState;
    
ServerState serverState = Disconnected_ServerState;

/** Handles SIGINT signals */
void sigintHandler(int s);
/** Handles SIGPIPE signals */
void sigpipeHandler(int s);
/** Registers signal handlers */
void registerSignalhandler();
/** Cleans up all used functions */
void cleanup();

void sigintHandler(int s) {
    (void)s;
    INFO("ok, ok I exit");
    cleanup();
    exit(EXIT_FAILURE); 
}

void sigpipeHandler(int s) {
    (void)s;
    INFO("got a sigpipe");
    cleanup();
    exit(EXIT_FAILURE); 
}

void registerSignalhandler() {
   signal(SIGINT, sigintHandler);
   signal(SIGPIPE, sigpipeHandler);
}

void cleanup()
{
    interface_closeServer();
    interface_cleanup();
    interface_closeLibrary();
}

int main(int argc, char *argv[]) {
    int opt;
    char identifier[MAX_ID_SIZE];
    char transport[5];
    char libname[30];
    
    strncpy(transport, "fifo", 5u);
    strncpy(identifier, "game", MAX_ID_SIZE);
    
    while ((opt = getopt(argc, argv, "i:t:h")) != -1) {
        switch (opt) {
        case 'i':
            (void)strncpy(identifier, optarg, MAX_ID_SIZE);
            break;
        case 't':
            (void)strncpy(transport, optarg, 5u);
            break;
        case 'h':
        default: /* '?' */
            (void)fprintf(stderr, "Usage: %s [-i identifier] [-h] [-t msg|fifo]\n"
                                  "The identifier should be a string with a maximum length of 30 characters\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    
    (void)snprintf(libname, 30u, "./libinterface_%s.so", transport);
    
    interface_openLibrary(libname);
    registerSignalhandler();
    interface_init(identifier);
    
    serverState = Disconnected_ServerState;
    
    INFO("waiting for client to connect");
    
    while (1) 
    {
        if (serverState == Disconnected_ServerState)
        {
            DEBUG(1, "open server");
            if (interface_openServer() == 0)
            {
                DEBUG(1, "server connected");
                INFO("client connected");
                serverState = Connected_ServerState;
                game_initGame(ComputerPlayer);
            }
        }
        
        if (serverState == Connected_ServerState)
        {
            DEBUG(1, "process game");
            if (game_process() != 0)
            {
                DEBUG(1, "game disconnected");
                INFO("client disconnected");
                interface_closeClient();
                serverState = Disconnected_ServerState;
            }
        }
        
        struct timespec tim;
        tim.tv_sec = 0;
        tim.tv_nsec = 20000000L; // 200ms
        (void)nanosleep(&tim, NULL);
    }
    
    return 0;
}
