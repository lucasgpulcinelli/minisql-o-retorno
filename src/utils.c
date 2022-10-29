#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <ctype.h>

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

void readFirstLine(char **line, FILE *fp){
    size_t line_len = INIT_LEN;
    size_t char_index = 0;
    XALLOC(char, *line, line_len);

    do {
        if (line_len == char_index) {
            line_len *= STR_GROWTH_FACTOR;
            XREALLOC(char, *line, line_len);
        }

        (*line)[char_index] = fgetc(fp);
        char_index++;
    } while (!feof(fp) && (*line)[char_index - 1] != '\n');

    (*line)[char_index - 1] = '\0';
}

ssize_t min(ssize_t a, ssize_t b) {
    return a*(a <= b) + b*(b < a);
}

void strStrip(char **str_ptr) {
    ssize_t len = strlen(*str_ptr);
    ssize_t end = len;
    while(len > 0 && isblank((*str_ptr)[len - 1])) {
        len--;
    }

    ssize_t start = 0;
    while(start <= len && isblank((*str_ptr)[start])) {
        start++;
    }

    if(start == 0 && len == end) {
        return;
    }

    char* striped_str;
    XALLOC(char, striped_str, len - start + 1);

    strncpy(striped_str, *(str_ptr) + start, len - start);
    striped_str[len - start] = '\0';

    free(*str_ptr);
    *str_ptr = striped_str;
} 
