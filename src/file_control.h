#ifndef __FILE_CONTROL_H__
#define __FILE_CONTROL_H__

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "entries.h"


#define PAGE_SIZE 960
#define ENTRIES_PER_PAGE (PAGE_SIZE/MAX_SIZE_ENTRY)
#define HEADER_SIZE 21

//struct header contains the header for a binary file
typedef struct{
    bool status;
    int32_t stack;
    uint32_t nextRRN;
    uint32_t entries_removed;
    uint32_t pages;
    uint32_t times_compacted;
} header;

/*
 * struct table is a abstract data type that represents a full binary file, with
 * functions for reading and writing entries in it.
 */
typedef struct{
    header* header;
    FILE* fp;
} table;

//readHeader reads a header from a file pointer fp.
header* readHeader(FILE* fp);


void writeHeader(FILE *fp, header *head);

/*
 * readTableBinary reads a table from a binary file, returning NULL if there was
 * any error.
 */
table* readTableBinary(FILE* fp);

/*
 * readNextEntry reads the next entry from the table, the returned entry must be
 * deleted afterwards.
 */
entry* readNextEntry(table* t);

/*
 * hasNextEntry returns true if the table can read another entry, meaning if 
 * readNextEntry(t) will not be NULL in the next call.
 */
bool hasNextEntry(table* t);

/*
 * seekTable seeks to the index of the entry provided, such that the 
 * readNextEntry(t) will return the entry with the RRN provided.
 */
void seekTable(table* t, size_t entry_number);

/*
 * rewindTable returns the table to the begginig, being equivalent of 
 * seekTable(t, 0).
 */
void rewindTable(table* t);

//deleteTable deletes the table and the header associated with it.
void deleteTable(table* t);

//deleteHeader deletes a header.
void deleteHeader(header* h);

#endif
