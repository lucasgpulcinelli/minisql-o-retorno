#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "file_control.h"

#define START 0
#define FROM_POSITION 0

#define EMPTY_STACK -1
#define NO_ENTRIES_REMOVED 0
#define NOT_COMPACTED 0

#define ONE_ELEMENT 1

#define INIT_FILE_HEADER(p, stat, top, next, removed, num_pages, compacted)    \
    p->status = stat;              /*File consistency status*/                 \
    p->stack = top;                /*Pointer to the top of stack*/             \
    p->nextRRN = next;             /*Next entry index*/                        \
    p->entries_removed = removed;  /*Number of entries removed*/               \
    p->pages = num_pages;          /*Number of disk pages used*/               \
    p->times_compacted = compacted;/*Number of times file was compacted*/      

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

void writeHeader(FILE *fp_out, header *head);
void commandCreate(void);

void commandFrom(void);

void commandWhere(void);

void commandDelete(void);

void commandInsert(void);

void commandCompact(void);

#endif

