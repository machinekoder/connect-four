#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#define MAX_ID_SIZE 30

#include "message.h"

/** Initializes the interface 
 * @identifier Identifier that should be used as address for the interface
 */
extern void interface_init(char *identifier);
/** Cleans up the data used by the interface */
extern void interface_cleanup();
/** Tries to establish a connection to the server
 * @return 0 on success 1 on failure
 */
extern int interface_openClient();
/** Tries to establish a connection to the client
 * @return 0 on success 1 on failure
 */
extern int interface_openServer();
/** Closes the connection to the server
 * @return 0 on success 1 on failure
 */
extern void interface_closeClient();
/** Closes the connection to the client
 * @return 0 on success 1 on failure
 */
extern void interface_closeServer();
/** Writes data to the server
 * @message pointer to message container with the data to write
 * @return 0 on success -1 on failure
 */
extern int interface_writeClient(MessageContainer *message);
/** Writes data to the client
 * @message pointer to message container with the data to write
 * @return 0 on success -1 on failure
 */
extern int interface_writeServer(MessageContainer *message);
/** Reads data from the server
 * @message pointer to message container where data should be stored
 * @return 0 on success -1 on failure
 */
extern int interface_readClient(MessageContainer *message);
/** Reads data from the client
 * @message pointer to message container where data should be stored
 * @return 0 on success -1 on failure
 */
extern int interface_readServer(MessageContainer *message);

#endif
