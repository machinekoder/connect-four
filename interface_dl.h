#ifndef _INTERFACE_DL_H_
#define _INTERFACE_DL_H_

#define MAX_ID_SIZE 30

#include "message.h"

void *interfaceLib;
void (*interface_init)(char*);
void (*interface_cleanup)(void);
int (*interface_openClient)(void);
void (*interface_closeClient)(void);
int (*interface_readClient)(MessageContainer*);
int (*interface_writeClient)(MessageContainer*);
int (*interface_openServer)(void);
void (*interface_closeServer)(void);
int (*interface_readServer)(MessageContainer*);
int (*interface_writeServer)(MessageContainer*);

/** Opens the interface library 
 * @name name of the library
 */
void interface_openLibrary(char *name);
/** Closes the interface library */
void interface_closeLibrary();

#endif
