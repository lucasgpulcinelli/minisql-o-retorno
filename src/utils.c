#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "utils.h"


void fatalError(int line, char* file, char* fmt, ...){
    int ini_errno = errno; //fprintf can change the errno value, so store it

    //initialize variable arguments list
    va_list ap;
    va_start(ap, fmt);

    fprintf(stderr, "Error at line %d of file %s: ", line, file);
    vfprintf(stderr, fmt, ap);
    
    if(!errno){
        //errno is SUCESS
        fprintf(stderr, "\n");
    }
    else{
        fprintf(stderr, ": %s\n", strerror(ini_errno));
    }

    //end the variable argument list
    va_end(ap);

    exit(EXIT_FAILURE);
}

