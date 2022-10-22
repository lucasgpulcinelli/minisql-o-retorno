#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "file_control.h"

#define START 0
#define FROM_POSITION 0

#define EMPTY_STACK -1
#define NO_ENTRIES_REMOVED 0
#define NOT_COMPACTED 0

#define INIT_FILE_HEADER(p, stat, top, next, removed, num_pages, compacted)    \
    p->status = stat;              /*File consistency status*/                 \
    p->stack = top;                /*Pointer to the top of stack*/             \
    p->nextRRN = next;             /*Next entry index*/                        \
    p->entries_removed = removed;  /*Number of entries removed*/               \
    p->pages = num_pages;          /*Number of disk pages used*/               \
    p->times_compacted = compacted;/*Number of times file was compacted*/      

#define COMMANDS_SIZE 6

enum Commands {
    create = 1,
    /*
     * originally, "select" was the name for the next command, however, select 
     * is a standard library function name.
     */
    from, 
    where,
    delete,
    insert,
    compact
};

/*
 * commandCreate reads a headerless CSV and stores it in a newly
 * created table (stored as a binary file).
 */
void commandCreate(void);

/*
 * commandFrom gets the name of a binary file from stdin and prints all 
 * entries contained in it. 
 */
void commandFrom(void);

/*
 * commandWhere gets the name of a binary file and a number n of queries from 
 * stdin, as well as a series of n key-value pairs (composed of a field name,
 * defined as in entries.c fields_str_arr, and an integer or strings between 
 * quotes). It prints, for each query, the fields in the binary file that have
 * the specified value.
 */
void commandWhere(void);

/*
 * commandDelete deletes all query matches, provided in the same way as 
 * commandWhere.
 */
void commandDelete(void);

void commandInsert(void);

void commandCompact(void);


#endif
