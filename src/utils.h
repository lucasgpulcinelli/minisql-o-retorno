#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include <sys/types.h>

#define INIT_LEN 64
#define STR_GROWTH_FACTOR 2

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

//MEMSET_ALLOC does the same as XALLOC, but it initializes all bytes to '$'
#define MEMSET_ALLOC(type, p, size)          \
    XALLOC(type, p, size)                    \
    memset(p, '$', sizeof(type)*size);       \

/*
 * READ_INPUT does the same as scanf, but it verifies if the input
 * has a valid format and throws an appropriate error message.
 */
#define READ_INPUT(...)                           \
    int32_t err = scanf(__VA_ARGS__);             \
    if (err == EOF) {                             \
        ABORT_PROGRAM("Invalid input format");    \
    }                                             \



/*
 * fatalError aborts the program with the file and line of the error, and a
 * printf-like formatted message
 */
void fatalError(int line, char* file, char* fmt, ...);

/*
 * readFirstLine reads the first line of fp (relative to current position)
 * and stores it in line. The function deals with memory allocation, do not
 * pass a malloced pointer to it.
 */
void readFirstLine(char **line, FILE *fp);

/*
 * min takes two ssize_ts and returns the smallest one.
 */
ssize_t min(ssize_t a, ssize_t b);

/*
 * binaryOnScreen returns a hash of the file of name filename
 */
void binaryOnScreen(char* filename);

#endif

