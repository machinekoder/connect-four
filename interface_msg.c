#include <mqueue.h>
#include <sys/stat.h>
#include "interface.h"
#include "debug.h"

#define FD_SIZE MAX_ID_SIZE+4

char pubname[FD_SIZE];
char cmdname[FD_SIZE];

mqd_t fdCmdMq;
mqd_t fdPubMq;

/** Reads a message from the interface
 * @container message container to store the data
 * @fd descriptor of the interface
 * @return 0 on success -1 on failure
 */
int readMessage(MessageContainer *container, mqd_t fd);
/** Writes message to the interface
 * @container message container where data is stored
 * @fd descriptor of the interface
 * @return 0 on success -1 on failure
 */
int writeMessage(MessageContainer *container, mqd_t fd);

void interface_init(char* identifier)
{
    fdCmdMq = (mqd_t)-1;
    fdPubMq = (mqd_t)-1;
    snprintf(pubname, FD_SIZE, "/%spub",identifier);
    snprintf(cmdname, FD_SIZE, "/%scmd",identifier);
}

void interface_cleanup()
{
}

int interface_openClient()
{
    if (fdPubMq == (mqd_t)-1)
    {
        DEBUG(2, "open pub mq");
        if ((fdPubMq = mq_open(pubname, O_RDONLY | O_NONBLOCK)) != (mqd_t)-1)
        {
            DEBUG(2, "pub open");
        }
        else
        {
            DEBUG(2, "pub not open");
            return 1;
        }
    }
    
    if (fdCmdMq == (mqd_t)-1)
    {
        DEBUG(2, "open cmd mq");
        if ((fdCmdMq = mq_open(cmdname, O_WRONLY | O_NONBLOCK)) != (mqd_t)-1)
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

int interface_openServer()
{
    struct mq_attr attr;
    attr.mq_curmsgs = 0;
    attr.mq_flags  = 0;
    attr.mq_msgsize = sizeof(MessageContainer);
    attr.mq_maxmsg = 3;
        
        
    if (fdCmdMq == (mqd_t)-1)
    {
        DEBUG(2, "open cmd mq");
        if ((fdCmdMq = mq_open(cmdname, O_RDONLY | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr)) != (mqd_t)-1)
        {
            DEBUG(2, "cmd open");
        }
        else
        {
            DEBUG(2, "cmd not open");
            return 1;
        }
    }
    
    if (fdPubMq == (mqd_t)-1)
    {
        DEBUG(2, "open pub mq");
        if ((fdPubMq = mq_open(pubname, O_WRONLY | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr)) != (mqd_t)-1)
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

void interface_closeClient()
{
    if (fdCmdMq != (mqd_t)-1)
    {
        DEBUG(2, "closing cmd mq");
        if (mq_close(fdCmdMq) == -1)
        {
            ERROR("error closing cmd mq");
        }
        fdCmdMq = (mqd_t)-1;
    }
    
    if (fdPubMq != (mqd_t)-1)
    {
        DEBUG(2, "closing pub mq");
        if (mq_close(fdPubMq) == -1)
        {
            ERROR("error closing pub mq");
        }
        
        fdPubMq = (mqd_t)-1;
    }
}

void interface_closeServer()
{
    if (fdCmdMq != (mqd_t)-1)
    {
        DEBUG(2, "closing cmd mq");
        if (mq_close(fdCmdMq) == -1)
        {
            ERROR("error closing cmd mq");
        }
        
        DEBUG(2, "unlinking cmd mq");
        if (mq_unlink(cmdname) == -1)
        {
            ERROR("error unlinking cmd mq");
        }
        
        fdCmdMq = (mqd_t)-1;
    }
    
    if (fdPubMq != (mqd_t)-1)
    {
        DEBUG(2, "closing pub fifo");
        if (mq_close(fdPubMq) == -1)
        {
            ERROR("error closing pub fifo");
        }
        
        DEBUG(2, "unlinking pub fifo");
        if (mq_unlink(pubname) == -1)
        {
            ERROR("error unlinking pub fifo");
        }
        
        fdPubMq = (mqd_t)-1;
    }
}

int readMessage(MessageContainer* container, mqd_t fd)
{
    int result;
    
    DEBUG(2, "reading mq");
    result = mq_receive(fd, (char*)container, sizeof(MessageContainer), 0);
    
    /*if (result == -1)
    {
        ERROR("error reading mq");
    }
    else */
    if (result > 0)
    {
        DEBUG_MESSAGE(3, container);
    }
    
    return result;
}

int writeMessage(MessageContainer* container, mqd_t fd)
{
    int result;
    
    DEBUG(2, "writing mq");
    result = mq_send(fd, (char*)container, sizeof(MessageContainer), 0);
    
    if (result == -1)
    {
        ERROR("error writing mq");
    }
    
    return result;
}

int interface_writeClient(MessageContainer *message) {
    return writeMessage(message, fdCmdMq);
}

int interface_writeServer(MessageContainer *message) {
    return writeMessage(message, fdPubMq);
}

int interface_readClient(MessageContainer *message) {
    return readMessage(message, fdPubMq);
}

int interface_readServer(MessageContainer *message) {
    return readMessage(message, fdCmdMq);
}
