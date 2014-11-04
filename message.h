#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#define PAYLOAD_SIZE 100

typedef enum {
    Restart_MessageType,    // restart message from client
    Close_MessageType,      // close request from client
    Place_MessageType,      // place message from client
    Unplace_MessageType,    // unplace message from client
    Commit_MessageType,     // commit placement message from client
    Field_MessageType,      // current field from server
    Message_MessageType,// some message
} MessageType;

typedef struct {
    MessageType type;
    char payload[PAYLOAD_SIZE];
} MessageContainer;

#endif
