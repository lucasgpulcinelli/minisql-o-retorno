#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "file_control.h"

#define COMMANDS_SIZE 4
#define COMMANDS_OFFSET 7

/*
 * enum Commands enumerates the 6 commands the user may call.
 */
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
 * commandCreate reads a CSV and stores it in a newly
 * created table (stored as a binary file). The name of the CSV file 
 * and the name of the table are passed through stdin.
 */
void commandCreate(void);

/*
 * commandWhere gets the name of a binary file and a number n of queries from 
 * stdin, as well as a series of n key-value pairs (composed of a field name,
 * defined as in entries.c fields_str_arr, and an integer or strings between 
 * quotes). It prints, for each query, the fields in the binary file that have
 * the specified value.
 */
void commandWhere(void);

/*
 * commandInsert reads a specified number of entries from stdin
 * and inserts them in a table. The number n of entries and the name of
 * the table are read from stdin.
 */
void commandInsert(void);

/*
 * 
 */
void commandJoin(void);

#endif
