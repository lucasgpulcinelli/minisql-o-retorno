#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#define INIT_LEN 64           //Initial string size in readFirstLine
#define STR_GROWTH_FACTOR 2   //Rate at which string grows in readFirstLine

/*
 * Returns the first argument passed to a variadic macro.
 */
#define _GET_FIRST_ARG(FIRST, ...) FIRST

/*
 * Returns the 17th argument passed to it. Its sole purpose is to
 * implement NUM_VA_ARGS.
 */
#define _GET_17TH_ARG(_1, _2, _3, _4, _5, _6, _7, _8,             \
                     _9, _10, _11, _12, _13, _14, _15, _16,       \
                     _SEVENTEENTH, ...) _SEVENTEENTH              \

/*
 * Returns the number of arguments passed to a variadic macro. The 
 * number of arguments must be greater than zero and less than or
 * equal to 16. The input is __VA_ARGS__, the output is the number
 * of arguments passed in __VA_ARGS__
 */
#define NUM_VA_ARGS(...) _GET_17TH_ARG(__VA_ARGS__, 16, 15, 14, 13, \
                                                    12, 11, 10, 9,  \
                                                    8, 7, 6, 5, 4,  \
                                                    3, 2, 1)

/*
 * ABORTPROGRAM aborts the program; showing the file, line, errno
 * and a message with printf-like formatting.
 */
#define ABORT_PROGRAM(...) fatalError(__LINE__, __FILE__, ##__VA_ARGS__);

/*
 * EXIT_ERROR has the same purpose as ABORT_PROGRAM, but it follows the
 * guidelines for this assignment. It has a major drawback, though: it
 * does not give any specific details of the error, only a generic error
 * message.
 */
#define EXIT_ERROR()                                        \
    printf("Falha no processamento do arquivo.\n");         \
    exit(EXIT_SUCCESS);                                     \

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

//XCALLOC does the same as XALLOC, but uses calloc to set memory to '0's.
#define XCALLOC(type, p, size)                \
    p = calloc(size, sizeof(type));           \
    if(!(p)){                                 \
        ABORT_PROGRAM("calloc");              \
    }                                         \

//OPEN_FILE does the same as fopen, but verifies if the specified file exists.
//usually, this would call ABORT_PROGRAM, however, because of runcodes test
//cases, a simple printf and a silent exit were used.
#define OPEN_FILE(file_pointer, filename, mode)         \
    (file_pointer) = fopen((filename), (mode));         \
    if((file_pointer) == NULL){                         \
        EXIT_ERROR()                                    \
    }                                                   \

//MEMSET_ALLOC does the same as XALLOC, but it initializes all bytes to chr
#define MEMSET_ALLOC(type, p, size, chr)          \
    XALLOC(type, p, size)                         \
    memset(p, chr, sizeof(type)*size);            \

/*
 * READ_INPUT does the same as scanf, but it verifies if the input
 * has a valid format and throws an appropriate error message.
 */
#define READ_INPUT(...)                                                     \
    do{                                                                     \
        int32_t matches = scanf(__VA_ARGS__);                               \
        if(matches == EOF){                                                 \
            ABORT_PROGRAM("Invalid input format");                          \
        }                                                                   \
        if(matches != NUM_VA_ARGS(__VA_ARGS__) - 1){                        \
            ABORT_PROGRAM("Invalid input format: matched "                  \
                          "%d patterns of %d required. Use %s",             \
                          matches, NUM_VA_ARGS(__VA_ARGS__) - 1,            \
                          _GET_FIRST_ARG(__VA_ARGS__))                      \
        }                                                                   \
    }while(false)                                                           \

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
void readFirstLine(char** line, FILE* fp);

/*
 * min takes two ssize_ts and returns the smallest one.
 */
ssize_t min(ssize_t a, ssize_t b);

/*
 * strStrip trims all the blanck characters (as defined in ctype.h)
 * from the beggining and the end of a string.
 */
void strStrip(char** str_ptr);

//binaryOnScreen prints a very unreliable hash of the file on stdout.
void binaryOnScreen(FILE* fp);

#endif

