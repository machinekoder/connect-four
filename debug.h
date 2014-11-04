#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>

#define DEBUG_LEVEL 0

/** Debug macro */
#define DEBUG(l, x) if (DEBUG_LEVEL >= l) (void)printf("[%s] %s\n", __func__, x);
/** Debugs a MessageContainer */
#define DEBUG_MESSAGE(l, x) if (DEBUG_LEVEL >= l) (void)printf("[%s] type: %i\n", __func__, x->type);
/** Prints out info strings */
#define INFO(x) (void)printf("%s\n", x)
/** Prints out error message */
#define ERROR(x) perror(x);

#endif
