Connect-Four
============

Implementation of the classic connect fource game for the lecture Linux Sysprog.
![Alt text](/doc/snapshot.png "The game")

## Build
The game uses only POSIX and C99 components. You can compile it using:

    make

## Documentation
Documentation is provided using Doxygen.

    make doc
    
## How to play
The game can be used with FIFO, message queue and the very special IRDA transports. Furthermore one can specify more a specific session or device identifier.

    c4 [-i identifier] [-s] [-c] [-h]
       i - The identifier should be a string with a maximum length of 30 characters
       s - Starts in server mode.
       c - Starts a computer player.
       h - Shows this help.

Inside the game the following commands can be used

    p<colnum> Places a piece in column <colnum> (numbered 0-6) and displays the provisional result on the board.
    u   Take back the last p command if it has not been committed yet.
    c   Commit the turn.
    q   Shutdown the client.
    r   Start a new game.
