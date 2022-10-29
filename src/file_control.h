#ifndef __FILE_CONTROL_H__
#define __FILE_CONTROL_H__

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "entries.h"


#define PAGE_SIZE 960
#define ENTRIES_PER_PAGE (PAGE_SIZE/MAX_SIZE_ENTRY)
#define HEADER_SIZE 21

//Macros that define header encoding. 
#define OK_HEADER '1'
#define ERR_HEADER '0'
#define EMPTY_STACK -1

/* The number of disk pages occupied by a table is the ceiling
 * of table_size/PAGE_SIZE. Its gruesome formula (that maximizes
 * efficiency) is given by the expression below:
 */
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

/*
 * writeHeader writes the contents of header* head in the header
 * of fp. It also sets the remaining bytes in the first page of
 * fp to '$'.
 */
void writeHeader(FILE *fp, header *head);

/*
 * createEmptyTable initializes an empty table and returns it
 * in read and write mode. 
 */
table* createEmptyTable(char* table_name);

/*
 * openTable opens the table in the file named table_name on a
 * mode specified by the user. It supports "rb", read-only mode,
 * "r+b", read and write mode, and "wb", write-only mode. These
 * modes work in the same way as file descriptors on the C
 * standart library.
 */
table* openTable(char* table_name, const char* mode);

/*
 * readNextEntry reads the next entry from the table, the returned entry must be
 * deleted afterwards.
 */
entry* readNextEntry(table* t);

/*
 * appendEntryOnTable writes entry es on table t. If the stack of
 * deleted entries is empty, it appends the entry at the end of the table.
 * Otherwise, it overwrites the first deleted entry and updates the stack.
 */
void appendEntryOnTable(table* t, entry* es);

//removeEntryFromTable deletes the entry of number rrn from the table.
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

/*
 * closeTable closes the table (passed in table* t) and frees the memory
 * allocated to open it. If the table is not in read-only mode, it
 * updates its header and prints a hash of the table on screen by calling
 * tableHashOnScreen.
 */
void closeTable(table* t);

//tableHashOnScreen prints very unreliable hash of the table on stdout.
void tableHashOnScreen(table* t);

//getTimesCompacted returns the number of times the table has been compacted.
uint32_t getTimesCompacted(table* t);

/*
 * setTimesCompacted sets the number of times the table has been compacted
 * to uint32_t num_times_compacted. This is not a dangerous operations, 
 * since this number is unimportant to all other table operations
 */
void setTimesCompacted(table* t, uint32_t num_times_compacted);

#endif
