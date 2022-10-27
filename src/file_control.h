#ifndef __FILE_CONTROL_H__
#define __FILE_CONTROL_H__

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "entries.h"


#define PAGE_SIZE 960
#define ENTRIES_PER_PAGE (PAGE_SIZE/MAX_SIZE_ENTRY)
#define HEADER_SIZE 21

#define OK_HEADER '1'
#define ERR_HEADER '0'
#define EMPTY_STACK -1

#define NUM_PAGES_FORMULA(nextRRN)                                   \
    nextRRN/ENTRIES_PER_PAGE +                                       \
    ((nextRRN/ENTRIES_PER_PAGE)*ENTRIES_PER_PAGE != nextRRN) + 1     \

//struct header contains the header for a binary file
typedef struct{
    char status;
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
    bool read_only;
} table;

//readHeader reads a header from a file pointer fp.
header* readHeader(FILE* fp);

void writeHeader(FILE *fp, header *head);

table* createEmptyTable(char* table_name);

table* openTable(char* table_name, const char* mode);

/*
 * readNextEntry reads the next entry from the table, the returned entry must be
 * deleted afterwards.
 */
entry* readNextEntry(table* t);

void appendEntryOnTable(table* t, entry* es);

void removeEntryFromTable(table* t, size_t rrn);

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

void closeTable(table *t);

void tableHashOnScreen(table* t);

//getTimesCompacted returns the number of times the table has been compacted.
uint32_t getTimesCompacted(table* t);

void setTimesCompacted(table* t, uint32_t num_times_compacted);

#endif
