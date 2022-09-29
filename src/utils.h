#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>

/*
 * ABORTPROGRAM aborts the program; showing the file, line, errno
 * and a message with printf-like formatting
 */
#define ABORT_PROGRAM(...) fatalError(__LINE__, __FILE__, __VA_ARGS__);

/*
 * XALLOC allocates memory in the heap in the pointer p,
 * checking if the allocation failed and aborting the program if so
 */
#define XALLOC(type, p, size)          \
    (p) = malloc(sizeof(type)*(size)); \
    if(!(p)){                          \
        ABORT_PROGRAM("malloc");        \
    }

//XREALLOC does the same as XALLOC, but uses realloc with a new size
#define XREALLOC(type, p, newsize)            \
    (p) = realloc(p, sizeof(type)*(newsize)); \
    if(!(p)){                                 \
        ABORT_PROGRAM("realloc");              \
    } \

//OPEN_FILE does the same as fopen, but verifies if the specified file exists.
#define OPEN_FILE(file_pointer, filename, mode) \
    (file_pointer) = fopen((filename), (mode)); \
    if ((file_pointer) == NULL) {               \
        ABORT_PROGRAM("open %s", (filename));    \
    }                                           \


/*
 * fatalError aborts the program with the file and line of the error, and a
 * printf-like formatted message
 */
void fatalError(int line, char* file, char* fmt, ...);

#endif

