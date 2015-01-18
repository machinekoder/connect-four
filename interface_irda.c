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

char irdaName[FD_SIZE];

int fdIrDA;
char readBuffer[READ_BUFFER_SIZE];
int readBufferPos;

/**
 * Reads a message from the interface
 * @container message container to store the data
 * @fd descriptor of the interface
 * @return 0 on success -1 on failure
 */
int readMessage(MessageContainer *container, int fd);
/**
 * Writes message to the interface
 * @container message container where data is stored
 * @fd descriptor of the interface
 * @return 0 on success -1 on failure
 */
int writeMessage(MessageContainer *container, int fd);

/**
 * Close the IrDA file descriptor.
 * @return int 0 on success -1 on failure
 */
int openIrDA();
/**
 * Close the IrDA file descriptor.
 */
void closeIrDA();

void interface_init(char* identifier) {
    fdIrDA = -1;
    readBufferPos = 0;
    snprintf(irdaName, FD_SIZE, "%scmd",identifier);
}

void interface_cleanup() {
    base64_cleanup();
}

int interface_openClient() {
    return openIrDA();
}

int interface_openServer() {
    return openIrDA();
}

void interface_closeClient() {
    closeIrDA();
}

void interface_closeServer() {
    closeIrDA();
}

void closeIrDA()
{
    if (fdIrDA != -1)
    {
        DEBUG(2, "closing cmd mq");
        if (close(fdIrDA) == -1)
        {
            ERROR("error closing cmd mq");
        }
        fdIrDA = -1;
    }
}

int openIrDA()
{
    fdIrDA = open("/dev/irda", O_RDWR);

    DEBUG(1, "Openingn irda driver!");
    if (fdIrDA < 0)
    {
	ERROR("error opening IrDA driver.");
	return 1;
    }

    return 0;

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

    return result;
}

int writeMessage(MessageContainer* container, int fd)
{
    size_t encodedLength;
    char *encodedData;
    char endChar = '\n';
    size_t sent = 0;

    DEBUG(3, "encoding data");
    encodedData = base64_encode((const unsigned char*)container,
                                sizeof(MessageContainer),
                                &encodedLength);
    DEBUG(3, encodedData);

    while (encodedLength > 0)
    {
        sent = write(fd, encodedData, encodedLength);
        if (sent == -1u)
        {
            ERROR("writing message failed");
            free(encodedData);
            return -1;
        }
        encodedLength -= sent;
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
    return writeMessage(message, fdIrDA);
}

int interface_writeServer(MessageContainer *message) {
    return writeMessage(message, fdIrDA);
}

int interface_readClient(MessageContainer *message) {
    return readMessage(message, fdIrDA);
}
int interface_readServer(MessageContainer *message) {
    return readMessage(message, fdIrDA);
}
