#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

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

    while (!feof(fp) && (*line)[char_index - 1] != '\n') {
        if (line_len == char_index) {
            line_len *= STR_GROWTH_FACTOR;
            XREALLOC(char, *line, line_len);
        }

        (*line)[char_index] = fgetc(fp);
        char_index++;
    }

    (*line)[char_index - 1] = '\0';
}

ssize_t min(ssize_t a, ssize_t b) {
    return a*(a <= b) + b*(b > a);
}

void binaryOnScreen(char* filename){
	FILE *fp;

	if(filename == NULL) {
        ABORT_PROGRAM("binaryOnScreen got NULL filename as input");
	}
    
    OPEN_FILE(fp, filename, "rb");

    uint32_t sum = 0;
	for(char c = getc(fp); c != EOF; c = getc(fp)) {
		sum += c;
	}

	printf("%lf\n", sum / (double) 100);
    fclose(fp);
}	
