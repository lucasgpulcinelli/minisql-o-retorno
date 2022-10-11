#ifndef __FILE_CONTROL_H__
#define __FILE_CONTROL_H__

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "entries.h"

#define PAGE_SIZE 960
#define HEADER_SIZE 21


typedef struct{
    bool status;
    int32_t stack;
    uint32_t nextRRN;
    uint32_t entries_removed;
    uint32_t pages;
    uint32_t times_compacted;
} header;

typedef struct{
    entry* entries;
    header* header;
    int size;
} table;


header* readHeader(FILE* fp);
void writeHeader(FILE *fp_out, header *head);
table* readTableBinary(FILE* fp);
void deleteTable(table* t);
void deleteHeader(header* h);

#endif
