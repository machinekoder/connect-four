#include "interface.h"
#include "debug.h"

#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "base64.h"

#define READ_BUFFER_SIZE 1000u

#define FD_SIZE MAX_ID_SIZE+3

char pubname[FD_SIZE];
char cmdname[FD_SIZE];

int fdCmdFifo;
int fdPubFifo;
char readBuffer[READ_BUFFER_SIZE];
int readBufferPos;

/** Reads a message from the interface
 * @container message container to store the data
 * @fd descriptor of the interface
 * @return 0 on success -1 on failure
 */
int readMessage(MessageContainer *container, int fd);
/** Writes message to the interface
 * @container message container where data is stored
 * @fd descriptor of the interface
 * @return 0 on success -1 on failure
 */
int writeMessage(MessageContainer *container, int fd);

void interface_init(char* identifier) {
    fdCmdFifo = -1;
    fdPubFifo = -1;
    readBufferPos = 0;
    snprintf(pubname, FD_SIZE, "%spub",identifier);
    snprintf(cmdname, FD_SIZE, "%scmd",identifier);
}

void interface_cleanup() {
    base64_cleanup();
}

int interface_openClient() {
    if (fdPubFifo == -1)
    {
        DEBUG(2, "open pub fifo");
        if ((fdPubFifo = open(pubname, O_RDONLY | O_NONBLOCK)) != -1)
        {
            DEBUG(2, "pub open");
        }
        else
        {
            DEBUG(2, "pub not open");
            return 1;
        }
    }
    
    if (fdCmdFifo == -1)
    {
        DEBUG(2, "open cmd fifo");
        if ((fdCmdFifo = open(cmdname, O_WRONLY | O_NONBLOCK)) != -1)
        {
            DEBUG(2, "cmd open");
        }
        else
        {
            DEBUG(2, "cmd not open");
            return 1;
        }
    }
    
    return 0;
}

int interface_openServer() {
    if (fdCmdFifo == -1)
    {
        if (access(cmdname, F_OK) == 0)
        {
            DEBUG(2, "cmd fifo exists");
        }
        else
        {
            DEBUG(2, "creating cmd fifo");
            if (mkfifo(cmdname, S_IRUSR | S_IWUSR) != 0) 
            {
                ERROR("error creating cmd fifo");
                exit(EXIT_FAILURE);
            }
        }
        
        DEBUG(2, "open cmd fifo");
        if ((fdCmdFifo = open(cmdname, O_RDONLY | O_NONBLOCK)) != -1)
        {
            DEBUG(2, "cmd open");
        }
        else
        {
            DEBUG(2, "cmd not open");
            return 1;
        }
    }
    
    if (fdPubFifo == -1)
    {
        if (access(pubname, F_OK) == 0)
        {
            DEBUG(2, "pub fifo exists");
        }
        else
        {
            DEBUG(2, "creating pub fifo");
            if (mkfifo(pubname, S_IRUSR | S_IWUSR) != 0) 
            {
                ERROR("error creating pub fifo");
                exit(EXIT_FAILURE);
            }
        }
        
        DEBUG(2, "open pub fifo");
        if ((fdPubFifo = open(pubname, O_WRONLY | O_NONBLOCK)) != -1)
        {
            DEBUG(2, "pub open");
        }
        else
        {
            DEBUG(2, "pub not open");
            return 1;
        }
    }
    
    return 0;
}

void interface_closeClient() {
    if (fdCmdFifo != -1)
    {
        DEBUG(2, "closing cmd fifo");
        if (close(fdCmdFifo) == -1)
        {
            ERROR("error closing cmd fifo");
        }
        fdCmdFifo = -1;
    }
    
    if (fdPubFifo != -1)
    {
        DEBUG(2, "closing pub fifo");
        if (close(fdPubFifo) == -1)
        {
            ERROR("error closing pub fifo");
        }
        
        fdPubFifo = -1;
    }
}

void interface_closeServer() {
    if (fdCmdFifo != -1)
    {
        DEBUG(2, "closing cmd fifo");
        if (close(fdCmdFifo) == -1)
        {
            ERROR("error closing cmd fifo");
        }
        
        DEBUG(2, "unlinking cmd fifo");
        if (unlink(cmdname) == -1)
        {
            ERROR("error unlinking cmd fifo");
        }
        
        fdCmdFifo = -1;
    }
    
    if (fdPubFifo != -1)
    {
        DEBUG(2, "closing pub fifo");
        if (close(fdPubFifo) == -1)
        {
            ERROR("error closing pub fifo");
        }
        
        DEBUG(2, "unlinking pub fifo");
        if (unlink(pubname) == -1)
        {
            ERROR("error unlinking pub fifo");
        }
        
        fdPubFifo = -1;
    }
}

int readMessage(MessageContainer* container, int fd)
{
    int result;
    char readChar;
    
    DEBUG(2, "reading fifo");
    while ((result = read(fd, &readChar, 1u)) == 1u)
    {
        if (readChar != '\n')
        {
            readBuffer[readBufferPos] = readChar;
            readBufferPos += 1;
            
            if (readBufferPos == READ_BUFFER_SIZE)
            {
                perror("message too long!!!");
                return -1;
            }
        }
        else
        {
            size_t dataLength;
            unsigned char *decodedData;
            
            readBuffer[readBufferPos] = '\0';
            DEBUG(2, "message received:");
            DEBUG(2, readBuffer);
            
            DEBUG(3, "decoding data");
            decodedData = base64_decode(readBuffer, readBufferPos, &dataLength);
            memcpy((void*)container, 
                   (void*)decodedData, 
                   (dataLength <= sizeof(MessageContainer) ? dataLength : sizeof(MessageContainer)));
            free(decodedData);
            DEBUG_MESSAGE(3, container);
            
            readBufferPos = 0;
            return 1;
        }
    }
    
    /*if (result == -1)
    {
        ERROR("error reading fifo");
    }*/
    
    return result;
}

int writeMessage(MessageContainer* container, int fd)
{
    size_t encodedLength;
    char *encodedData;
    char endChar = '\n';
    
    DEBUG(3, "encoding data");
    encodedData = base64_encode((const unsigned char*)container, 
                                sizeof(MessageContainer), 
                                &encodedLength);
    DEBUG(3, encodedData);
    
    if (write(fd, encodedData, encodedLength) == -1u)
    {
        ERROR("writing message failed");
        free(encodedData);
        return -1;
    }
    DEBUG(3, "writing message succeded");
    free(encodedData);
    
    if (write(fd, &endChar, 1u) == -1u)
    {
        ERROR("writing end char failed");
        return -1;
    }
    DEBUG(3, "writing end char succeded");
    
    return 0;
}

int interface_writeClient(MessageContainer *message) {
    return writeMessage(message, fdCmdFifo);
}

int interface_writeServer(MessageContainer *message) {
    return writeMessage(message, fdPubFifo);
}

int interface_readClient(MessageContainer *message) {
    return readMessage(message, fdPubFifo);
}
int interface_readServer(MessageContainer *message) {
    return readMessage(message, fdCmdFifo);
}
